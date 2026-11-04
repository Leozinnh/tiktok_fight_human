#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LBStatistics.generated.h"

/** Contribuicao de um espectador na rodada. */
USTRUCT(BlueprintType)
struct LBMATCH_API FLBViewerStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	int32 Gifts = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	int32 Hits = 0;

	/** Dano aproximado em unidades de regiao. */
	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	float Damage = 0.0f;
};

/** Resultado consolidado de uma rodada. */
USTRUCT(BlueprintType)
struct LBMATCH_API FLBMatchStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	int32 TotalHits = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	int32 TotalGifts = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	FString TopViewer;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	int32 TopViewerGifts = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	int32 BestCombo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	float DurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "LiveBeatdown|Stats")
	TArray<FLBViewerStats> Leaderboard;
};

/**
 * Acumula contribuicoes da rodada corrente. Vive no GameInstance para
 * sobreviver a troca de mapa e alimentar o HUD de resultados.
 */
UCLASS()
class LBMATCH_API ULBStatisticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Stats")
	void BeginMatch();

	/** Registra um golpe efetivamente aplicado. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Stats")
	void RecordHit(const FString& ViewerName, int32 Units, float Damage);

	/** Registra um combo (maior contagem vista na rodada). */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Stats")
	void RecordCombo(int32 Units);

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Stats")
	FLBMatchStats EndMatch();

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Stats")
	FLBMatchStats GetCurrentStats() const;

private:
	FLBViewerStats& FindOrAddViewer(const FString& ViewerName);

	TMap<FString, FLBViewerStats> Viewers;
	FLBMatchStats Current;
	double StartTimeSeconds = 0.0;
	bool bInMatch = false;
};
