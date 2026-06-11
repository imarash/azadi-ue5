// AZADI: Rise of the Dawn — core runtime module

using UnrealBuildTool;

public class Azadi : ModuleRules
{
	public Azadi(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"Json",
			"JsonUtilities"
		});

		PublicIncludePaths.AddRange(new string[] {
			"Azadi"
		});
	}
}
