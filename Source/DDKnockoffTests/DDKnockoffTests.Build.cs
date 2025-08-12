using UnrealBuildTool;

public class DDKnockoffTests : ModuleRules
{
    public DDKnockoffTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
	        new[]
	        {
		        "CoreUObject",
		        "Engine",
		        "UMG",
		        "Slate",
		        "SlateCore",
		        "DDKnockoff",
		        "AutomationController",
		        "AutomationUtils",
		        "NavigationSystem",
		        "UnrealEd",
		        "FunctionalTesting",
	        }
        );
    }
}