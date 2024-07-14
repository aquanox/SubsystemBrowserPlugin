// Copyright 2022, Aquanox.

using UnrealBuildTool;

public class SubsystemBrowserTests : ModuleRules
{
	public bool bStrictIncludesCheck = false;

    public SubsystemBrowserTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "UMG",
            "SubsystemBrowser"
        });
    }
}