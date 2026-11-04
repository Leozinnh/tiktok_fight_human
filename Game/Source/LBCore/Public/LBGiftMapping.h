#pragma once

#include "CoreMinimal.h"
#include "LBGiftMapping.generated.h"

/** Como a quantidade de unidades de um presente vira golpes. */
UENUM(BlueprintType)
enum class ELBGiftCountMode : uint8
{
	/** 5 rosas = 5 golpes individuais, com reacao propria (requisito 9). */
	PerUnit UMETA(DisplayName = "Um golpe por unidade"),

	/** N unidades = 1 execucao, com dano multiplicado por N. */
	Single UMETA(DisplayName = "Execucao unica multiplicada")
};

/**
 * Mapeamento presente -> golpe. Vem de Config/gifts.json e e editavel pelo
 * painel administrativo em tempo de execucao.
 */
USTRUCT(BlueprintType)
struct LBCORE_API FLBGiftMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FString GiftName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	FName AttackId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	ELBGiftCountMode CountMode = ELBGiftCountMode::PerUnit;

	/** Multiplicador adicional de dano por unidade. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown", meta = (ClampMin = "0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LiveBeatdown")
	bool bEnabled = true;
};
