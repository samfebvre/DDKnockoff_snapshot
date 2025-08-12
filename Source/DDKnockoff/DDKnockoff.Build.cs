// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DDKnockoff : ModuleRules
{
	public DDKnockoff(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AnimationModifiers", "AnimationBlueprintLibrary"});
		PrivateDependencyModuleNames.AddRange( new string[] { "AIModule", "NavigationSystem", "UMG", "Slate", "SlateCore", "Niagara", "AITestSuite", } );
	}
}
