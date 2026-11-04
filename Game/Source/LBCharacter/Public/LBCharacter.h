#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LBCharacter.generated.h"

class ULBAttackComponent;
class ULBDamageComponent;
class ULBDamageVisualComponent;
class ULBEventDirector;
class ULBRagdollComponent;

/**
 * O alvo do programa.
 *
 * Composicao, nao heranca: o personagem e apenas o corpo. Dano, fisica, golpes e
 * direcao de eventos vivem em componentes independentes, o que permite
 * substituir o MetaHuman por outro corpo sem tocar na logica.
 *
 * Recebe a tag "BeatdownTarget", que o LBMatch usa para se vincular.
 */
UCLASS()
class LBCHARACTER_API ALBCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALBCharacter();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Character")
	ULBDamageComponent* GetDamage() const { return Damage; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Character")
	ULBAttackComponent* GetAttack() const { return Attack; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Character")
	ULBRagdollComponent* GetRagdoll() const { return Ragdoll; }

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Character")
	ULBEventDirector* GetDirector() const { return Director; }

	/** Entrada em cena. Implemente a animacao/camera no Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|Character")
	void OnEnterScene();

	/** Reacao facial de dor. Chamado apos cada impacto. */
	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|Character")
	void OnPainReaction(float Intensity);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LiveBeatdown|Character")
	TObjectPtr<ULBDamageComponent> Damage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LiveBeatdown|Character")
	TObjectPtr<ULBDamageVisualComponent> DamageVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LiveBeatdown|Character")
	TObjectPtr<ULBRagdollComponent> Ragdoll;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LiveBeatdown|Character")
	TObjectPtr<ULBAttackComponent> Attack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LiveBeatdown|Character")
	TObjectPtr<ULBEventDirector> Director;
};
