// Copyright 2024 Horizon Games. All Rights Reserved.

using UnrealBuildTool;

public class ActorIOEditor : ModuleRules
{
    public ActorIOEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // Opt-in to using the "Include-What-You-Use" mode as per engine plugin standard.
        // Implies that PCHUsage is set to UseExplicitOrSharedPCHs.
        //IWYUSupport = IWYUSupport.Full;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
        });

        PrivateDependencyModuleNames.Add("ActorIO");
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "LevelEditor",
            "PropertyEditor",
            "EditorSubsystem",
            "Projects",
            "Slate",
            "SlateCore",
            "InputCore",
            "WorkspaceMenuStructure",
            "ToolWidgets",
        });
    }
}