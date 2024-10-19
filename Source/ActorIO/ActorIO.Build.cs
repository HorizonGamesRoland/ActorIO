// Copyright 2024 Horizon Games. All Rights Reserved.

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
			"Engine"
		});
	}
}
