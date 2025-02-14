// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

/**
 * Visualizer for the Actor I/O Component.
 */
class ACTORIOEDITOR_API FActorIOComponentVisualizer : public FComponentVisualizer
{
public:

	//~ Begin FComponentVisualizer
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//~ End FComponentVisualizer
};
