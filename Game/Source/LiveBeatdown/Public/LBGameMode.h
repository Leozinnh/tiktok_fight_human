#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LBGameMode.generated.h"

class ULBMatchComponent;

/**
 * Ponto de entrada do jogo. Nao contem regra: apenas instala o ULBMatchComponent
 * e expoe atalhos para o painel.
 */
UCLASS()
class ALBGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALBGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Match")
	void StartMatch();

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Match")
	void FinishMatch();

	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Match")
	bool ResetCharacter();

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Match")
	ULBMatchComponent* GetMatch() const { return Match; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveBeatdown|Match")
	bool bAutoStart = true;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LiveBeatdown|Match")
	TObjectPtr<ULBMatchComponent> Match;
};
