// Copyright 2022, Aquanox.

using UnrealBuildTool;

public class SubsystemBrowser : ModuleRules
{
	public SubsystemBrowser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// This is to emulate engine installation and verify includes during development
		if (Target.Configuration == UnrealTargetConfiguration.DebugGame
			|| Target.Configuration == UnrealTargetConfiguration.Debug)
		{
			bUseUnity = false;
			bTreatAsEngineModule = true;
		}

		// This is to use non Public/Private folder system
		PublicIncludePaths.Add(ModuleDirectory);
		PrivateIncludePaths.Add(ModuleDirectory);

		// These are dependencies always needed if planning on expanding plugin
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
		});

		// These are dependencies that nobody should know of
		PrivateDependencyModuleNames.AddRange(new string []
		{
			"ApplicationCore",
			"InputCore",
			"UnrealEd",
			"EditorStyle",
			"EditorSubsystem",
			"WorkspaceMenuStructure",
			"WorldBrowser",
			"LevelEditor",
			"ToolMenus",
			"AssetTools",
			"Projects"
		});
	}
}
