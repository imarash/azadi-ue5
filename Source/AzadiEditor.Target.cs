// AZADI: Rise of the Dawn

using UnrealBuildTool;
using System.Collections.Generic;

public class AzadiEditorTarget : TargetRules
{
	public AzadiEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Azadi");
	}
}
