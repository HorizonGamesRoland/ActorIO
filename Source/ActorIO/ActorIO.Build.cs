// Copyright 2025 Horizon Games. All Rights Reserved.

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
            "CoreUObject",
			"DeveloperSettings"
        });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Engine"
        });
    }
}
