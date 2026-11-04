#include "LBRagdollComponent.h"

#include "LBLog.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

ULBRagdollComponent::ULBRagdollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void ULBRagdollComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		CachedMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	}

	if (!CachedMesh)
	{
		UE_LOG(LogLB, Error, TEXT("RagdollComponent sem SkeletalMeshComponent no ator '%s'"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("null"));
	}
}

USkeletalMeshComponent* ULBRagdollComponent::GetMesh() const
{
	return CachedMesh;
}

void ULBRagdollComponent::React(ELBReactionTier Tier, FVector Impulse, FName Bone)
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		return;
	}

	// Leve: a animacao resolve. Ligar fisica aqui e o que transforma o
	// personagem em boneco de pano.
	if (Tier == ELBReactionTier::Light)
	{
		return;
	}

	const bool bPartial = (Tier == ELBReactionTier::Medium);
	const float NewTarget = bPartial ? PartialWeight : 1.0f;
	const float Duration = bPartial ? PartialDuration : FullDuration;

	Mesh->SetAllBodiesBelowSimulatePhysics(RootBone, true, true);
	SetTargetWeight(NewTarget, bPartial ? ELBRagdollPhase::Partial : ELBRagdollPhase::Full, Duration);

	OnRagdollStarted.Broadcast();

	if (!Impulse.IsNearlyZero())
	{
		const FName ApplyBone = Bone.IsNone() ? RootBone : Bone;
		Mesh->AddImpulseToAllBodiesBelow(Impulse, ApplyBone, /*bVelChange=*/true, /*bIncludeSelf=*/true);
	}
}

void ULBRagdollComponent::SetTargetWeight(float NewTarget, ELBRagdollPhase NewPhase, float Duration)
{
	TargetWeight = FMath::Clamp(NewTarget, 0.0f, 1.0f);
	Phase = NewPhase;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoldTimer);
		if (Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				HoldTimer, this, &ULBRagdollComponent::HandleHoldFinished, Duration, /*bLoop=*/false);
		}
	}
}

void ULBRagdollComponent::HandleHoldFinished()
{
	if (Phase == ELBRagdollPhase::Partial || Phase == ELBRagdollPhase::Full)
	{
		Phase = ELBRagdollPhase::Recovering;
		TargetWeight = 0.0f;
	}
}

void ULBRagdollComponent::ForceRecover()
{
	if (Phase == ELBRagdollPhase::Animated)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoldTimer);
	}

	Phase = ELBRagdollPhase::Recovering;
	TargetWeight = 0.0f;
	BlendOutSpeed = FMath::Max(BlendOutSpeed, 8.0f);
}

void ULBRagdollComponent::EndSimulationIfIdle()
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		return;
	}

	if (Phase == ELBRagdollPhase::Recovering && CurrentWeight <= KINDA_SMALL_NUMBER)
	{
		CurrentWeight = 0.0f;
		Mesh->SetAllBodiesBelowPhysicsBlendWeight(RootBone, 0.0f, false, true);
		Mesh->SetAllBodiesBelowSimulatePhysics(RootBone, false, true);
		Phase = ELBRagdollPhase::Animated;
		OnRagdollRecovered.Broadcast();
	}
}

void ULBRagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh || Phase == ELBRagdollPhase::Animated)
	{
		return;
	}

	if (FMath::IsNearlyEqual(CurrentWeight, TargetWeight, 0.001f))
	{
		CurrentWeight = TargetWeight;
		EndSimulationIfIdle();
		return;
	}

	const float Speed = (TargetWeight > CurrentWeight) ? BlendInSpeed : BlendOutSpeed;
	CurrentWeight = FMath::FInterpConstantTo(CurrentWeight, TargetWeight, DeltaTime, Speed);
	CurrentWeight = FMath::Clamp(CurrentWeight, 0.0f, 1.0f);

	Mesh->SetAllBodiesBelowPhysicsBlendWeight(RootBone, CurrentWeight, /*bSkipCustomPhysicsType=*/false, /*bIncludeSelf=*/true);

	EndSimulationIfIdle();
}
