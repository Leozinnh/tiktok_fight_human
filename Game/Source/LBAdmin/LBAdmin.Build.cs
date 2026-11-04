using UnrealBuildTool;

public class LBAdmin : ModuleRules
{
	public LBAdmin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"LBCore",
			"LBEvents",
			"LBMatch"
		});
	}
}
