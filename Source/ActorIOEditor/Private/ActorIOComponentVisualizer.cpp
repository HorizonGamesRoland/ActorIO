// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIOComponentVisualizer.h"
#include "ActorIOSystem.h"
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

	for (const TWeakObjectPtr<UActorIOAction>& OutputAction : IOComponent->GetActions())
	{
		if (OutputAction.IsValid() && IsValid(OutputAction->TargetActor))
		{
			const FVector Start = IOComponentOwner->GetActorLocation();
			const FVector End = OutputAction->TargetActor->GetActorLocation();
			PDI->DrawLine(Start, End, FColor(215, 255, 135).ReinterpretAsLinear(), SDPG_Foreground, LineThickness, 0.01f);
		}
	}

	for (const TWeakObjectPtr<UActorIOAction>& InputAction : UActorIOSystem::GetInputActionsForObject(IOComponentOwner))
	{
		if (InputAction.IsValid())
		{
			const AActor* ActionOwner = InputAction->GetOwnerActor();
			if (IsValid(ActionOwner))
			{
				const FVector Start = IOComponentOwner->GetActorLocation();
				const FVector End = ActionOwner->GetActorLocation();
				PDI->DrawLine(Start, End, FColor(255, 210, 135).ReinterpretAsLinear(), SDPG_Foreground, LineThickness);
			}
		}
	}
}
