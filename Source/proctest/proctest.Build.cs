// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class proctest : ModuleRules
{
	public proctest(ReadOnlyTargetRules Target) : base(Target)
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
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"proctest",
			"proctest/Variant_Platforming",
			"proctest/Variant_Platforming/Animation",
			"proctest/Variant_Combat",
			"proctest/Variant_Combat/AI",
			"proctest/Variant_Combat/Animation",
			"proctest/Variant_Combat/Gameplay",
			"proctest/Variant_Combat/Interfaces",
			"proctest/Variant_Combat/UI",
			"proctest/Variant_SideScrolling",
			"proctest/Variant_SideScrolling/AI",
			"proctest/Variant_SideScrolling/Gameplay",
			"proctest/Variant_SideScrolling/Interfaces",
			"proctest/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
