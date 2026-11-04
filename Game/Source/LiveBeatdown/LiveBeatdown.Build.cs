using UnrealBuildTool;

public class LiveBeatdown : ModuleRules
{
	public LiveBeatdown(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"LBCore",
			"LBDamage",
			"LBPhysics",
			"LBCombat",
			"LBCharacter",
			"LBEvents",
			"LBMatch",
			"LBAudio",
			"LBUI",
			"LBAdmin"
		});
	}
}
