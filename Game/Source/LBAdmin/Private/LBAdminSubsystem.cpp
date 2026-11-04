#include "LBAdminSubsystem.h"

#include "LBConfigSubsystem.h"
#include "LBEventDirector.h"
#include "LBEventQueue.h"
#include "LBLiveEventReceiver.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FString ULBAdminSubsystem::GetStatusJson() const
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return TEXT("{}");
	}

	const ULBEventQueue* Queue = GI->GetSubsystem<ULBEventQueue>();
	const ULBLiveEventReceiver* Receiver = GI->GetSubsystem<ULBLiveEventReceiver>();

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> Bridge = MakeShared<FJsonObject>();
	Bridge->SetBoolField(TEXT("connected"), Receiver ? Receiver->IsConnected() : false);
	Bridge->SetNumberField(TEXT("messages"), Receiver ? Receiver->GetMessagesReceived() : 0);
	Bridge->SetNumberField(TEXT("parseErrors"), Receiver ? Receiver->GetParseErrors() : 0);
	Root->SetObjectField(TEXT("bridge"), Bridge);

	TSharedPtr<FJsonObject> EventQueue = MakeShared<FJsonObject>();
	EventQueue->SetNumberField(TEXT("pending"), Queue ? Queue->Num() : 0);
	EventQueue->SetNumberField(TEXT("received"), static_cast<double>(Queue ? Queue->GetTotalReceived() : 0));
	EventQueue->SetNumberField(TEXT("duplicates"), static_cast<double>(Queue ? Queue->GetTotalDuplicates() : 0));
	Root->SetObjectField(TEXT("events"), EventQueue);

	FString Output;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	return Output;
}

bool ULBAdminSubsystem::ReloadConfig()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return false;
	}

	ULBConfigSubsystem* Config = GI->GetSubsystem<ULBConfigSubsystem>();
	return Config ? Config->ReloadAll() : false;
}
