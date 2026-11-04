#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LBDamageState.h"
#include "LBDamageComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLBOnDamageChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLBOnRegionDamaged, ELBDamageRegion, Region, float, Amount);

/**
 * Guardiao do estado de dano. Nao desenha nada — apenas acumula, decai e avisa.
 * A parte visual escuta ULBDamageVisualComponent.
 */
UCLASS(ClassGroup = (LiveBeatdown), meta = (BlueprintSpawnableComponent))
class LBDAMAGE_API ULBDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULBDamageComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Aplica um golpe. `Amount` esta em unidades onde 1.0 satura uma regiao.
	 * `Intensity` (0..1) governa tontura, dor e perda de equilibrio.
	 * Falloff < 0 usa o valor de tuning.
	 */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Damage")
	void ApplyHit(ELBDamageRegion PrimaryRegion, float Amount, float Intensity, float Falloff = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Damage")
	void AddToRegion(ELBDamageRegion Region, float Amount);

	/** So deve ser chamado por LBMatch no comeco de uma nova rodada. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Damage")
	void ResetDamage();

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Damage")
	FLBDamageState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Damage")
	float GetNormalizedDamage() const;

	/** Traduz um osso atingido (ex.: "head") na regiao correspondente. */
	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Damage")
	static ELBDamageRegion BoneToRegion(FName BoneName);

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Damage")
	FLBOnDamageChanged OnDamageChanged;

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Damage")
	FLBOnRegionDamaged OnRegionDamaged;

private:
	static void GetNeighbors(ELBDamageRegion Region, TArray<ELBDamageRegion>& Out);

	UPROPERTY(VisibleInstanceOnly, Category = "LiveBeatdown|Damage")
	FLBDamageState State;

	float DefaultFalloff = 0.35f;
	float BalanceRecoverPerSec = 0.02f;
	float StunDecayPerSec = 0.05f;
	float MaxTotalDamage = 6.0f;

	/** Evita broadcast por frame enquanto os valores decaem. */
	float BroadcastAccumulator = 0.0f;
};
