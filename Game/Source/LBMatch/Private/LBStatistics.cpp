#include "LBStatistics.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"

void ULBStatisticsSubsystem::BeginMatch()
{
	Viewers.Reset();
	Current = FLBMatchStats();

	const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	StartTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	bInMatch = true;
}

FLBViewerStats& ULBStatisticsSubsystem::FindOrAddViewer(const FString& ViewerName)
{
	FLBViewerStats& Stats = Viewers.FindOrAdd(ViewerName);
	if (Stats.Name.IsEmpty())
	{
		Stats.Name = ViewerName;
	}
	return Stats;
}

void ULBStatisticsSubsystem::RecordHit(const FString& ViewerName, int32 Units, float Damage)
{
	if (!bInMatch)
	{
		return;
	}

	const int32 Counted = FMath::Max(1, Units);
	Current.TotalHits += Counted;

	FLBViewerStats& Viewer = FindOrAddViewer(ViewerName.IsEmpty() ? TEXT("Espectador") : ViewerName);
	Viewer.Hits += Counted;
	Viewer.Gifts += Counted;
	Viewer.Damage += Damage;
}

void ULBStatisticsSubsystem::RecordCombo(int32 Units)
{
	Current.BestCombo = FMath::Max(Current.BestCombo, Units);
}

FLBMatchStats ULBStatisticsSubsystem::GetCurrentStats() const
{
	FLBMatchStats Snapshot = Current;

	const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	Snapshot.DurationSeconds = (World && bInMatch)
		? static_cast<float>(World->GetTimeSeconds() - StartTimeSeconds)
		: 0.0f;

	// Ranking ordenado por dano, depois por golpes.
	Viewers.GenerateValueArray(Snapshot.Leaderboard);
	Snapshot.Leaderboard.Sort([](const FLBViewerStats& A, const FLBViewerStats& B)
	{
		if (!FMath::IsNearlyEqual(A.Damage, B.Damage))
		{
			return A.Damage > B.Damage;
		}
		return A.Hits > B.Hits;
	});

	if (Snapshot.Leaderboard.Num() > 0)
	{
		Snapshot.TopViewer = Snapshot.Leaderboard[0].Name;
		Snapshot.TopViewerGifts = Snapshot.Leaderboard[0].Hits;
	}

	Snapshot.TotalGifts = 0;
	for (const FLBViewerStats& Viewer : Snapshot.Leaderboard)
	{
		Snapshot.TotalGifts += Viewer.Gifts;
	}

	return Snapshot;
}

FLBMatchStats ULBStatisticsSubsystem::EndMatch()
{
	bInMatch = false;
	return GetCurrentStats();
}
