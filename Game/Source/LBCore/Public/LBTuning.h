#pragma once

#include "CoreMinimal.h"
#include "LBTuning.generated.h"

/** Parametros da fila e do diretor de eventos. */
USTRUCT(BlueprintType)
struct LBCORE_API FLBQueueTuning
{
	GENERATED_BODY()

	/** Duracao base de um slot de golpe. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "100"))
	float SlotMs = 900.0f;

	/** Teto da fila. Ao atingir, o excedente e agregado, nunca descartado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "1"))
	int32 MaxPending = 200;

	/** Numero de slots que um item pode esperar antes de furar a fila. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "1"))
	int32 AntiStarvationSlots = 12;

	/** Limite de golpes animados ao mesmo tempo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "1"))
	int32 MaxConcurrentAttacks = 1;
};

/** Modo "hype": quando a fila entope, o lote vira um golpe especial. */
USTRUCT(BlueprintType)
struct LBCORE_API FLBHypeTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "1"))
	int32 Threshold = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0"))
	float CooldownMs = 20000.0f;
};

/** Parametros do modelo de dano. */
USTRUCT(BlueprintType)
struct LBCORE_API FLBDamageTuning
{
	GENERATED_BODY()

	/** Quanto um golpe vaza para regioes vizinhas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0", ClampMax = "1"))
	float RegionFalloff = 0.35f;

	/** Quanto de equilibrio se recupera por segundo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0"))
	float BalanceRecoverPerSec = 0.02f;

	/** Quanto de tontura se dissipa por segundo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0"))
	float StunDecayPerSec = 0.05f;

	/** Abaixo deste equilibrio, o personagem cai. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0", ClampMax = "1"))
	float FallThreshold = 0.75f;

	/** Dano total que satura o personagem (derrota). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "1"))
	float MaxTotalDamage = 100.0f;
};

/** Raiz de configuracao, carregada de Config/tuning.json. */
USTRUCT(BlueprintType)
struct LBCORE_API FLBTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FLBQueueTuning Queue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FLBHypeTuning Hype;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FLBDamageTuning Damage;
};
