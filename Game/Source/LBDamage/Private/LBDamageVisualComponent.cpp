#include "LBDamageVisualComponent.h"

#include "LBLog.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	// Nomes de parametro do material de pele. Precisam existir no MI da pele.
	const FName ParamFace(TEXT("DMG_Face"));       // media das regioes faciais
	const FName ParamBody(TEXT("DMG_Body"));
	const FName ParamStun(TEXT("DMG_Stun"));
	const FName ParamPain(TEXT("DMG_Pain"));
	const FName ParamSweat(TEXT("DMG_Sweat"));

	const FName FlagBlackEyeLeft(TEXT("FLAG_BlackEyeL"));
	const FName FlagBlackEyeRight(TEXT("FLAG_BlackEyeR"));
	const FName FlagLipSplit(TEXT("FLAG_LipSplit"));
	const FName FlagToothChip(TEXT("FLAG_ToothChip"));
	const FName FlagBandage(TEXT("FLAG_Bandage"));
}

ULBDamageVisualComponent::ULBDamageVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULBDamageVisualComponent::BeginPlay()
{
	Super::BeginPlay();

	if (TargetMeshes.Num() == 0)
	{
		if (const AActor* Owner = GetOwner())
		{
			if (USkeletalMeshComponent* Body = Owner->FindComponentByClass<USkeletalMeshComponent>())
			{
				TargetMeshes.Add(Body);
			}
		}
	}

	EnsureDynamicMaterials();
	ApplyState(LastState);
}

void ULBDamageVisualComponent::EnsureDynamicMaterials()
{
	DynamicMaterials.Reset();

	for (const TObjectPtr<UMeshComponent>& Mesh : TargetMeshes)
	{
		if (!Mesh)
		{
			continue;
		}

		const int32 MaterialCount = Mesh->GetNumMaterials();
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			UMaterialInterface* Current = Mesh->GetMaterial(Index);
			if (!Current)
			{
				continue;
			}

			UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(Current, this);
			if (!Dynamic)
			{
				continue;
			}

			Mesh->SetMaterial(Index, Dynamic);
			DynamicMaterials.Add(Dynamic);
		}
	}

	UE_LOG(LogLBDamage, Log, TEXT("DamageVisual: %d materiais dinamicos preparados"), DynamicMaterials.Num());
}

FName ULBDamageVisualComponent::GetParamName(ELBDamageRegion Region)
{
	switch (Region)
	{
	case ELBDamageRegion::Forehead:   return FName(TEXT("DMG_Forehead"));
	case ELBDamageRegion::LeftCheek:  return FName(TEXT("DMG_LeftCheek"));
	case ELBDamageRegion::RightCheek: return FName(TEXT("DMG_RightCheek"));
	case ELBDamageRegion::Nose:       return FName(TEXT("DMG_Nose"));
	case ELBDamageRegion::Jaw:        return FName(TEXT("DMG_Jaw"));
	case ELBDamageRegion::Chin:       return FName(TEXT("DMG_Chin"));
	case ELBDamageRegion::LeftEye:    return FName(TEXT("DMG_LeftEye"));
	case ELBDamageRegion::RightEye:   return FName(TEXT("DMG_RightEye"));
	case ELBDamageRegion::Mouth:      return FName(TEXT("DMG_Mouth"));
	case ELBDamageRegion::Torso:      return FName(TEXT("DMG_Torso"));
	case ELBDamageRegion::LeftArm:    return FName(TEXT("DMG_LeftArm"));
	case ELBDamageRegion::RightArm:   return FName(TEXT("DMG_RightArm"));
	case ELBDamageRegion::LeftLeg:    return FName(TEXT("DMG_LeftLeg"));
	case ELBDamageRegion::RightLeg:   return FName(TEXT("DMG_RightLeg"));
	default:                          return NAME_None;
	}
}

void ULBDamageVisualComponent::PushScalars(const FLBDamageState& State)
{
	const float FaceAverage =
		(State.Forehead + State.LeftCheek + State.RightCheek + State.Nose
			+ State.Jaw + State.Chin + State.LeftEye + State.RightEye + State.Mouth) / 9.0f;

	const float BodyAverage =
		(State.Torso + State.LeftArm + State.RightArm + State.LeftLeg + State.RightLeg) / 5.0f;

	for (UMaterialInstanceDynamic* Dynamic : DynamicMaterials)
	{
		if (!Dynamic)
		{
			continue;
		}

		// Por regiao.
		for (uint8 Index = 1; Index <= static_cast<uint8>(ELBDamageRegion::RightLeg); ++Index)
		{
			const ELBDamageRegion Region = static_cast<ELBDamageRegion>(Index);
			const FName ParamName = GetParamName(Region);
			if (!ParamName.IsNone())
			{
				Dynamic->SetScalarParameterValue(ParamName, State.GetRegion(Region));
			}
		}

		// Agregados, para materiais que so precisam de um valor global.
		Dynamic->SetScalarParameterValue(ParamFace, FaceAverage);
		Dynamic->SetScalarParameterValue(ParamBody, BodyAverage);
		Dynamic->SetScalarParameterValue(ParamStun, State.Stun);
		Dynamic->SetScalarParameterValue(ParamPain, State.Pain);
		Dynamic->SetScalarParameterValue(ParamSweat, FMath::Clamp(State.Pain * 0.8f + State.Stun * 0.5f, 0.0f, 1.0f));

		// Flags discretas.
		Dynamic->SetScalarParameterValue(FlagBlackEyeLeft, State.bBlackEyeLeft ? 1.0f : 0.0f);
		Dynamic->SetScalarParameterValue(FlagBlackEyeRight, State.bBlackEyeRight ? 1.0f : 0.0f);
		Dynamic->SetScalarParameterValue(FlagLipSplit, State.bLipSplit ? 1.0f : 0.0f);
		Dynamic->SetScalarParameterValue(FlagToothChip, State.bToothChip ? 1.0f : 0.0f);
		Dynamic->SetScalarParameterValue(FlagBandage, State.bBandage ? 1.0f : 0.0f);
	}
}

void ULBDamageVisualComponent::UpdateAccessories(const FLBDamageState& State)
{
	if (BandageMesh)
	{
		BandageMesh->SetVisibility(State.bBandage, true);
	}

	if (DamageLayerMesh)
	{
		// A malha de dano so aparece quando ha algo para mostrar.
		const bool bAnyDiscrete = State.bBlackEyeLeft || State.bBlackEyeRight || State.bLipSplit || State.bToothChip;
		DamageLayerMesh->SetVisibility(bAnyDiscrete, true);
	}
}

void ULBDamageVisualComponent::ApplyState(const FLBDamageState& InState)
{
	LastState = InState;

	PushScalars(LastState);
	UpdateAccessories(LastState);

	OnDamageVisualsUpdated(LastState);
}
