#pragma once

#include "CoreMinimal.h"
#include "LBTypes.generated.h"

/**
 * Quao longe um golpe leva a reacao. Define qual estagio do controlador de
 * fisica responde (ver ULBRagdollComponent).
 */
UENUM(BlueprintType)
enum class ELBReactionTier : uint8
{
	Light   UMETA(DisplayName = "Leve"),    // hit reaction animada + empurrao
	Medium  UMETA(DisplayName = "Medio"),   // ragdoll parcial (cambaleio)
	Heavy   UMETA(DisplayName = "Pesado"),  // ragdoll total + queda
	Special UMETA(DisplayName = "Especial") // ragdoll total + camera cinematografica
};

/**
 * Regioes de dano. O dano e rastreado por regiao, nao globalmente, e persiste
 * durante toda a rodada.
 */
UENUM(BlueprintType)
enum class ELBDamageRegion : uint8
{
	None UMETA(DisplayName = "Nenhuma"),

	// Face
	Forehead    UMETA(DisplayName = "Testa"),
	LeftCheek   UMETA(DisplayName = "Bochecha Esq."),
	RightCheek  UMETA(DisplayName = "Bochecha Dir."),
	Nose        UMETA(DisplayName = "Nariz"),
	Jaw         UMETA(DisplayName = "Mandibula"),
	Chin        UMETA(DisplayName = "Queixo"),
	LeftEye     UMETA(DisplayName = "Olho Esq."),
	RightEye    UMETA(DisplayName = "Olho Dir."),
	Mouth       UMETA(DisplayName = "Boca"),

	// Corpo
	Torso       UMETA(DisplayName = "Tronco"),
	LeftArm     UMETA(DisplayName = "Braco Esq."),
	RightArm    UMETA(DisplayName = "Braco Dir."),
	LeftLeg     UMETA(DisplayName = "Perna Esq."),
	RightLeg    UMETA(DisplayName = "Perna Dir.")
};

/** Fluxo de vida de uma rodada. Reset de dano so e permitido em Idle -> Intro. */
UENUM(BlueprintType)
enum class ELBMatchState : uint8
{
	Idle      UMETA(DisplayName = "Ocioso"),
	WarmingUp UMETA(DisplayName = "Aquecendo"),
	Intro     UMETA(DisplayName = "Entrada"),
	Running   UMETA(DisplayName = "Em Jogo"),
	Finishing UMETA(DisplayName = "Encerrando"),
	Results   UMETA(DisplayName = "Resultados")
};

UENUM(BlueprintType)
enum class ELBEventKind : uint8
{
	Gift    UMETA(DisplayName = "Presente"),
	Comment UMETA(DisplayName = "Comentario"),
	Like    UMETA(DisplayName = "Curtida"),
	Follow  UMETA(DisplayName = "Follow"),
	Join    UMETA(DisplayName = "Entrada")
};

/** Identidade congelada do espectador no momento do evento. */
USTRUCT(BlueprintType)
struct LBCORE_API FLBUser
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString UniqueId;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString Nickname;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString AvatarUrl;

	bool IsValid() const { return !Id.IsEmpty() || !UniqueId.IsEmpty(); }
};

/** Carga de um presente. `Count` e o numero de unidades deste evento. */
USTRUCT(BlueprintType)
struct LBCORE_API FLBGiftPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString GiftId;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString GiftName;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	int32 DiamondCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	int32 Count = 1;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString ComboId;

	/** Verdadeiro quando o streak terminou (ou quando o bridge forcou o fechamento). */
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	bool bComboEnd = true;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	bool bStreakable = false;
};

/**
 * Evento canonico. E a UNICA estrutura que o jogo entende da LIVE.
 * Nenhum conceito especifico de TikTok pode atravessar esta fronteira.
 */
USTRUCT(BlueprintType)
struct LBCORE_API FLBLiveEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	ELBEventKind Kind = ELBEventKind::Gift;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	int64 TimestampMs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FLBUser User;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FLBGiftPayload Gift;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString Comment;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown")
	FString Source;

	bool IsGift() const { return Kind == ELBEventKind::Gift; }
};
