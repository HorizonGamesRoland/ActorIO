// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

using System.IO;
using UnrealBuildTool;

public class ActorIOEditor : ModuleRules
{
    public ActorIOEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
            "DataLayerEditor",
            "EditorSubsystem",
            "Projects",
            "Slate",
            "SlateCore",
            "InputCore",
            "PlacementMode",
            "WorkspaceMenuStructure",
            "ToolWidgets",
        });

        AddInstancedStructSupport(Target);
    }

    private void AddInstancedStructSupport(ReadOnlyTargetRules Target)
    {
        if (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion >= 5)
        {
            // UE 5.5+:
            // TInstancedStruct is moved to the CoreUObject module of the engine.
            // Add custom include path to match previous #include for InstancedStruct.h.
            PublicIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Runtime/CoreUObject/Public/StructUtils"));
        }
        else
        {
            // UE 5.0 - 5.4:
            // TInstancedStruct is part of the StructUtils plugin.
            // As such, add the required dependency.
            PublicDependencyModuleNames.Add("StructUtils");
        }
    }
}