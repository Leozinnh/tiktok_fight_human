#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LBAttackDef.h"
#include "LBAttackComponent.generated.h"

class ULBDamageComponent;
class ULBRagdollComponent;
class ULBDamageVisualComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FLBOnAttackExecuted, FName, AttackId, ELBDamageRegion, Region, const FString&, ViewerName);

/**
 * Executor de golpes. Vive no personagem atingido — o agressor e a camera em
 * primeira pessoa, entao nao existe um segundo ator para animar.
 *
 * Ciclo de um golpe:
 *   1. toca a montagem (visual do braco/arma entrando em quadro)
 *   2. no instante de impacto, aplica dano + impulso fisico + som + VFX
 *   3. libera o slot para o proximo item da fila
 */
UCLASS(ClassGroup = (LiveBeatdown), meta = (BlueprintSpawnableComponent))
class LBCOMBAT_API ULBAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULBAttackComponent();

	virtual void BeginPlay() override;

	/**
	 * Executa um golpe ja resolvido pelo diretor.
	 * @param AttackId          id em attacks.json
	 * @param DamageMultiplier  multiplicador final (presente x golpe)
	 * @param ViewerName        quem provocou, para HUD
	 */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Combat")
	bool ExecuteAttack(FName AttackId, float DamageMultiplier, const FString& ViewerName);

	/** Versao que ja recebe a definicao (usada pelo teste manual do painel). */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Combat")
	bool ExecuteAttackDef(const FLBAttackDef& Attack, float DamageMultiplier, const FString& ViewerName);

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Combat")
	bool IsBusy() const { return bBusy; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Combat")
	float GetSlotDurationSeconds() const;

	/** Duracao do ultimo slot executado, em segundos (o diretor usa isso). */
	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Combat")
	float GetLastSlotSeconds() const { return LastSlotSeconds; }

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Combat")
	FLBOnAttackExecuted OnAttackExecuted;

	// --- Ganchos de apresentacao ---

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|Combat")
	void OnPlayAttackMontage(const FLBAttackDef& Attack);

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|Combat")
	void OnSpawnImpactVfx(const FLBAttackDef& Attack, FVector ImpactLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|Combat")
	void OnPlayImpactSound(const FLBAttackDef& Attack, FName SoundId);

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|Combat")
	void OnImpactCameraShake(const FLBAttackDef& Attack);

	/** Fracao do slot em que o impacto acontece (0..1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Combat", meta = (ClampMin = "0.05", ClampMax = "1"))
	float ImpactDelayFraction = 0.35f;

	/** Dano base por golpe, em unidades de regiao (1.0 satura uma regiao). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Combat", meta = (ClampMin = "0.001"))
	float BaseDamageUnits = 0.14f;

private:
	UFUNCTION()
	void HandleDamageChanged();

	void HandleImpact();
	void FinishSlot();
	float ComputeSlotSeconds(const FLBAttackDef& Attack) const;
	FVector ComputeImpactDirection() const;

	UPROPERTY(Transient)
	TObjectPtr<ULBDamageComponent> DamageComp;

	UPROPERTY(Transient)
	TObjectPtr<ULBRagdollComponent> RagdollComp;

	UPROPERTY(Transient)
	TObjectPtr<ULBDamageVisualComponent> VisualComp;

	FLBAttackDef PendingAttack;
	float PendingMultiplier = 1.0f;
	FString PendingViewer;

	bool bBusy = false;
	float LastSlotSeconds = 0.9f;

	FTimerHandle ImpactTimer;
	FTimerHandle SlotTimer;
};
