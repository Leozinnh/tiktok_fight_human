using UnrealBuildTool;

public class LBEvents : ModuleRules
{
	public LBEvents(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"JsonUtilities",
			"WebSockets",
			"LBCore",
			"LBCombat"
		});
	}
}
