#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LBAdminSubsystem.generated.h"

/**
 * Superficie de diagnostico do jogo para o painel administrativo.
 *
 * O painel e uma aplicacao web externa; este subsistema so monta um retrato do
 * estado interno em JSON. A ponte HTTP/WebSocket fica no bridge (Node), nao aqui.
 */
UCLASS()
class LBADMIN_API ULBAdminSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Estado atual: fila, conexao, contadores e rodada. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Admin")
	FString GetStatusJson() const;

	/** Recarrega attacks.json/gifts.json/tuning.json em tempo de execucao. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Admin")
	bool ReloadConfig();
};
