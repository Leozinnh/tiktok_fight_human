#include "LBCharacter.h"

#include "LBAttackComponent.h"
#include "LBDamageComponent.h"
#include "LBDamageVisualComponent.h"
#include "LBEventDirector.h"
#include "LBRagdollComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ALBCharacter::ALBCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	Tags.Add(TEXT("BeatdownTarget"));

	// O personagem e uma vitrine: fica parado e reage, nao se move sozinho.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
		Movement->SetMovementMode(MOVE_None);
	}

	Damage = CreateDefaultSubobject<ULBDamageComponent>(TEXT("Damage"));
	DamageVisual = CreateDefaultSubobject<ULBDamageVisualComponent>(TEXT("DamageVisual"));
	Ragdoll = CreateDefaultSubobject<ULBRagdollComponent>(TEXT("Ragdoll"));
	Attack = CreateDefaultSubobject<ULBAttackComponent>(TEXT("Attack"));
	Director = CreateDefaultSubobject<ULBEventDirector>(TEXT("Director"));

	// A malha do MetaHuman e maior que a capsula padrao de um jogador.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCapsuleHalfHeight(95.0f);
		Capsule->SetCapsuleRadius(38.0f);
	}
}

void ALBCharacter::BeginPlay()
{
	Super::BeginPlay();

	OnEnterScene();
}
