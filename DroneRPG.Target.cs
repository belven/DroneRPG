// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DroneRPGTarget : TargetRules
{
	public DroneRPGTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("DroneRPG");
        CppStandard = CppStandardVersion.Default;
        WindowsPlatform.bStrictConformanceMode = true;
        bValidateFormatStrings = true;
    }
}
