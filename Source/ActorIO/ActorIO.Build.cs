// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

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
    }
}
