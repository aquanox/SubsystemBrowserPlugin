// Copyright 2022, Aquanox.

using UnrealBuildTool;

public class SubsystemBrowser : ModuleRules
{
	public SubsystemBrowser(ReadOnlyTargetRules Target)
		: base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// These two to emulate engine installation and includes
        // bUseUnity = false;
        // bTreatAsEngineModule = true;

        PublicIncludePaths.Add(ModuleDirectory);
		PrivateIncludePaths.Add(ModuleDirectory);

		PrivateDependencyModuleNames.AddRange(new string []
		{
			"Core",
			"CoreUObject",
			"ApplicationCore",
			"Engine",
			"Slate",
			"SlateCore",
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
