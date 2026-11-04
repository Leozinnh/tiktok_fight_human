using UnrealBuildTool;

public class LBCombat : ModuleRules
{
	public LBCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"LBCore",
			"LBDamage",
			"LBPhysics"
		});
	}
}
