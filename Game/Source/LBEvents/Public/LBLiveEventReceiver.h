#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LBTypes.h"
#include "LBLiveEventReceiver.generated.h"

class IWebSocket;

/**
 * Conector com o bridge local (nao com o TikTok).
 *
 * O jogo nunca fala com a plataforma: recebe eventos canonicos por WebSocket em
 * loopback. Trocar TikTokLive por EulerStream nao muda nada aqui.
 *
 * Callbacks do WebSocket chegam em thread de rede; este objeto apenas valida e
 * empilha na ULBEventQueue (que e thread-safe).
 */
UCLASS()
class LBEVENTS_API ULBLiveEventReceiver : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Events")
	bool Connect(const FString& Url = TEXT("ws://127.0.0.1:8765"));

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Events")
	void Disconnect();

	/** Alimenta um evento direto na fila (modo simulacao do painel). */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Events")
	void InjectEvent(const FLBLiveEvent& Event);

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	bool IsConnected() const;

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	int32 GetMessagesReceived() const;

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Events")
	int32 GetParseErrors() const;

private:
	void HandleConnected();
	void HandleConnectionError(const FString& Error);
	void HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void HandleMessage(const FString& Message);

	class ULBEventQueue* GetQueue() const;

	TSharedPtr<IWebSocket> Socket;

	/** Escrito pela thread de rede, lido pela game thread. */
	FThreadSafeCounter MessageCounter;
	FThreadSafeCounter ParseErrorCounter;
	FThreadSafeBool bSocketConnected;

	static bool ParseEventJson(const FString& Json, FLBLiveEvent& OutEvent);
};
