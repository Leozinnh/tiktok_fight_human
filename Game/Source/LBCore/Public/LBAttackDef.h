#pragma once

#include "CoreMinimal.h"
#include "LBTypes.h"
#include "LBAttackDef.generated.h"

class UAnimMontage;

/**
 * Definicao de um golpe. Vem de Config/attacks.json — nunca hardcoded.
 * `Intensity` e a forca relativa; `DamageMultiplier` escala o dano aplicado.
 */
USTRUCT(BlueprintType)
struct LBCORE_API FLBAttackDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FText Label;

	/** Montagem executada no agressor (primeira pessoa) ou no personagem. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	TSoftObjectPtr<UAnimMontage> Montage;

	/** 0..1. Governa camera shake, som e probabilidade de queda. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0", ClampMax = "1"))
	float Intensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	ELBDamageRegion PrimaryRegion = ELBDamageRegion::None;

	/** Osso alvo do empurrao fisico (ex.: head, spine_02). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FName HitBone = NAME_None;

	/** Impulso aplicado ao ragdoll, em N.s. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	float Impulse = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	ELBReactionTier ReactionTier = ELBReactionTier::Light;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0", ClampMax = "2"))
	float CameraShake = 0.3f;

	/** Variantes de som; o sistema sorteia uma para evitar repeticao. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	TArray<FName> AudioIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	TSoftObjectPtr<UObject> Vfx;

	/** Tempo minimo entre duas execucoes deste golpe. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0"))
	float CooldownMs = 250.0f;

	/** Duracao do slot na fila do diretor. 0 = derivar da intensidade. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0"))
	float SlotMs = 0.0f;

	bool IsValid() const { return !Id.IsNone(); }
};
