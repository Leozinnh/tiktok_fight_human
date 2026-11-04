using UnrealBuildTool;

public class LBCharacter : ModuleRules
{
	public LBCharacter(ReadOnlyTargetRules Target) : base(Target)
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
			"LBCombat",
			"LBEvents"
		});
	}
}
