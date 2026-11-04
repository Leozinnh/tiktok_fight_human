using UnrealBuildTool;

public class LBMatch : ModuleRules
{
	public LBMatch(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"LBCore",
			"LBDamage",
			"LBPhysics",
			"LBEvents",
			"LBCombat"
		});
	}
}
