using UnrealBuildTool;

public class AirBuild : ModuleRules
{
	public AirBuild(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"SML",
			"FactoryGame",
			// Rebindable air-place / reach controls via Enhanced Input
			// (per-mapping PlayerMappableKeySettings names, required for rebinding).
			"EnhancedInput",
			"InputCore",
			// Player-facing SML mod config (reach defaults, indicator placement).
			"DeveloperSettings",
			// HUD indicator widget (programmatic Slate tree).
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
