#include "LBEventQueue.h"

void ULBEventQueue::Push(const FLBLiveEvent& Event)
{
	FScopeLock Lock(&Mutex);

	++TotalReceived;

	if (!Event.Id.IsEmpty() && IsDuplicate_NoLock(Event.Id))
	{
		++TotalDuplicates;
		return;
	}

	Pending.Add(Event);
}

bool ULBEventQueue::IsDuplicate_NoLock(const FString& EventId)
{
	if (SeenIds.Contains(EventId))
	{
		return true;
	}

	SeenIds.Add(EventId);
	SeenOrder.Add(EventId);

	// LRU simples: quando estoura, esquece os mais antigos.
	if (SeenOrder.Num() > MaxSeenIds)
	{
		const int32 ToRemove = SeenOrder.Num() - MaxSeenIds;
		for (int32 Index = 0; Index < ToRemove; ++Index)
		{
			SeenIds.Remove(SeenOrder[Index]);
		}
		SeenOrder.RemoveAt(0, ToRemove, EAllowShrinking::No);
	}

	return false;
}

int32 ULBEventQueue::Drain(TArray<FLBLiveEvent>& OutEvents, int32 Max)
{
	FScopeLock Lock(&Mutex);

	const int32 Count = FMath::Min(Max, Pending.Num());
	if (Count <= 0)
	{
		return 0;
	}

	OutEvents.Reserve(OutEvents.Num() + Count);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		OutEvents.Add(MoveTemp(Pending[Index]));
	}

	Pending.RemoveAt(0, Count, EAllowShrinking::No);
	return Count;
}

int32 ULBEventQueue::Num() const
{
	FScopeLock Lock(&Mutex);
	return Pending.Num();
}

void ULBEventQueue::Reset()
{
	FScopeLock Lock(&Mutex);
	Pending.Reset();
	SeenIds.Reset();
	SeenOrder.Reset();
}
