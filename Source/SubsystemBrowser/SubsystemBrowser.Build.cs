// Copyright 2022, Aquanox.

using UnrealBuildTool;

public class SubsystemBrowser : ModuleRules
{
	public bool bStrictIncludesCheck = false;

	public SubsystemBrowser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// This is to emulate engine installation and verify includes during development
		// Gives effect similar to BuildPlugin with -StrictIncludes
		if (bStrictIncludesCheck)
		{
			bUseUnity = false;
            PCHUsage = PCHUsageMode.NoPCHs;
			// Enable additional checks used for Engine modules
            bTreatAsEngineModule = true;
		}

		// This is to use non-Public/Private folder system
		PublicIncludePaths.Add(ModuleDirectory);
		// Same as above but exclusively for current module
		// PrivateIncludePaths.Add(ModuleDirectory);

		// These are dependencies always needed if planning on expanding plugin
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		// These are dependencies that nobody should know of
		PrivateDependencyModuleNames.AddRange(new string []
		{
			"Slate",
			"SlateCore",
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
