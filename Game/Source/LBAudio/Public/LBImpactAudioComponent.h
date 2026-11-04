#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LBImpactAudioComponent.generated.h"

class USoundBase;
class USoundAttenuation;
class USoundConcurrency;

/**
 * Toca os sons de impacto. O catalogo vem de config (ids) e o banco de sons e
 * preenchido no Blueprint, para nao acoplar assets ao codigo.
 *
 * A variacao vem de duas fontes: ids diferentes por golpe e aleatorizacao de
 * pitch/volume a cada disparo, para nao soar repetitivo.
 */
UCLASS(ClassGroup = (LiveBeatdown), meta = (BlueprintSpawnableComponent))
class LBAUDIO_API ULBImpactAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULBImpactAudioComponent();

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Audio")
	void PlayById(FName SoundId, FVector Location);

	/** Escolhe um id aleatorio da lista (usado pelo catalogo de golpes). */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Audio")
	void PlayRandomFrom(const TArray<FName>& SoundIds, FVector Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Audio")
	TMap<FName, TObjectPtr<USoundBase>> SoundBank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Audio")
	TObjectPtr<USoundAttenuation> Attenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Audio")
	TObjectPtr<USoundConcurrency> Concurrency;

	/** Variacao de pitch aplicada a cada disparo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Audio")
	FVector2D PitchRange = FVector2D(0.92f, 1.08f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Audio")
	FVector2D VolumeRange = FVector2D(0.9f, 1.0f);
};
