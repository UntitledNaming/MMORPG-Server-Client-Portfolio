// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class M1 : ModuleRules
{
	public M1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "EnhancedInput",
            "GameplayTags",
            "AnimGraphRuntime",
            "UMG"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        string CommonPath1 = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../Common/Contents"));

        PublicIncludePaths.Add(CommonPath1);

        string CommonPath2 = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../Common/Network"));

        PublicIncludePaths.Add(CommonPath2);

        PublicIncludePaths.AddRange(new string[]
        {
                "M1"
        });
    }
}
