// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOLibrary.h"

void UActorIOLibrary::K2_RegisterIOEvent(UObject* WorldContextObject, FActorIOEventList& Registry, FName EventId, const FText& DisplayNameText, const FText& TooltipText, FName EventDispatcherName, FName EventProcessorName, FName SubobjectName)
{
    // Temporary solution to support registering an I/O event for a subobject of the actor in blueprints too.
    // From C++ you can already set the delegate owner directly.
    // This will be reworked once the new registration methods are introduced.
    UObject* DelegateOwner = WorldContextObject;
    if (IsValid(WorldContextObject) && !SubobjectName.IsNone())
    {
        DelegateOwner = WorldContextObject->GetDefaultSubobjectByName(SubobjectName);
    }

    Registry.RegisterEvent(FActorIOEvent()
        .SetId(EventId)
        .SetDisplayName(DisplayNameText)
        .SetTooltipText(TooltipText)
        .SetBlueprintDelegate(DelegateOwner, EventDispatcherName)
        .SetEventProcessor(DelegateOwner, EventProcessorName));
}

void UActorIOLibrary::K2_RegisterIOFunction(UObject* WorldContextObject, FActorIOFunctionList& Registry, FName FunctionId, const FText& DisplayNameText, const FText& TooltipText, FString FunctionToExec, FName SubobjectName)
{
    Registry.RegisterFunction(FActorIOFunction()
        .SetId(FunctionId)
        .SetDisplayName(DisplayNameText)
        .SetTooltipText(TooltipText)
        .SetFunction(FunctionToExec)
        .SetSubobject(SubobjectName));
}

void UActorIOLibrary::K2_SetNamedArgument(UObject* WorldContextObject, const FString& ArgumentName, const FString& ArgumentValue)
{
    FActionExecutionContext& ExecContext = FActionExecutionContext::Get(WorldContextObject);
    ExecContext.SetNamedArgument(ArgumentName, ArgumentValue);
}
