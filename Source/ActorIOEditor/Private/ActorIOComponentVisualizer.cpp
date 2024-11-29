// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOComponentVisualizer.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"

void FActorIOComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UActorIOComponent* IOComponent = Cast<const UActorIOComponent>(Component);
	if (!IOComponent)
	{
		return;
	}

	const AActor* IOComponentOwner = IOComponent->GetOwner();
	const float LineThickness = 3.0f;

	for (const TObjectPtr<UActorIOAction>& Action : IOComponent->GetActions())
	{
		if (Action->TargetActor)
		{
			const FVector Start = IOComponentOwner->GetActorLocation();
			const FVector End = Action->TargetActor->GetActorLocation();
			PDI->DrawLine(Start, End, FColor(215, 255, 135).ReinterpretAsLinear(), SDPG_Foreground, LineThickness, 0.01f);
		}
	}

	for (const TObjectPtr<UActorIOAction>& Action : UActorIOComponent::GetInputActionsForObject(IOComponentOwner))
	{
		const AActor* ActionOwner = Action->GetOwnerActor();
		if (ActionOwner)
		{
			const FVector Start = IOComponentOwner->GetActorLocation();
			const FVector End = ActionOwner->GetActorLocation();
			PDI->DrawLine(Start, End, FColor(255, 210, 135).ReinterpretAsLinear(), SDPG_Foreground, LineThickness);
		}
	}
}
