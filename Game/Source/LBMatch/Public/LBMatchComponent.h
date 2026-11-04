#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LBTypes.h"
#include "LBStatistics.h"
#include "LBMatchComponent.generated.h"

class ULBAttackComponent;
class ULBEventDirector;
class ULBDamageComponent;
class ULBRagdollComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLBOnMatchStateChanged, ELBMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLBOnMatchFinished, FLBMatchStats, Stats);

/**
 * Dono do ciclo da rodada.
 *
 * Idle -> WarmingUp -> Intro -> Running -> Finishing -> Results -> Idle
 *
 * Unico ponto autorizado a zerar o dano: Idle, imediatamente antes de uma nova
 * rodada. Um golpe nunca reseta nada.
 */
UCLASS(ClassGroup = (LiveBeatdown), meta = (BlueprintSpawnableComponent))
class LBMATCH_API ULBMatchComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULBMatchComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Match")
	void StartMatch();

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Match")
	void RequestFinish();

	/** Volta o personagem ao estado original. So funciona em Idle. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Match")
	bool ResetCharacter();

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Match")
	ELBMatchState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Match")
	FLBMatchStats GetStats() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Match", meta = (ClampMin = "0"))
	float WarmupSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Match", meta = (ClampMin = "0"))
	float IntroSeconds = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Match", meta = (ClampMin = "0"))
	float FinishingSeconds = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Match", meta = (ClampMin = "0"))
	float ResultsSeconds = 12.0f;

	/** Tag usada para localizar o personagem atingido no nivel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Match")
	FName TargetActorTag = TEXT("BeatdownTarget");

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Match")
	FLBOnMatchStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Match")
	FLBOnMatchFinished OnMatchFinished;

private:
	UFUNCTION()
	void HandleAttackExecuted(FName AttackId, ELBDamageRegion Region, const FString& ViewerName);

	void SetState(ELBMatchState NewState);
	void BindTarget();
	void ScheduleNext(float Delay, TFunction<void()> Callback);

	UPROPERTY(Transient)
	TObjectPtr<ULBAttackComponent> AttackComp;

	UPROPERTY(Transient)
	TObjectPtr<ULBEventDirector> DirectorComp;

	UPROPERTY(Transient)
	TObjectPtr<ULBDamageComponent> DamageComp;

	UPROPERTY(Transient)
	TObjectPtr<ULBRagdollComponent> RagdollComp;

	TWeakObjectPtr<AActor> Target;

	ELBMatchState State = ELBMatchState::Idle;
	FTimerHandle PhaseTimer;
};
