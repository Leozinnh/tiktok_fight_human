#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LBDamageState.h"
#include "LBStatistics.h"
#include "LBTypes.h"
#include "LBHudWidget.generated.h"

/**
 * Base do HUD vertical 1080x1920.
 *
 * O widget nao decide nada: recebe estado e desenha. Toda a regra de jogo fica
 * nos sistemas. Isso mantem a UI substituivel sem tocar em gameplay.
 */
UCLASS()
class LBUI_API ULBHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|HUD")
	void OnMatchStateChanged(ELBMatchState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|HUD")
	void OnDamageUpdated(const FLBDamageState& State, float NormalizedDamage);

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|HUD")
	void OnHitLanded(const FString& ViewerName, FName AttackId, int32 QueueDepth);

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|HUD")
	void OnQueueChanged(int32 PendingCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "LiveBeatdown|HUD")
	void OnMatchFinished(const FLBMatchStats& Stats);

	/** Formata numero grande no padrao de live: 1.2K, 3.4M. */
	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|HUD")
	static FText FormatCompactNumber(int64 Value);
};
