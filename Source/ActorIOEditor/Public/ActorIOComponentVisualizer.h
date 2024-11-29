// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class ACTORIOEDITOR_API FActorIOComponentVisualizer : public FComponentVisualizer
{
public:

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
