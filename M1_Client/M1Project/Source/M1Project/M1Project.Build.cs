using System.IO;
using UnrealBuildTool;

public class M1Project : ModuleRules
{
	public M1Project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "GameplayAbilities", // GAS 사용을 위해 추가
            "GameplayTags",      // GAS 사용을 위해 추가
            "GameplayTasks",     // GAS 사용을 위해 추가
            "NavigationSystem"   // AI 및 추격 구현을 위해 추가
		});

        PublicSystemLibraries.Add("ws2_32.lib");
        PublicSystemLibraries.Add("mswsock.lib");

        PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"M1Project",
			"M1Project/Variant_Platforming",
			"M1Project/Variant_Platforming/Animation",
			"M1Project/Variant_Combat",
			"M1Project/Variant_Combat/AI",
			"M1Project/Variant_Combat/Animation",
			"M1Project/Variant_Combat/Gameplay",
			"M1Project/Variant_Combat/Interfaces",
			"M1Project/Variant_Combat/UI",
			"M1Project/Variant_SideScrolling",
			"M1Project/Variant_SideScrolling/AI",
			"M1Project/Variant_SideScrolling/Gameplay",
			"M1Project/Variant_SideScrolling/Interfaces",
			"M1Project/Variant_SideScrolling/UI"
		});


        string CommonPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../Common/ContentsProtocol"));
        PublicIncludePaths.Add(CommonPath);
    }
}
