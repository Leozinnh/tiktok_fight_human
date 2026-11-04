#include "LBConfigSubsystem.h"

#include "LBLog.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Animation/AnimMontage.h"

namespace
{
	float ReadFloat(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Key, float Default)
	{
		double Value = 0.0;
		return Obj->TryGetNumberField(Key, Value) ? static_cast<float>(Value) : Default;
	}

	int32 ReadInt(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Key, int32 Default)
	{
		double Value = 0.0;
		return Obj->TryGetNumberField(Key, Value) ? static_cast<int32>(Value) : Default;
	}

	bool ReadBool(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Key, bool Default)
	{
		bool Value = false;
		return Obj->TryGetBoolField(Key, Value) ? Value : Default;
	}

	FString ReadString(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Key, const FString& Default = FString())
	{
		FString Value;
		return Obj->TryGetStringField(Key, Value) ? Value : Default;
	}

	ELBDamageRegion ParseRegion(const FString& In)
	{
		const FString Key = In.ToLower();
		if (Key == TEXT("forehead"))   return ELBDamageRegion::Forehead;
		if (Key == TEXT("leftcheek"))  return ELBDamageRegion::LeftCheek;
		if (Key == TEXT("rightcheek")) return ELBDamageRegion::RightCheek;
		if (Key == TEXT("nose"))       return ELBDamageRegion::Nose;
		if (Key == TEXT("jaw"))        return ELBDamageRegion::Jaw;
		if (Key == TEXT("chin"))       return ELBDamageRegion::Chin;
		if (Key == TEXT("lefteye"))    return ELBDamageRegion::LeftEye;
		if (Key == TEXT("righteye"))   return ELBDamageRegion::RightEye;
		if (Key == TEXT("mouth"))      return ELBDamageRegion::Mouth;
		if (Key == TEXT("torso"))      return ELBDamageRegion::Torso;
		if (Key == TEXT("leftarm"))    return ELBDamageRegion::LeftArm;
		if (Key == TEXT("rightarm"))   return ELBDamageRegion::RightArm;
		if (Key == TEXT("leftleg"))    return ELBDamageRegion::LeftLeg;
		if (Key == TEXT("rightleg"))   return ELBDamageRegion::RightLeg;
		return ELBDamageRegion::None;
	}

	ELBReactionTier ParseTier(const FString& In)
	{
		const FString Key = In.ToLower();
		if (Key == TEXT("medium"))  return ELBReactionTier::Medium;
		if (Key == TEXT("heavy"))   return ELBReactionTier::Heavy;
		if (Key == TEXT("special")) return ELBReactionTier::Special;
		return ELBReactionTier::Light;
	}
}

void ULBConfigSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadAll();
}

bool ULBConfigSubsystem::LoadJsonFile(const FString& FileName, TSharedPtr<FJsonObject>& OutRoot) const
{
	const FString Path = FPaths::Combine(FPaths::ProjectConfigDir(), FileName);

	FString Raw;
	if (!FFileHelper::LoadFileToString(Raw, *Path))
	{
		UE_LOG(LogLB, Warning, TEXT("Config nao encontrada: %s"), *Path);
		return false;
	}

	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
	if (!FJsonSerializer::Deserialize(Reader, OutRoot) || !OutRoot.IsValid())
	{
		UE_LOG(LogLB, Error, TEXT("JSON invalido em %s"), *Path);
		return false;
	}

	return true;
}

bool ULBConfigSubsystem::ReloadAll()
{
	bool bAllOk = true;

	TSharedPtr<FJsonObject> AttackRoot;
	if (LoadJsonFile(TEXT("attacks.json"), AttackRoot))
	{
		ParseAttacks(AttackRoot);
	}
	else
	{
		bAllOk = false;
	}

	TSharedPtr<FJsonObject> GiftRoot;
	if (LoadJsonFile(TEXT("gifts.json"), GiftRoot))
	{
		ParseGifts(GiftRoot);
	}
	else
	{
		bAllOk = false;
	}

	TSharedPtr<FJsonObject> TuningRoot;
	if (LoadJsonFile(TEXT("tuning.json"), TuningRoot))
	{
		ParseTuning(TuningRoot);
	}
	else
	{
		bAllOk = false;
	}

	UE_LOG(LogLB, Log, TEXT("Config recarregada: %d golpes, %d presentes"),
		Attacks.Num(), GiftMappings.Num());

	return bAllOk;
}

void ULBConfigSubsystem::ParseAttacks(const TSharedPtr<FJsonObject>& Root)
{
	const TSharedPtr<FJsonObject>* AttacksObj = nullptr;
	if (!Root->TryGetObjectField(TEXT("attacks"), AttacksObj) || AttacksObj == nullptr)
	{
		UE_LOG(LogLB, Error, TEXT("attacks.json sem o objeto 'attacks'"));
		return;
	}

	Attacks.Reset();

	for (const auto& Pair : (*AttacksObj)->Values)
	{
		const TSharedPtr<FJsonObject>* EntryPtr = nullptr;
		if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(EntryPtr) || EntryPtr == nullptr)
		{
			continue;
		}
		const TSharedPtr<FJsonObject>& Entry = *EntryPtr;

		// Em UE 5.8 as chaves de FJsonObject::Values sao UE::FSharedString,
		// nao FString. Convertemos uma vez e trabalhamos com FString.
		const FString AttackKey(*Pair.Key);

		FLBAttackDef Def;
		Def.Id = FName(*AttackKey);
		Def.Label = FText::FromString(ReadString(Entry, TEXT("label"), AttackKey));

		const FString MontagePath = ReadString(Entry, TEXT("montage"));
		if (!MontagePath.IsEmpty())
		{
			Def.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(MontagePath));
		}

		const FString VfxPath = ReadString(Entry, TEXT("vfx"));
		if (!VfxPath.IsEmpty())
		{
			Def.Vfx = TSoftObjectPtr<UObject>(FSoftObjectPath(VfxPath));
		}

		Def.Intensity = FMath::Clamp(ReadFloat(Entry, TEXT("intensity"), Def.Intensity), 0.0f, 1.0f);
		Def.DamageMultiplier = FMath::Max(0.0f, ReadFloat(Entry, TEXT("damageMultiplier"), Def.DamageMultiplier));
		Def.PrimaryRegion = ParseRegion(ReadString(Entry, TEXT("primaryRegion")));
		Def.HitBone = FName(*ReadString(Entry, TEXT("hitBone")));
		Def.Impulse = FMath::Max(0.0f, ReadFloat(Entry, TEXT("impulse"), Def.Impulse));
		Def.ReactionTier = ParseTier(ReadString(Entry, TEXT("reactionTier"), TEXT("light")));
		Def.CameraShake = FMath::Max(0.0f, ReadFloat(Entry, TEXT("cameraShake"), Def.CameraShake));
		Def.CooldownMs = FMath::Max(0.0f, ReadFloat(Entry, TEXT("cooldownMs"), Def.CooldownMs));
		Def.SlotMs = FMath::Max(0.0f, ReadFloat(Entry, TEXT("slotMs"), 0.0f));

		const TArray<TSharedPtr<FJsonValue>>* AudioArray = nullptr;
		if (Entry->TryGetArrayField(TEXT("audio"), AudioArray) && AudioArray != nullptr)
		{
			for (const TSharedPtr<FJsonValue>& Value : *AudioArray)
			{
				if (!Value.IsValid())
				{
					continue;
				}
				const FString SoundId = Value->AsString();
				if (!SoundId.IsEmpty())
				{
					Def.AudioIds.Add(FName(*SoundId));
				}
			}
		}

		Attacks.Add(Def.Id, Def);
	}
}

void ULBConfigSubsystem::ParseGifts(const TSharedPtr<FJsonObject>& Root)
{
	const TSharedPtr<FJsonObject>* GiftsObj = nullptr;
	if (!Root->TryGetObjectField(TEXT("gifts"), GiftsObj) || GiftsObj == nullptr)
	{
		UE_LOG(LogLB, Error, TEXT("gifts.json sem o objeto 'gifts'"));
		return;
	}

	GiftMappings.Reset();

	for (const auto& Pair : (*GiftsObj)->Values)
	{
		const TSharedPtr<FJsonObject>* EntryPtr = nullptr;
		if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(EntryPtr) || EntryPtr == nullptr)
		{
			continue;
		}
		const TSharedPtr<FJsonObject>& Entry = *EntryPtr;

		FLBGiftMapping Mapping;
		Mapping.GiftName = FString(*Pair.Key);
		Mapping.AttackId = FName(*ReadString(Entry, TEXT("attack")));
		Mapping.DamageMultiplier = FMath::Max(0.0f, ReadFloat(Entry, TEXT("damageMultiplier"), 1.0f));
		Mapping.bEnabled = ReadBool(Entry, TEXT("enabled"), true);

		const FString CountMode = ReadString(Entry, TEXT("countMode"), TEXT("perUnit"));
		Mapping.CountMode = (CountMode.ToLower() == TEXT("single"))
			? ELBGiftCountMode::Single
			: ELBGiftCountMode::PerUnit;

		if (Mapping.AttackId.IsNone())
		{
			UE_LOG(LogLB, Warning, TEXT("Presente '%s' sem golpe associado; ignorado"), *Mapping.GiftName);
			continue;
		}

		GiftMappings.Add(NormalizeGiftKey(Mapping.GiftName), Mapping);
	}
}

void ULBConfigSubsystem::ParseTuning(const TSharedPtr<FJsonObject>& Root)
{
	const TSharedPtr<FJsonObject>* QueueObj = nullptr;
	if (Root->TryGetObjectField(TEXT("queue"), QueueObj) && QueueObj != nullptr)
	{
		const TSharedPtr<FJsonObject>& Q = *QueueObj;
		Tuning.Queue.SlotMs = FMath::Max(100.0f, ReadFloat(Q, TEXT("slotMs"), Tuning.Queue.SlotMs));
		Tuning.Queue.MaxPending = FMath::Max(1, ReadInt(Q, TEXT("maxPending"), Tuning.Queue.MaxPending));
		Tuning.Queue.AntiStarvationSlots = FMath::Max(1, ReadInt(Q, TEXT("antiStarvationSlots"), Tuning.Queue.AntiStarvationSlots));
		Tuning.Queue.MaxConcurrentAttacks = FMath::Max(1, ReadInt(Q, TEXT("maxConcurrentAttacks"), Tuning.Queue.MaxConcurrentAttacks));
	}

	const TSharedPtr<FJsonObject>* HypeObj = nullptr;
	if (Root->TryGetObjectField(TEXT("hype"), HypeObj) && HypeObj != nullptr)
	{
		const TSharedPtr<FJsonObject>& H = *HypeObj;
		Tuning.Hype.Threshold = FMath::Max(1, ReadInt(H, TEXT("threshold"), Tuning.Hype.Threshold));
		Tuning.Hype.CooldownMs = FMath::Max(0.0f, ReadFloat(H, TEXT("cooldownMs"), Tuning.Hype.CooldownMs));
	}

	const TSharedPtr<FJsonObject>* DamageObj = nullptr;
	if (Root->TryGetObjectField(TEXT("damage"), DamageObj) && DamageObj != nullptr)
	{
		const TSharedPtr<FJsonObject>& D = *DamageObj;
		Tuning.Damage.RegionFalloff = FMath::Clamp(ReadFloat(D, TEXT("regionFalloff"), Tuning.Damage.RegionFalloff), 0.0f, 1.0f);
		Tuning.Damage.BalanceRecoverPerSec = FMath::Max(0.0f, ReadFloat(D, TEXT("balanceRecoverPerSec"), Tuning.Damage.BalanceRecoverPerSec));
		Tuning.Damage.StunDecayPerSec = FMath::Max(0.0f, ReadFloat(D, TEXT("stunDecayPerSec"), Tuning.Damage.StunDecayPerSec));
		Tuning.Damage.FallThreshold = FMath::Clamp(ReadFloat(D, TEXT("fallThreshold"), Tuning.Damage.FallThreshold), 0.0f, 1.0f);
		Tuning.Damage.MaxTotalDamage = FMath::Max(1.0f, ReadFloat(D, TEXT("maxTotalDamage"), Tuning.Damage.MaxTotalDamage));
	}
}

bool ULBConfigSubsystem::FindAttack(FName AttackId, FLBAttackDef& OutAttack) const
{
	if (const FLBAttackDef* Found = Attacks.Find(AttackId))
	{
		OutAttack = *Found;
		return true;
	}
	return false;
}

bool ULBConfigSubsystem::FindGiftMapping(const FString& GiftName, FLBGiftMapping& OutMapping) const
{
	if (const FLBGiftMapping* Found = GiftMappings.Find(NormalizeGiftKey(GiftName)))
	{
		OutMapping = *Found;
		return true;
	}
	return false;
}

TArray<FName> ULBConfigSubsystem::GetAttackIds() const
{
	TArray<FName> Ids;
	Attacks.GetKeys(Ids);
	return Ids;
}

FString ULBConfigSubsystem::NormalizeGiftKey(const FString& InName)
{
	FString Out;
	Out.Reserve(InName.Len());
	for (const TCHAR C : InName)
	{
		if (FChar::IsAlnum(C))
		{
			Out.AppendChar(FChar::ToLower(C));
		}
	}
	return Out;
}
