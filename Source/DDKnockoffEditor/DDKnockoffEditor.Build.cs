using UnrealBuildTool;

public class DDKnockoffEditor : ModuleRules
{
	public DDKnockoffEditor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"InputCore",
				"EditorSubsystem",
				"BlueprintGraph",
				"EditorScriptingUtilities",
				"DDKnockoff",
				"Blutility",
				"UMG",
				"UMGEditor",
			} );
	}
}