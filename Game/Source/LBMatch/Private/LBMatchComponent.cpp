#include "LBMatchComponent.h"

#include "LBAttackComponent.h"
#include "LBDamageComponent.h"
#include "LBEventDirector.h"
#include "LBLog.h"
#include "LBRagdollComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

ULBMatchComponent::ULBMatchComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULBMatchComponent::BeginPlay()
{
	Super::BeginPlay();
	BindTarget();
}

void ULBMatchComponent::BindTarget()
{
	Target = nullptr;
	AttackComp = nullptr;
	DirectorComp = nullptr;
	DamageComp = nullptr;
	RagdollComp = nullptr;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (!Candidate || !Candidate->ActorHasTag(TargetActorTag))
		{
			continue;
		}

		Target = Candidate;
		AttackComp = Candidate->FindComponentByClass<ULBAttackComponent>();
		DirectorComp = Candidate->FindComponentByClass<ULBEventDirector>();
		DamageComp = Candidate->FindComponentByClass<ULBDamageComponent>();
		RagdollComp = Candidate->FindComponentByClass<ULBRagdollComponent>();
		break;
	}

	if (!Target.IsValid())
	{
		UE_LOG(LogLBMatch, Warning, TEXT("Nenhum ator com a tag '%s' encontrado"), *TargetActorTag.ToString());
		return;
	}

	if (AttackComp)
	{
		AttackComp->OnAttackExecuted.AddDynamic(this, &ULBMatchComponent::HandleAttackExecuted);
	}

	if (DirectorComp)
	{
		DirectorComp->SetRunning(false);
	}

	UE_LOG(LogLBMatch, Log, TEXT("Alvo vinculado: %s"), *Target->GetName());
}

void ULBMatchComponent::HandleAttackExecuted(FName AttackId, ELBDamageRegion Region, const FString& ViewerName)
{
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (ULBStatisticsSubsystem* Stats = GI->GetSubsystem<ULBStatisticsSubsystem>())
		{
			Stats->RecordHit(ViewerName, 1, 1.0f);
		}
	}
}

FLBMatchStats ULBMatchComponent::GetStats() const
{
	if (const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (const ULBStatisticsSubsystem* Stats = GI->GetSubsystem<ULBStatisticsSubsystem>())
		{
			return Stats->GetCurrentStats();
		}
	}
	return FLBMatchStats();
}

void ULBMatchComponent::SetState(ELBMatchState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
	OnStateChanged.Broadcast(State);

	UE_LOG(LogLBMatch, Log, TEXT("Estado da rodada -> %d"), static_cast<int32>(State));
}

void ULBMatchComponent::ScheduleNext(float Delay, TFunction<void()> Callback)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(PhaseTimer);

	if (Delay <= 0.0f)
	{
		Callback();
		return;
	}

	FTimerDelegate Delegate;
	Delegate.BindLambda(MoveTemp(Callback));
	World->GetTimerManager().SetTimer(PhaseTimer, Delegate, Delay, false);
}

void ULBMatchComponent::StartMatch()
{
	if (State != ELBMatchState::Idle)
	{
		UE_LOG(LogLBMatch, Warning, TEXT("StartMatch ignorado: rodada em andamento"));
		return;
	}

	// Reset autorizado: ainda estamos em Idle, antes da nova rodada.
	ResetCharacter();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (ULBStatisticsSubsystem* Stats = GI->GetSubsystem<ULBStatisticsSubsystem>())
		{
			Stats->BeginMatch();
		}
	}

	BindTarget();
	SetState(ELBMatchState::WarmingUp);

	ScheduleNext(WarmupSeconds, [this]()
	{
		SetState(ELBMatchState::Intro);

		ScheduleNext(IntroSeconds, [this]()
		{
			if (DirectorComp)
			{
				DirectorComp->SetRunning(true);
			}
			SetState(ELBMatchState::Running);
		});
	});
}

void ULBMatchComponent::RequestFinish()
{
	if (State != ELBMatchState::Running)
	{
		return;
	}

	if (DirectorComp)
	{
		DirectorComp->SetRunning(false);
	}

	SetState(ELBMatchState::Finishing);

	ScheduleNext(FinishingSeconds, [this]()
	{
		FLBMatchStats Final;
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (ULBStatisticsSubsystem* Stats = GI->GetSubsystem<ULBStatisticsSubsystem>())
			{
				Final = Stats->EndMatch();
			}
		}

		OnMatchFinished.Broadcast(Final);
		SetState(ELBMatchState::Results);

		ScheduleNext(ResultsSeconds, [this]()
		{
			SetState(ELBMatchState::Idle);
		});
	});
}

bool ULBMatchComponent::ResetCharacter()
{
	if (State != ELBMatchState::Idle)
	{
		UE_LOG(LogLBMatch, Warning,
			TEXT("ResetCharacter bloqueado: so e permitido em Idle (estado atual %d)"),
			static_cast<int32>(State));
		return false;
	}

	if (DamageComp)
	{
		DamageComp->ResetDamage();
	}

	if (RagdollComp)
	{
		RagdollComp->ForceRecover();
	}

	UE_LOG(LogLBMatch, Log, TEXT("Personagem reiniciado"));
	return true;
}
