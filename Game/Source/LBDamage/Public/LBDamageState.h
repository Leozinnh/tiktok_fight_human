#pragma once

#include "CoreMinimal.h"
#include "LBTypes.h"
#include "LBDamageState.generated.h"

/**
 * Estado persistente de dano de uma rodada.
 *
 * Regra dura: um golpe NUNCA reduz estes valores. Só ULBMatchSubsystem, na
 * transicao Idle -> Intro de uma nova rodada, pode chamar Reset().
 */
USTRUCT(BlueprintType)
struct LBDAMAGE_API FLBDamageState
{
	GENERATED_BODY()

	// --- Face (0..1) ---
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float Forehead = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float LeftCheek = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float RightCheek = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float Nose = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float Jaw = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float Chin = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float LeftEye = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float RightEye = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float Mouth = 0.0f;

	// --- Corpo (0..1) ---
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float Torso = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float LeftArm = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float RightArm = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float LeftLeg = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") float RightLeg = 0.0f;

	// --- Estado geral ---
	/** 1 = firme, 0 = no chao. */
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage")
	float Balance = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage")
	float Stun = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage")
	float Pain = 0.0f;

	// --- Efeitos discretos persistentes ---
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") bool bBlackEyeLeft = false;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") bool bBlackEyeRight = false;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") bool bBandage = false;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") bool bToothChip = false;
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Damage") bool bLipSplit = false;

	/** Zera tudo. Chamado apenas no inicio de uma nova rodada. */
	void Reset();

	/** Soma de todas as regioes, normalizada por MaxTotal. */
	float GetNormalizedTotal(float MaxTotal) const;

	/** Soma bruta de todas as regioes. */
	float GetRawTotal() const;

	/** Referencia mutavel para a regiao. Retorna um lixo descartavel se None. */
	float& RegionRef(ELBDamageRegion Region);

	/** Leitura por regiao. */
	float GetRegion(ELBDamageRegion Region) const;

	/** Recalcula flags discretas a partir dos valores continuos. */
	void UpdateDerivedFlags();

	void ClampAll();
};
