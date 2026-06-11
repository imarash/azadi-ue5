// AZADI: Rise of the Dawn

using UnrealBuildTool;
using System.Collections.Generic;

public class AzadiTarget : TargetRules
{
	public AzadiTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Azadi");
	}
}
