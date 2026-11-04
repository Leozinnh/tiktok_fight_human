using UnrealBuildTool;
using System.Collections.Generic;

public class LiveBeatdownTarget : TargetRules
{
	public LiveBeatdownTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppStandard = CppStandardVersion.Cpp20;

		ExtraModuleNames.AddRange(new string[]
		{
			"LBCore", "LBDamage", "LBPhysics", "LBCombat", "LBCharacter",
			"LBEvents", "LBMatch", "LBAudio", "LBUI", "LBAdmin", "LiveBeatdown"
		});
	}
}
