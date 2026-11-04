#include "LBDamageComponent.h"

#include "LBConfigSubsystem.h"
#include "LBLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

ULBDamageComponent::ULBDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void ULBDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (const ULBConfigSubsystem* Config = GI->GetSubsystem<ULBConfigSubsystem>())
			{
				const FLBDamageTuning& Tuning = Config->GetTuning().Damage;
				DefaultFalloff = Tuning.RegionFalloff;
				BalanceRecoverPerSec = Tuning.BalanceRecoverPerSec;
				StunDecayPerSec = Tuning.StunDecayPerSec;
				MaxTotalDamage = Tuning.MaxTotalDamage;
			}
		}
	}

	State.Reset();
}

void ULBDamageComponent::GetNeighbors(ELBDamageRegion Region, TArray<ELBDamageRegion>& Out)
{
	switch (Region)
	{
	case ELBDamageRegion::Forehead:
		Out = { ELBDamageRegion::LeftEye, ELBDamageRegion::RightEye, ELBDamageRegion::LeftCheek, ELBDamageRegion::RightCheek };
		break;
	case ELBDamageRegion::LeftCheek:
		Out = { ELBDamageRegion::LeftEye, ELBDamageRegion::Jaw, ELBDamageRegion::Mouth, ELBDamageRegion::Forehead };
		break;
	case ELBDamageRegion::RightCheek:
		Out = { ELBDamageRegion::RightEye, ELBDamageRegion::Jaw, ELBDamageRegion::Mouth, ELBDamageRegion::Forehead };
		break;
	case ELBDamageRegion::Nose:
		Out = { ELBDamageRegion::LeftEye, ELBDamageRegion::RightEye, ELBDamageRegion::Mouth };
		break;
	case ELBDamageRegion::Jaw:
		Out = { ELBDamageRegion::Chin, ELBDamageRegion::Mouth, ELBDamageRegion::LeftCheek, ELBDamageRegion::RightCheek };
		break;
	case ELBDamageRegion::Chin:
		Out = { ELBDamageRegion::Jaw, ELBDamageRegion::Mouth };
		break;
	case ELBDamageRegion::LeftEye:
		Out = { ELBDamageRegion::Forehead, ELBDamageRegion::LeftCheek, ELBDamageRegion::Nose };
		break;
	case ELBDamageRegion::RightEye:
		Out = { ELBDamageRegion::Forehead, ELBDamageRegion::RightCheek, ELBDamageRegion::Nose };
		break;
	case ELBDamageRegion::Mouth:
		Out = { ELBDamageRegion::Chin, ELBDamageRegion::Jaw, ELBDamageRegion::LeftCheek, ELBDamageRegion::RightCheek };
		break;
	case ELBDamageRegion::Torso:
		Out = { ELBDamageRegion::LeftArm, ELBDamageRegion::RightArm, ELBDamageRegion::LeftLeg, ELBDamageRegion::RightLeg };
		break;
	case ELBDamageRegion::LeftArm:
	case ELBDamageRegion::RightArm:
	case ELBDamageRegion::LeftLeg:
	case ELBDamageRegion::RightLeg:
		Out = { ELBDamageRegion::Torso };
		break;
	default:
		break;
	}
}

void ULBDamageComponent::ApplyHit(ELBDamageRegion PrimaryRegion, float Amount, float Intensity, float Falloff)
{
	if (PrimaryRegion == ELBDamageRegion::None || Amount <= 0.0f)
	{
		return;
	}

	const float FalloffToUse = Falloff < 0.0f ? DefaultFalloff : FMath::Clamp(Falloff, 0.0f, 1.0f);

	// Dano direto.
	State.RegionRef(PrimaryRegion) += Amount;

	// Vazamento para regioes vizinhas.
	TArray<ELBDamageRegion> Neighbors;
	GetNeighbors(PrimaryRegion, Neighbors);
	for (const ELBDamageRegion Neighbor : Neighbors)
	{
		State.RegionRef(Neighbor) += Amount * FalloffToUse;
	}

	// Estado sistemico.
	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	State.Stun = FMath::Clamp(State.Stun + ClampedIntensity * 0.35f, 0.0f, 1.0f);
	State.Pain = FMath::Clamp(State.Pain + ClampedIntensity * 0.40f, 0.0f, 1.0f);
	State.Balance = FMath::Clamp(State.Balance - ClampedIntensity * 0.30f, 0.0f, 1.0f);

	State.UpdateDerivedFlags();
	State.ClampAll();

	OnRegionDamaged.Broadcast(PrimaryRegion, Amount);
	OnDamageChanged.Broadcast();
}

void ULBDamageComponent::AddToRegion(ELBDamageRegion Region, float Amount)
{
	if (Region == ELBDamageRegion::None || Amount <= 0.0f)
	{
		return;
	}

	State.RegionRef(Region) = FMath::Clamp(State.RegionRef(Region) + Amount, 0.0f, 1.0f);
	State.UpdateDerivedFlags();
	State.ClampAll();

	OnDamageChanged.Broadcast();
}

void ULBDamageComponent::ResetDamage()
{
	State.Reset();
	BroadcastAccumulator = 0.0f;
	OnDamageChanged.Broadcast();
}

float ULBDamageComponent::GetNormalizedDamage() const
{
	return State.GetNormalizedTotal(MaxTotalDamage);
}

void ULBDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bNeedsDecay = State.Stun > 0.0f || State.Pain > 0.0f || State.Balance < 1.0f;
	if (!bNeedsDecay)
	{
		return;
	}

	State.Stun = FMath::Max(0.0f, State.Stun - StunDecayPerSec * DeltaTime);
	State.Pain = FMath::Max(0.0f, State.Pain - StunDecayPerSec * DeltaTime);
	State.Balance = FMath::Min(1.0f, State.Balance + BalanceRecoverPerSec * DeltaTime);

	// A UI nao precisa de 30 atualizacoes por segundo para uma barra que decai.
	BroadcastAccumulator += DeltaTime;
	if (BroadcastAccumulator >= 0.25f)
	{
		BroadcastAccumulator = 0.0f;
		OnDamageChanged.Broadcast();
	}
}

ELBDamageRegion ULBDamageComponent::BoneToRegion(FName BoneName)
{
	const FString Bone = BoneName.ToString().ToLower();

	if (Bone.Contains(TEXT("head")))                                        return ELBDamageRegion::Forehead;
	if (Bone.Contains(TEXT("jaw")) || Bone.Contains(TEXT("chin")))          return ELBDamageRegion::Jaw;
	if (Bone.Contains(TEXT("neck")))                                        return ELBDamageRegion::Jaw;

	if (Bone.Contains(TEXT("_l")))
	{
		if (Bone.Contains(TEXT("clavicle")) || Bone.Contains(TEXT("upperarm")) || Bone.Contains(TEXT("lowerarm")) || Bone.Contains(TEXT("hand")))
		{
			return ELBDamageRegion::LeftArm;
		}
		if (Bone.Contains(TEXT("thigh")) || Bone.Contains(TEXT("calf")) || Bone.Contains(TEXT("foot")))
		{
			return ELBDamageRegion::LeftLeg;
		}
	}

	if (Bone.Contains(TEXT("_r")))
	{
		if (Bone.Contains(TEXT("clavicle")) || Bone.Contains(TEXT("upperarm")) || Bone.Contains(TEXT("lowerarm")) || Bone.Contains(TEXT("hand")))
		{
			return ELBDamageRegion::RightArm;
		}
		if (Bone.Contains(TEXT("thigh")) || Bone.Contains(TEXT("calf")) || Bone.Contains(TEXT("foot")))
		{
			return ELBDamageRegion::RightLeg;
		}
	}

	if (Bone.Contains(TEXT("spine")) || Bone.Contains(TEXT("pelvis")) || Bone.Contains(TEXT("root")))
	{
		return ELBDamageRegion::Torso;
	}

	return ELBDamageRegion::Torso;
}
