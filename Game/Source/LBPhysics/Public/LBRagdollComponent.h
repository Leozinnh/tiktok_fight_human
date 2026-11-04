#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LBTypes.h"
#include "LBRagdollComponent.generated.h"

class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class ELBRagdollPhase : uint8
{
	/** Totalmente animado. */
	Animated UMETA(DisplayName = "Animado"),
	/** Musculos parcialmente cedendo (cambaleio). */
	Partial UMETA(DisplayName = "Parcial"),
	/** Ragdoll completo, no chao. */
	Full UMETA(DisplayName = "Completo"),
	/** Voltando ao controle da animacao. */
	Recovering UMETA(DisplayName = "Recuperando")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLBOnRagdollStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLBOnRagdollRecovered);

/**
 * Controlador de reacao fisica em tres estagios.
 *
 * Leve  -> apenas animacao (nunca liga fisica; boneco de pano e o que queremos evitar).
 * Medio -> ragdoll parcial: tronco e bracos cedem, pernas ainda sustentam.
 * Pesado-> ragdoll total, colapso e recuperacao.
 *
 * O peso de fisica e interpolado por tick, nunca trocado de uma vez — e isso que
 * evita o "pop" na transicao.
 */
UCLASS(ClassGroup = (LiveBeatdown), meta = (BlueprintSpawnableComponent))
class LBPHYSICS_API ULBRagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULBRagdollComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Reage a um golpe.
	 * @param Tier      estagio de reacao
	 * @param Impulse   impulso em mundo (aplicado no osso)
	 * @param Bone      osso atingido
	 */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Physics")
	void React(ELBReactionTier Tier, FVector Impulse, FName Bone);

	/** Aborta o ragdoll e volta para animacao imediatamente. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Physics")
	void ForceRecover();

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Physics")
	ELBRagdollPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Physics")
	bool IsDown() const { return Phase == ELBRagdollPhase::Full; }

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Physics")
	FLBOnRagdollStarted OnRagdollStarted;

	UPROPERTY(BlueprintAssignable, Category = "LiveBeatdown|Physics")
	FLBOnRagdollRecovered OnRagdollRecovered;

	/** Osso raiz do ragdoll. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Physics")
	FName RootBone = TEXT("pelvis");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Physics", meta = (ClampMin = "0"))
	float PartialWeight = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Physics", meta = (ClampMin = "0.1"))
	float PartialDuration = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Physics", meta = (ClampMin = "0.1"))
	float FullDuration = 2.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Physics", meta = (ClampMin = "0.1"))
	float BlendInSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Physics", meta = (ClampMin = "0.1"))
	float BlendOutSpeed = 3.0f;

private:
	USkeletalMeshComponent* GetMesh() const;

	void SetTargetWeight(float NewTarget, ELBRagdollPhase NewPhase, float Duration);
	void EndSimulationIfIdle();

	UFUNCTION()
	void HandleHoldFinished();

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> CachedMesh;

	ELBRagdollPhase Phase = ELBRagdollPhase::Animated;

	float CurrentWeight = 0.0f;
	float TargetWeight = 0.0f;

	FTimerHandle HoldTimer;
};
