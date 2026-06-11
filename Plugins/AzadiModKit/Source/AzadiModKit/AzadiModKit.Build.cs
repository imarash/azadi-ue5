// AZADI: Rise of the Dawn — ModKit plugin

using UnrealBuildTool;

public class AzadiModKit : ModuleRules
{
	public AzadiModKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"JsonUtilities",
			// Intentional: the ModKit layers content into the game's registry.
			"Azadi"
		});
	}
}
