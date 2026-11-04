#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LBDamageState.h"
#include "LBDamageVisualComponent.generated.h"

class UMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Traduz o estado de dano em aparencia.
 *
 * Camada A: escalares de material (vermelhidao, hematoma, inchaco, suor).
 * Camada C: acessorios discretos (curativo, gelo) ligados por flag.
 * Camada B (malha de dano skinnada) e um MeshComponent extra apontado em
 * DamageLayerMesh — ele usa o mesmo esqueleto, entao acompanha a deformacao
 * facial sem "nadar" na pele.
 *
 * O material de pele precisa expor os parametros listados em GetParamName().
 */
UCLASS(ClassGroup = (LiveBeatdown), meta = (BlueprintSpawnableComponent))
class LBDAMAGE_API ULBDamageVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULBDamageVisualComponent();

	virtual void BeginPlay() override;

	/** Idempotente: reaplicar o mesmo estado nao altera o resultado. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Damage")
	void ApplyState(const FLBDamageState& InState);

	/** Malhas que recebem os parametros de dano. Vazio = skeletal mesh do dono. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Damage")
	TArray<TObjectPtr<UMeshComponent>> TargetMeshes;

	/** Malha de dano skinnada (camada B). Opcional. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Damage")
	TObjectPtr<UMeshComponent> DamageLayerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Damage")
	TObjectPtr<UMeshComponent> BandageMesh;

	/** Gancho para Blueprint cuidar de detalhes (particulas, morphs, olhos). */
	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|Damage")
	void OnDamageVisualsUpdated(const FLBDamageState& State);

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Damage")
	FLBDamageState GetLastState() const { return LastState; }

	static FName GetParamName(ELBDamageRegion Region);

private:
	void EnsureDynamicMaterials();
	void PushScalars(const FLBDamageState& State);
	void UpdateAccessories(const FLBDamageState& State);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	UPROPERTY(Transient)
	FLBDamageState LastState;
};
