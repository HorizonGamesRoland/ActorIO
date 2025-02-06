// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

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

	AActor* IOComponentOwner = IOComponent->GetOwner();
	constexpr float LineThickness = 3.0f;

	for (const TObjectPtr<UActorIOAction>& OutputAction : IOComponent->GetActions())
	{
		if (OutputAction && IsValid(OutputAction->TargetActor))
		{
			const FVector Start = IOComponentOwner->GetActorLocation();
			const FVector End = OutputAction->TargetActor->GetActorLocation();
			PDI->DrawLine(Start, End, FColor(150, 255, 80).ReinterpretAsLinear(), SDPG_Foreground, LineThickness, 0.01f);
		}
	}

	for (UActorIOAction* InputAction : IActorIO::GetInputActionsForObject(IOComponentOwner))
	{
		if (InputAction)
		{
			const AActor* ActionOwner = InputAction->GetOwnerActor();
			if (IsValid(ActionOwner))
			{
				const FVector Start = IOComponentOwner->GetActorLocation();
				const FVector End = ActionOwner->GetActorLocation();
				PDI->DrawLine(Start, End, FColor(255, 200, 80).ReinterpretAsLinear(), SDPG_Foreground, LineThickness);
			}
		}
	}
}
