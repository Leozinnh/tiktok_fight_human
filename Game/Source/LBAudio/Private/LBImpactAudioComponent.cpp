#include "LBImpactAudioComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ULBImpactAudioComponent::ULBImpactAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULBImpactAudioComponent::PlayById(FName SoundId, FVector Location)
{
	if (SoundId.IsNone())
	{
		return;
	}

	const TObjectPtr<USoundBase>* Found = SoundBank.Find(SoundId);
	if (!Found || !*Found)
	{
		return;
	}

	const float Pitch = FMath::FRandRange(PitchRange.X, PitchRange.Y);
	const float Volume = FMath::FRandRange(VolumeRange.X, VolumeRange.Y);

	UGameplayStatics::PlaySoundAtLocation(
		this, *Found, Location, FRotator::ZeroRotator, Volume, Pitch, 0.0f, Attenuation, Concurrency);
}

void ULBImpactAudioComponent::PlayRandomFrom(const TArray<FName>& SoundIds, FVector Location)
{
	if (SoundIds.Num() == 0)
	{
		return;
	}

	const int32 Index = FMath::RandRange(0, SoundIds.Num() - 1);
	PlayById(SoundIds[Index], Location);
}
