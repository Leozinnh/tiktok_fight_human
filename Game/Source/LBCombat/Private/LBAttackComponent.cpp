#include "LBAttackComponent.h"

#include "LBConfigSubsystem.h"
#include "LBDamageComponent.h"
#include "LBDamageVisualComponent.h"
#include "LBLog.h"
#include "LBRagdollComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ULBAttackComponent::ULBAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULBAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		DamageComp = Owner->FindComponentByClass<ULBDamageComponent>();
		RagdollComp = Owner->FindComponentByClass<ULBRagdollComponent>();
		VisualComp = Owner->FindComponentByClass<ULBDamageVisualComponent>();
	}

	if (DamageComp)
	{
		DamageComp->OnDamageChanged.AddDynamic(this, &ULBAttackComponent::HandleDamageChanged);
	}

	if (!DamageComp || !RagdollComp)
	{
		UE_LOG(LogLBCombat, Warning,
			TEXT("AttackComponent em '%s' sem Damage/Ragdoll. Golpes nao terao efeito."),
			GetOwner() ? *GetOwner()->GetName() : TEXT("null"));
	}
}

void ULBAttackComponent::HandleDamageChanged()
{
	if (VisualComp && DamageComp)
	{
		VisualComp->ApplyState(DamageComp->GetState());
	}
}

float ULBAttackComponent::ComputeSlotSeconds(const FLBAttackDef& Attack) const
{
	if (Attack.SlotMs > 0.0f)
	{
		return Attack.SlotMs / 1000.0f;
	}

	// Sem slot explicito: deriva da intensidade.
	return FMath::Lerp(0.6f, 1.6f, FMath::Clamp(Attack.Intensity, 0.0f, 1.0f));
}

float ULBAttackComponent::GetSlotDurationSeconds() const
{
	return bBusy ? ComputeSlotSeconds(PendingAttack) : LastSlotSeconds;
}

bool ULBAttackComponent::ExecuteAttack(FName AttackId, float DamageMultiplier, const FString& ViewerName)
{
	const UWorld* World = GetWorld();
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	const ULBConfigSubsystem* Config = GI ? GI->GetSubsystem<ULBConfigSubsystem>() : nullptr;
	if (!Config)
	{
		UE_LOG(LogLBCombat, Error, TEXT("ExecuteAttack sem ULBConfigSubsystem"));
		return false;
	}

	FLBAttackDef Attack;
	if (!Config->FindAttack(AttackId, Attack))
	{
		UE_LOG(LogLBCombat, Error, TEXT("Golpe '%s' nao existe em attacks.json"), *AttackId.ToString());
		return false;
	}

	return ExecuteAttackDef(Attack, DamageMultiplier, ViewerName);
}

bool ULBAttackComponent::ExecuteAttackDef(const FLBAttackDef& Attack, float DamageMultiplier, const FString& ViewerName)
{
	if (bBusy)
	{
		// O diretor ja deveria impedir; protege contra chamada manual do painel.
		UE_LOG(LogLBCombat, Verbose, TEXT("Golpe '%s' ignorado: componente ocupado"), *Attack.Id.ToString());
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	PendingAttack = Attack;
	PendingMultiplier = FMath::Max(0.0f, DamageMultiplier);
	PendingViewer = ViewerName;
	bBusy = true;

	const float SlotSeconds = ComputeSlotSeconds(Attack);
	LastSlotSeconds = SlotSeconds;

	OnPlayAttackMontage(Attack);

	World->GetTimerManager().SetTimer(
		ImpactTimer, this, &ULBAttackComponent::HandleImpact,
		FMath::Max(0.02f, SlotSeconds * ImpactDelayFraction), false);

	World->GetTimerManager().SetTimer(
		SlotTimer, this, &ULBAttackComponent::FinishSlot, SlotSeconds, false);

	UE_LOG(LogLBCombat, Verbose, TEXT("Golpe '%s' x%.2f por '%s' (slot %.2fs)"),
		*Attack.Id.ToString(), PendingMultiplier, *ViewerName, SlotSeconds);

	return true;
}

FVector ULBAttackComponent::ComputeImpactDirection() const
{
	// O personagem encara a camera; empurrar ao longo da visao afasta do agressor.
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
		return ViewRotation.Vector();
	}

	const AActor* Owner = GetOwner();
	return Owner ? -Owner->GetActorForwardVector() : FVector::ForwardVector;
}

void ULBAttackComponent::HandleImpact()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const float Intensity = FMath::Clamp(PendingAttack.Intensity, 0.0f, 1.0f);
	const float Amount = BaseDamageUnits * PendingAttack.DamageMultiplier
		* PendingMultiplier * (0.6f + 0.4f * Intensity);

	if (DamageComp)
	{
		DamageComp->ApplyHit(PendingAttack.PrimaryRegion, Amount, Intensity);
	}

	if (RagdollComp)
	{
		const FVector Impulse = ComputeImpactDirection() * PendingAttack.Impulse;
		const FName Bone = PendingAttack.HitBone.IsNone() ? TEXT("head") : PendingAttack.HitBone;
		RagdollComp->React(PendingAttack.ReactionTier, Impulse, Bone);
	}

	FVector ImpactLocation = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 160.0f);
	if (const USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
	{
		const FName Bone = PendingAttack.HitBone.IsNone() ? TEXT("head") : PendingAttack.HitBone;
		if (Mesh->DoesSocketExist(Bone))
		{
			ImpactLocation = Mesh->GetSocketLocation(Bone);
		}
	}

	OnSpawnImpactVfx(PendingAttack, ImpactLocation);
	OnImpactCameraShake(PendingAttack);

	if (PendingAttack.AudioIds.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, PendingAttack.AudioIds.Num() - 1);
		OnPlayImpactSound(PendingAttack, PendingAttack.AudioIds[Index]);
	}

	OnAttackExecuted.Broadcast(PendingAttack.Id, PendingAttack.PrimaryRegion, PendingViewer);
}

void ULBAttackComponent::FinishSlot()
{
	bBusy = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ImpactTimer);
	}
}
