#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LBTypes.h"
#include "LBEventDirector.generated.h"

class ULBAttackComponent;
class ULBEventQueue;
class ULBConfigSubsystem;

/** Um golpe esperando na fila. */
USTRUCT()
struct FLBQueuedAttack
{
	GENERATED_BODY()

	UPROPERTY()
	FName AttackId = NAME_None;

	UPROPERTY()
	FString ViewerName;

	UPROPERTY()
	float DamageMultiplier = 1.0f;

	UPROPERTY()
	float Intensity = 0.0f;

	/** Quantos slots este item ja esperou (anti-inanicao). */
	UPROPERTY()
	int32 Age = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLBOnQueueChanged, int32, PendingCount);

/**
 * O diretor.
 *
 * Traduz eventos em uma linha do tempo de golpes, respeitando prioridade,
 * anti-inanicao, agrupamento, combo e modo hype.
 *
 * Invariante: nenhum presente e descartado silenciosamente. Se a fila enche,
 * os dados sao agregados; se vira hype, o total e contabilizado.
 */
UCLASS(ClassGroup = (LiveBeatdown), meta = (BlueprintSpawnableComponent))
class LBEVENTS_API ULBEventDirector : public UActorComponent
{
	GENERATED_BODY()

public:
	ULBEventDirector();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Liga/desliga o consumo. Rodada parada ainda enfileira, mas nao golpeia. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Events")
	void SetRunning(bool bInRunning);

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	bool IsRunning() const { return bRunning; }

	/** Injeta um golpe manual (painel / testes). Respeita combo e multiplicador. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Events")
	int32 EnqueueManualAttack(FName AttackId, const FString& ViewerName, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Events")
	void ClearPending();

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	int32 GetPendingCount() const { return Pending.Num(); }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	int64 GetTotalGiftsCounted() const { return TotalGiftsCounted; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	int64 GetTotalAttacksExecuted() const { return TotalAttacksExecuted; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	int64 GetHypeConsumedCount() const { return HypeConsumedCount; }

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Events")
	FLBOnQueueChanged OnQueueChanged;

	/** Quantos eventos drenar da fila por tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Events", meta = (ClampMin = "1"))
	int32 MaxEventsPerTick = 64;

private:
	ULBEventQueue* GetQueue() const;
	ULBConfigSubsystem* GetConfig() const;

	void ConsumeEvents();
	void ExpandGift(const FLBLiveEvent& Event);
	void QueueAttack(FName AttackId, const FString& ViewerName, float Multiplier, float Intensity);
	void TryHype();
	void TryExecuteNext();

	int32 PickBestIndex() const;
	FString ResolveViewerName(const FLBUser& User) const;

	UPROPERTY(Transient)
	TObjectPtr<ULBAttackComponent> AttackComp;

	TArray<FLBQueuedAttack> Pending;

	bool bRunning = true;

	int64 TotalGiftsCounted = 0;
	int64 TotalAttacksExecuted = 0;
	int64 HypeConsumedCount = 0;

	double LastHypeSeconds = -1000.0;
	int32 LastBroadcastPending = -1;
};
