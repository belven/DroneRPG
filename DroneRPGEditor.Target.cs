// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DroneRPGEditorTarget : TargetRules
{
	public DroneRPGEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("DroneRPG");
        bLegacyParentIncludePaths = true; // TODO update this - would require a lot of changes
        WindowsPlatform.bStrictConformanceMode = true;
        bValidateFormatStrings = true;
	}
}
