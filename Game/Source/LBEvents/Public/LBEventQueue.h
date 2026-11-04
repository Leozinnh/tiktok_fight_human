#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LBTypes.h"
#include "LBEventQueue.generated.h"

/**
 * Fronteira entre a thread de rede e a game thread.
 *
 * O receptor empilha de qualquer thread; o diretor drena na game thread.
 * Nunca ha espera de I/O na thread principal.
 *
 * Tambem faz deduplicacao por id (LRU limitado) para que reconexao do
 * WebSocket nao gere golpes duplicados.
 */
UCLASS()
class LBEVENTS_API ULBEventQueue : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Seguro para chamar de qualquer thread. */
	void Push(const FLBLiveEvent& Event);

	/** Drena ate Max eventos. Retorna quantos foram retirados. */
	int32 Drain(TArray<FLBLiveEvent>& OutEvents, int32 Max);

	/** Thread-safe. */
	int32 Num() const;

	void Reset();

	/** Total de eventos recebidos desde o inicio. */
	uint64 GetTotalReceived() const { return TotalReceived; }

	/** Total descartado por duplicidade. */
	uint64 GetTotalDuplicates() const { return TotalDuplicates; }

private:
	bool IsDuplicate_NoLock(const FString& EventId);

	mutable FCriticalSection Mutex;
	TArray<FLBLiveEvent> Pending;
	TSet<FString> SeenIds;
	TArray<FString> SeenOrder;

	uint64 TotalReceived = 0;
	uint64 TotalDuplicates = 0;

	static constexpr int32 MaxSeenIds = 5000;
};
