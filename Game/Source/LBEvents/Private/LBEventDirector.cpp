#include "LBEventDirector.h"

#include "LBAttackComponent.h"
#include "LBConfigSubsystem.h"
#include "LBEventQueue.h"
#include "LBLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

ULBEventDirector::ULBEventDirector()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void ULBEventDirector::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		AttackComp = Owner->FindComponentByClass<ULBAttackComponent>();
	}

	if (!AttackComp)
	{
		UE_LOG(LogLBEvents, Error, TEXT("EventDirector em '%s' sem ULBAttackComponent"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("null"));
	}
}

ULBEventQueue* ULBEventDirector::GetQueue() const
{
	const UWorld* World = GetWorld();
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<ULBEventQueue>() : nullptr;
}

ULBConfigSubsystem* ULBEventDirector::GetConfig() const
{
	const UWorld* World = GetWorld();
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<ULBConfigSubsystem>() : nullptr;
}

void ULBEventDirector::SetRunning(bool bInRunning)
{
	bRunning = bInRunning;
}

FString ULBEventDirector::ResolveViewerName(const FLBUser& User) const
{
	if (!User.Nickname.IsEmpty()) return User.Nickname;
	if (!User.UniqueId.IsEmpty()) return User.UniqueId;
	if (!User.Id.IsEmpty()) return User.Id;
	return TEXT("Espectador");
}

void ULBEventDirector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRunning)
	{
		return;
	}

	ConsumeEvents();
	TryHype();
	TryExecuteNext();

	if (Pending.Num() != LastBroadcastPending)
	{
		LastBroadcastPending = Pending.Num();
		OnQueueChanged.Broadcast(LastBroadcastPending);
	}
}

void ULBEventDirector::ConsumeEvents()
{
	ULBEventQueue* Queue = GetQueue();
	if (!Queue)
	{
		return;
	}

	TArray<FLBLiveEvent> Events;
	Queue->Drain(Events, MaxEventsPerTick);

	for (const FLBLiveEvent& Event : Events)
	{
		if (Event.IsGift())
		{
			ExpandGift(Event);
		}
		// Comentarios serao tratados pelo comment system (fase 4).
	}
}

void ULBEventDirector::ExpandGift(const FLBLiveEvent& Event)
{
	const ULBConfigSubsystem* Config = GetConfig();
	if (!Config)
	{
		return;
	}

	FLBGiftMapping Mapping;
	if (!Config->FindGiftMapping(Event.Gift.GiftName, Mapping) || !Mapping.bEnabled)
	{
		UE_LOG(LogLBEvents, Verbose, TEXT("Presente sem mapeamento: '%s'"), *Event.Gift.GiftName);
		return;
	}

	FLBAttackDef Attack;
	if (!Config->FindAttack(Mapping.AttackId, Attack))
	{
		UE_LOG(LogLBEvents, Warning, TEXT("Presente '%s' aponta para golpe inexistente '%s'"),
			*Event.Gift.GiftName, *Mapping.AttackId.ToString());
		return;
	}

	const int32 Units = FMath::Max(1, Event.Gift.Count);
	const FString Viewer = ResolveViewerName(Event.User);

	TotalGiftsCounted += Units;

	// Regra de streak: se o combo ainda nao terminou, o bridge manda apenas
	// incrementos. Ainda assim contabilizamos; quem fecha e o comboEnd.
	if (Mapping.CountMode == ELBGiftCountMode::Single)
	{
		// Um unico golpe, dano proporcional (com teto para nao explodir).
		const float Multiplier = Mapping.DamageMultiplier * FMath::Min(static_cast<float>(Units), 20.0f);
		QueueAttack(Attack.Id, Viewer, Multiplier, Attack.Intensity);
	}
	else
	{
		// Um golpe por unidade: 5 rosas = 5 pauladas com reacao propria.
		const int32 ToEnqueue = FMath::Min(Units, 50);
		for (int32 Index = 0; Index < ToEnqueue; ++Index)
		{
			QueueAttack(Attack.Id, Viewer, Mapping.DamageMultiplier, Attack.Intensity);
		}

		if (Units > ToEnqueue)
		{
			// O excedente vira multiplicador, nunca e perdido.
			QueueAttack(Attack.Id, Viewer,
				Mapping.DamageMultiplier * (Units - ToEnqueue), Attack.Intensity);
		}
	}
}

void ULBEventDirector::QueueAttack(FName AttackId, const FString& ViewerName, float Multiplier, float Intensity)
{
	const ULBConfigSubsystem* Config = GetConfig();
	const int32 MaxPending = Config ? Config->GetTuning().Queue.MaxPending : 200;

	if (Pending.Num() >= MaxPending)
	{
		// Fila cheia: agrega no item existente do mesmo espectador/golpe.
		// Preserva a contagem, que e o requisito.
		for (FLBQueuedAttack& Item : Pending)
		{
			if (Item.AttackId == AttackId && Item.ViewerName == ViewerName)
			{
				Item.DamageMultiplier += Multiplier;
				return;
			}
		}
	}

	FLBQueuedAttack Item;
	Item.AttackId = AttackId;
	Item.ViewerName = ViewerName;
	Item.DamageMultiplier = Multiplier;
	Item.Intensity = Intensity;
	Pending.Add(Item);
}

int32 ULBEventDirector::EnqueueManualAttack(FName AttackId, const FString& ViewerName, int32 Count)
{
	const ULBConfigSubsystem* Config = GetConfig();
	if (!Config)
	{
		return 0;
	}

	FLBAttackDef Attack;
	if (!Config->FindAttack(AttackId, Attack))
	{
		return 0;
	}

	const int32 Units = FMath::Clamp(Count, 1, 50);
	for (int32 Index = 0; Index < Units; ++Index)
	{
		QueueAttack(AttackId, ViewerName.IsEmpty() ? TEXT("Painel") : ViewerName, 1.0f, Attack.Intensity);
	}

	return Units;
}

void ULBEventDirector::ClearPending()
{
	Pending.Reset();
	OnQueueChanged.Broadcast(0);
}

int32 ULBEventDirector::PickBestIndex() const
{
	const ULBConfigSubsystem* Config = GetConfig();
	const int32 StarvationSlots = Config ? Config->GetTuning().Queue.AntiStarvationSlots : 12;

	int32 BestIndex = 0;
	float BestScore = -1.0f;

	for (int32 Index = 0; Index < Pending.Num(); ++Index)
	{
		float Score = Pending[Index].Intensity;

		// Anti-inanicao: item velho ganha prioridade absoluta.
		if (Pending[Index].Age >= StarvationSlots)
		{
			Score += 100.0f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestIndex = Index;
		}
	}

	return BestIndex;
}

void ULBEventDirector::TryExecuteNext()
{
	if (!AttackComp || AttackComp->IsBusy() || Pending.Num() == 0)
	{
		return;
	}

	const int32 Index = PickBestIndex();
	if (!Pending.IsValidIndex(Index))
	{
		return;
	}

	const FLBQueuedAttack Item = Pending[Index];
	Pending.RemoveAt(Index, 1, EAllowShrinking::No);

	for (FLBQueuedAttack& Remaining : Pending)
	{
		++Remaining.Age;
	}

	if (AttackComp->ExecuteAttack(Item.AttackId, Item.DamageMultiplier, Item.ViewerName))
	{
		++TotalAttacksExecuted;
	}
	else
	{
		// Falhou por corrida (componente ocupado). Devolve para a fila.
		Pending.Insert(Item, 0);
	}
}

void ULBEventDirector::TryHype()
{
	const ULBConfigSubsystem* Config = GetConfig();
	if (!Config || !AttackComp || AttackComp->IsBusy())
	{
		return;
	}

	const FLBTuning& Tuning = Config->GetTuning();
	if (Pending.Num() <= Tuning.Hype.Threshold)
	{
		return;
	}

	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (Now - LastHypeSeconds < (Tuning.Hype.CooldownMs / 1000.0))
	{
		return;
	}

	FLBAttackDef HypeAttack;
	const FName HypeId(TEXT("hype_burst"));
	if (!Config->FindAttack(HypeId, HypeAttack))
	{
		return;
	}

	// Consome o lote, somando o multiplicador de cada item.
	const int32 ToConsume = FMath::Min(Pending.Num(), Tuning.Hype.Threshold);
	float Multiplier = 0.0f;
	for (int32 Index = 0; Index < ToConsume; ++Index)
	{
		Multiplier += Pending[Index].DamageMultiplier;
	}
	Pending.RemoveAt(0, ToConsume, EAllowShrinking::No);

	HypeConsumedCount += ToConsume;
	LastHypeSeconds = Now;

	if (AttackComp->ExecuteAttack(HypeId, FMath::Max(1.0f, Multiplier), TEXT("HYPE")))
	{
		++TotalAttacksExecuted;
	}

	UE_LOG(LogLBEvents, Log, TEXT("HYPE: %d itens consumidos, multiplicador %.1f"), ToConsume, Multiplier);
}
