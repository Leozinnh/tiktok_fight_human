using UnrealBuildTool;
using System.Collections.Generic;

public class LiveBeatdownEditorTarget : TargetRules
{
	public LiveBeatdownEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
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
