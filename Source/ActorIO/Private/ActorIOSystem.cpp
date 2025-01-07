// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBase.h"
#include "Particles/Emitter.h"

UActorIOSystem::UActorIOSystem()
{
    ActionExecContext = FActionExecutionContext();
}

TArray<FActorIOEvent> UActorIOSystem::GetEventsForObject(AActor* InObject)
{
    TArray<FActorIOEvent> OutEvents = TArray<FActorIOEvent>();
    if (IsValid(InObject))
    {
        if (InObject->Implements<UActorIOInterface>())
        {
            IActorIOInterface::Execute_RegisterIOEvents(InObject, OutEvents);
        }

        GetNativeEventsForObject(InObject, OutEvents);
    }
    
    return OutEvents;
}

TArray<FActorIOFunction> UActorIOSystem::GetFunctionsForObject(AActor* InObject)
{
    TArray<FActorIOFunction> OutFunctions = TArray<FActorIOFunction>();
    if (IsValid(InObject))
    {
        if (InObject->Implements<UActorIOInterface>())
        {
            IActorIOInterface::Execute_RegisterIOFunctions(InObject, OutFunctions);
        }

        GetNativeFunctionsForObject(InObject, OutFunctions);
    }

    return OutFunctions;
}

TArray<TWeakObjectPtr<UActorIOAction>> UActorIOSystem::GetInputActionsForObject(const AActor* InObject)
{
    TArray<TWeakObjectPtr<UActorIOAction>> OutActions = TArray<TWeakObjectPtr<UActorIOAction>>();
    if (IsValid(InObject))
    {
        for (TObjectIterator<UActorIOAction> ActionItr; ActionItr; ++ActionItr)
        {
            UActorIOAction* Action = *ActionItr;
            if (IsValid(Action) && Action->TargetActor == InObject)
            {
                OutActions.Add(Action);
            }
        }
    }

    return OutActions;
}

int32 UActorIOSystem::GetNumInputActionsForObject(const AActor* InObject)
{
    TArray<TWeakObjectPtr<UActorIOAction>> InputActions = GetInputActionsForObject(InObject);
    return InputActions.Num();
}

void UActorIOSystem::RegisterIOEvent(UObject* WorldContextObject, TArray<FActorIOEvent>& RegisterTo, FName EventId, FName EventDispatcherName, const FText& DisplayName, const FText& TooltipText)
{
    RegisterTo.Add(FActorIOEvent()
        .SetId(EventId)
        .SetDisplayName(DisplayName)
        .SetTooltipText(TooltipText)
        .SetBlueprintDelegate(WorldContextObject, EventDispatcherName));
}

void UActorIOSystem::RegisterIOFunction(UObject* WorldContextObject, TArray<FActorIOFunction>& RegisterTo, FName FunctionId, FString FunctionToExec, const FText& DisplayName, const FText& TooltipText)
{
    RegisterTo.Add(FActorIOFunction()
        .SetId(FunctionId)
        .SetDisplayName(DisplayName)
        .SetTooltipText(TooltipText)
        .SetFunction(FunctionToExec));
}

void UActorIOSystem::GetNativeEventsForObject(AActor* InObject, TArray<FActorIOEvent>& RegisteredEvents)
{
    if (InObject->IsA<ATriggerBase>())
    {
        RegisteredEvents.Add(FActorIOEvent()
            .SetId(TEXT("AActor::OnActorBeginOverlap"))
            .SetDisplayName(FText::FromString(TEXT("OnActorBeginOverlap")))
            .SetTooltipText(FText::FromString(TEXT("Event when something overlaps with the actor.")))
            .SetSparseDelegate(InObject, TEXT("OnActorBeginOverlap")));

        RegisteredEvents.Add(FActorIOEvent()
            .SetId(TEXT("AActor::OnActorEndOverlap"))
            .SetDisplayName(FText::FromString(TEXT("OnActorEndOverlap")))
            .SetTooltipText(FText::FromString(TEXT("Event when something no longer overlaps with the actor.")))
            .SetSparseDelegate(InObject, TEXT("OnActorEndOverlap")));
    }
}

void UActorIOSystem::GetNativeFunctionsForObject(AActor* InObject, TArray<FActorIOFunction>& RegisteredFunctions)
{
    if (InObject->IsA<AEmitter>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AEmitter::Activate"))
            .SetDisplayName(FText::FromString(TEXT("Activate")))
            .SetTooltipText(FText::FromString(TEXT("Activate the particle system.")))
            .SetFunction(TEXT("Activate")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AEmitter::Deactivate"))
            .SetDisplayName(FText::FromString(TEXT("Deactivate")))
            .SetTooltipText(FText::FromString(TEXT("Deactivate the particle system.")))
            .SetFunction(TEXT("Deactivate")));
    }

    //RegisteredFunctions.Add(FActorIOFunction()
    //    .SetId(TEXT("AActor::SetActorHiddenInGame"))
    //    .SetDisplayName(FText::FromString(TEXT("SetActorHiddenInGame")))
    //    .SetTooltipText(FText::FromString(TEXT("Changes actor hidden state.")))
    //    .SetFunction(TEXT("SetActorHiddenInGame")));

    RegisteredFunctions.Add(FActorIOFunction()
        .SetId(TEXT("AActor::Destroy"))
        .SetDisplayName(FText::FromString(TEXT("Destroy")))
        .SetTooltipText(FText::FromString(TEXT("Destroy the actor.")))
        .SetFunction(TEXT("K2_DestroyActor")));
}
