#include "LBLiveEventReceiver.h"

#include "LBEventQueue.h"
#include "LBLog.h"

#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "WebSocketsModule.h"

void ULBLiveEventReceiver::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bSocketConnected = false;
}

void ULBLiveEventReceiver::Deinitialize()
{
	Disconnect();
	Super::Deinitialize();
}

ULBEventQueue* ULBLiveEventReceiver::GetQueue() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<ULBEventQueue>() : nullptr;
}

bool ULBLiveEventReceiver::Connect(const FString& Url)
{
	Disconnect();

	if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
	{
		FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
	}

	Socket = FWebSocketsModule::Get().CreateWebSocket(Url);
	if (!Socket.IsValid())
	{
		UE_LOG(LogLBEvents, Error, TEXT("Falha ao criar WebSocket para %s"), *Url);
		return false;
	}

	Socket->OnConnected().AddUObject(this, &ULBLiveEventReceiver::HandleConnected);
	Socket->OnConnectionError().AddUObject(this, &ULBLiveEventReceiver::HandleConnectionError);
	Socket->OnClosed().AddUObject(this, &ULBLiveEventReceiver::HandleClosed);
	Socket->OnMessage().AddUObject(this, &ULBLiveEventReceiver::HandleMessage);

	UE_LOG(LogLBEvents, Log, TEXT("Conectando ao bridge em %s"), *Url);
	Socket->Connect();

	return true;
}

void ULBLiveEventReceiver::Disconnect()
{
	if (Socket.IsValid())
	{
		Socket->OnConnected().Clear();
		Socket->OnConnectionError().Clear();
		Socket->OnClosed().Clear();
		Socket->OnMessage().Clear();
		Socket->Close();
		Socket.Reset();
	}

	bSocketConnected = false;
}

void ULBLiveEventReceiver::InjectEvent(const FLBLiveEvent& Event)
{
	if (ULBEventQueue* Queue = GetQueue())
	{
		Queue->Push(Event);
	}
}

bool ULBLiveEventReceiver::IsConnected() const
{
	return bSocketConnected;
}

int32 ULBLiveEventReceiver::GetMessagesReceived() const
{
	return MessageCounter.GetValue();
}

int32 ULBLiveEventReceiver::GetParseErrors() const
{
	return ParseErrorCounter.GetValue();
}

void ULBLiveEventReceiver::HandleConnected()
{
	bSocketConnected = true;
	UE_LOG(LogLBEvents, Log, TEXT("Bridge conectado"));
}

void ULBLiveEventReceiver::HandleConnectionError(const FString& Error)
{
	bSocketConnected = false;
	UE_LOG(LogLBEvents, Warning, TEXT("Bridge: erro de conexao: %s"), *Error);
}

void ULBLiveEventReceiver::HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	bSocketConnected = false;
	UE_LOG(LogLBEvents, Warning, TEXT("Bridge: conexao fechada (%d, %s)"),
		StatusCode, *Reason);
}

void ULBLiveEventReceiver::HandleMessage(const FString& Message)
{
	// ATENCAO: esta funcao roda na thread de rede. Nao tocar em UObject aqui
	// alem do que for thread-safe.
	MessageCounter.Increment();

	FLBLiveEvent Event;
	if (!ParseEventJson(Message, Event))
	{
		ParseErrorCounter.Increment();
		return;
	}

	if (ULBEventQueue* Queue = GetQueue())
	{
		Queue->Push(Event);
	}
}

bool ULBLiveEventReceiver::ParseEventJson(const FString& Json, FLBLiveEvent& OutEvent)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	FString Kind;
	if (!Root->TryGetStringField(TEXT("kind"), Kind))
	{
		// Mensagens de controle do bridge (hello, ping, stats) nao sao eventos.
		return false;
	}

	OutEvent = FLBLiveEvent();
	Root->TryGetStringField(TEXT("id"), OutEvent.Id);

	double Timestamp = 0.0;
	if (Root->TryGetNumberField(TEXT("ts"), Timestamp))
	{
		OutEvent.TimestampMs = static_cast<int64>(Timestamp);
	}

	if (Kind.Equals(TEXT("gift"), ESearchCase::IgnoreCase))
	{
		OutEvent.Kind = ELBEventKind::Gift;
	}
	else if (Kind.Equals(TEXT("comment"), ESearchCase::IgnoreCase))
	{
		OutEvent.Kind = ELBEventKind::Comment;
	}
	else if (Kind.Equals(TEXT("like"), ESearchCase::IgnoreCase))
	{
		OutEvent.Kind = ELBEventKind::Like;
	}
	else if (Kind.Equals(TEXT("follow"), ESearchCase::IgnoreCase))
	{
		OutEvent.Kind = ELBEventKind::Follow;
	}
	else if (Kind.Equals(TEXT("join"), ESearchCase::IgnoreCase))
	{
		OutEvent.Kind = ELBEventKind::Join;
	}
	else
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* UserObj = nullptr;
	if (Root->TryGetObjectField(TEXT("user"), UserObj) && UserObj && UserObj->IsValid())
	{
		(*UserObj)->TryGetStringField(TEXT("id"), OutEvent.User.Id);
		(*UserObj)->TryGetStringField(TEXT("uniqueId"), OutEvent.User.UniqueId);
		(*UserObj)->TryGetStringField(TEXT("nickname"), OutEvent.User.Nickname);
		(*UserObj)->TryGetStringField(TEXT("avatar"), OutEvent.User.AvatarUrl);
	}

	const TSharedPtr<FJsonObject>* GiftObj = nullptr;
	if (Root->TryGetObjectField(TEXT("gift"), GiftObj) && GiftObj && GiftObj->IsValid())
	{
		const TSharedPtr<FJsonObject>& Gift = *GiftObj;
		Gift->TryGetStringField(TEXT("id"), OutEvent.Gift.GiftId);
		Gift->TryGetStringField(TEXT("name"), OutEvent.Gift.GiftName);
		Gift->TryGetStringField(TEXT("comboId"), OutEvent.Gift.ComboId);

		double Diamonds = 0.0;
		if (Gift->TryGetNumberField(TEXT("diamondCount"), Diamonds))
		{
			OutEvent.Gift.DiamondCount = static_cast<int32>(Diamonds);
		}

		double Count = 1.0;
		if (Gift->TryGetNumberField(TEXT("count"), Count))
		{
			OutEvent.Gift.Count = FMath::Max(1, static_cast<int32>(Count));
		}

		bool bFlag = false;
		if (Gift->TryGetBoolField(TEXT("comboEnd"), bFlag))
		{
			OutEvent.Gift.bComboEnd = bFlag;
		}
		if (Gift->TryGetBoolField(TEXT("streakable"), bFlag))
		{
			OutEvent.Gift.bStreakable = bFlag;
		}
	}

	const TSharedPtr<FJsonObject>* CommentObj = nullptr;
	if (Root->TryGetObjectField(TEXT("comment"), CommentObj) && CommentObj && CommentObj->IsValid())
	{
		(*CommentObj)->TryGetStringField(TEXT("text"), OutEvent.Comment);
	}

	const TSharedPtr<FJsonObject>* MetaObj = nullptr;
	if (Root->TryGetObjectField(TEXT("meta"), MetaObj) && MetaObj && MetaObj->IsValid())
	{
		(*MetaObj)->TryGetStringField(TEXT("source"), OutEvent.Source);
	}

	if (OutEvent.Id.IsEmpty())
	{
		// Sem id nao ha deduplicacao possivel; sintetiza um.
		OutEvent.Id = FString::Printf(TEXT("auto_%lld_%s_%s"),
			OutEvent.TimestampMs, *OutEvent.User.UniqueId, *OutEvent.Gift.GiftName);
	}

	return true;
}
