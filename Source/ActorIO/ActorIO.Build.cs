// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

using System.IO;
using UnrealBuildTool;
public class ActorIO : ModuleRules
{
	public ActorIO(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject"
        });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Engine",
			"DeveloperSettings"
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
