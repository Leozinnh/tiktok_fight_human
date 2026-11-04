#include "LBDamageState.h"

void FLBDamageState::Reset()
{
	Forehead = LeftCheek = RightCheek = Nose = Jaw = Chin = 0.0f;
	LeftEye = RightEye = Mouth = 0.0f;
	Torso = LeftArm = RightArm = LeftLeg = RightLeg = 0.0f;
	Balance = 1.0f;
	Stun = 0.0f;
	Pain = 0.0f;
	bBlackEyeLeft = bBlackEyeRight = false;
	bBandage = bToothChip = bLipSplit = false;
}

float FLBDamageState::GetRawTotal() const
{
	return Forehead + LeftCheek + RightCheek + Nose + Jaw + Chin
		+ LeftEye + RightEye + Mouth
		+ Torso + LeftArm + RightArm + LeftLeg + RightLeg;
}

float FLBDamageState::GetNormalizedTotal(float MaxTotal) const
{
	return MaxTotal > KINDA_SMALL_NUMBER
		? FMath::Clamp(GetRawTotal() / MaxTotal, 0.0f, 1.0f)
		: 0.0f;
}

float& FLBDamageState::RegionRef(ELBDamageRegion Region)
{
	static float Dummy = 0.0f;

	switch (Region)
	{
	case ELBDamageRegion::Forehead:   return Forehead;
	case ELBDamageRegion::LeftCheek:  return LeftCheek;
	case ELBDamageRegion::RightCheek: return RightCheek;
	case ELBDamageRegion::Nose:       return Nose;
	case ELBDamageRegion::Jaw:        return Jaw;
	case ELBDamageRegion::Chin:       return Chin;
	case ELBDamageRegion::LeftEye:    return LeftEye;
	case ELBDamageRegion::RightEye:   return RightEye;
	case ELBDamageRegion::Mouth:      return Mouth;
	case ELBDamageRegion::Torso:      return Torso;
	case ELBDamageRegion::LeftArm:    return LeftArm;
	case ELBDamageRegion::RightArm:   return RightArm;
	case ELBDamageRegion::LeftLeg:    return LeftLeg;
	case ELBDamageRegion::RightLeg:   return RightLeg;
	default:
		Dummy = 0.0f;
		return Dummy;
	}
}

float FLBDamageState::GetRegion(ELBDamageRegion Region) const
{
	switch (Region)
	{
	case ELBDamageRegion::Forehead:   return Forehead;
	case ELBDamageRegion::LeftCheek:  return LeftCheek;
	case ELBDamageRegion::RightCheek: return RightCheek;
	case ELBDamageRegion::Nose:       return Nose;
	case ELBDamageRegion::Jaw:        return Jaw;
	case ELBDamageRegion::Chin:       return Chin;
	case ELBDamageRegion::LeftEye:    return LeftEye;
	case ELBDamageRegion::RightEye:   return RightEye;
	case ELBDamageRegion::Mouth:      return Mouth;
	case ELBDamageRegion::Torso:      return Torso;
	case ELBDamageRegion::LeftArm:    return LeftArm;
	case ELBDamageRegion::RightArm:   return RightArm;
	case ELBDamageRegion::LeftLeg:    return LeftLeg;
	case ELBDamageRegion::RightLeg:   return RightLeg;
	default:
		return 0.0f;
	}
}

void FLBDamageState::UpdateDerivedFlags()
{
	// Efeitos discretos nascem de limiares e nunca voltam sozinhos.
	bBlackEyeLeft  = bBlackEyeLeft  || LeftEye  >= 0.55f;
	bBlackEyeRight = bBlackEyeRight || RightEye >= 0.55f;
	bLipSplit      = bLipSplit      || Mouth    >= 0.50f;
	bToothChip     = bToothChip     || Mouth    >= 0.70f;
	bBandage       = bBandage       || (Forehead + LeftCheek + RightCheek) >= 1.20f;
}

void FLBDamageState::ClampAll()
{
	for (uint8 Index = 1; Index <= static_cast<uint8>(ELBDamageRegion::RightLeg); ++Index)
	{
		RegionRef(static_cast<ELBDamageRegion>(Index)) =
			FMath::Clamp(RegionRef(static_cast<ELBDamageRegion>(Index)), 0.0f, 1.0f);
	}

	Balance = FMath::Clamp(Balance, 0.0f, 1.0f);
	Stun = FMath::Clamp(Stun, 0.0f, 1.0f);
	Pain = FMath::Clamp(Pain, 0.0f, 1.0f);
}
