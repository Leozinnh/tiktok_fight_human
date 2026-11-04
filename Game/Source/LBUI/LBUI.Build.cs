using UnrealBuildTool;

public class LBUI : ModuleRules
{
	public LBUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"Slate",
			"SlateCore",
			"LBCore",
			"LBDamage",
			"LBMatch"
		});
	}
}
