#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LBAttackDef.h"
#include "LBGiftMapping.h"
#include "LBTuning.h"
#include "LBConfigSubsystem.generated.h"

/**
 * Fonte unica de configuracao do jogo. Le JSON de Config/ e pode recarregar a
 * quente quando o painel administrativo escreve um arquivo novo.
 *
 * Nada de presente, golpe ou parametro fica hardcoded no codigo.
 */
UCLASS()
class LBCORE_API ULBConfigSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Rele attacks.json, gifts.json e tuning.json. Seguro chamar a qualquer momento. */
	UFUNCTION(BlueprintCallable, Category = "LiveBeatdown|Config")
	bool ReloadAll();

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Config")
	bool FindAttack(FName AttackId, FLBAttackDef& OutAttack) const;

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Config")
	bool FindGiftMapping(const FString& GiftName, FLBGiftMapping& OutMapping) const;

	UFUNCTION(BlueprintPure, Category = "LiveBeatdown|Config")
	TArray<FName> GetAttackIds() const;

	const FLBTuning& GetTuning() const { return Tuning; }

	/** Slug do presente -> ataque, para consulta rapida por nome normalizado. */
	static FString NormalizeGiftKey(const FString& InName);

private:
	bool LoadJsonFile(const FString& FileName, TSharedPtr<class FJsonObject>& OutRoot) const;

	void ParseAttacks(const TSharedPtr<class FJsonObject>& Root);
	void ParseGifts(const TSharedPtr<class FJsonObject>& Root);
	void ParseTuning(const TSharedPtr<class FJsonObject>& Root);

	TMap<FName, FLBAttackDef> Attacks;
	TMap<FString, FLBGiftMapping> GiftMappings;
	FLBTuning Tuning;
};
