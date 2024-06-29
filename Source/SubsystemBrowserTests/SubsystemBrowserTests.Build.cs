// Copyright 2022, Aquanox.

using UnrealBuildTool;

public class SubsystemBrowserTests : ModuleRules
{
    public SubsystemBrowserTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(ModuleDirectory);
        
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "SubsystemBrowser",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            
        });
    }
}