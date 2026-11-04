#include "LBGameMode.h"

#include "LBMatchComponent.h"

ALBGameMode::ALBGameMode()
{
	Match = CreateDefaultSubobject<ULBMatchComponent>(TEXT("Match"));
}

void ALBGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart && Match)
	{
		Match->StartMatch();
	}
}

void ALBGameMode::StartMatch()
{
	if (Match)
	{
		Match->StartMatch();
	}
}

void ALBGameMode::FinishMatch()
{
	if (Match)
	{
		Match->RequestFinish();
	}
}

bool ALBGameMode::ResetCharacter()
{
	return Match ? Match->ResetCharacter() : false;
}
