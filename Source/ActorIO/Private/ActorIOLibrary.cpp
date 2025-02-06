// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#include "ActorIOLibrary.h"

void UActorIOLibrary::K2_RegisterIOEvent(UObject* WorldContextObject, FActorIOEventList& Registry, FName EventId, const FText& DisplayNameText, const FText& TooltipText, FName EventDispatcherName, FName EventProcessorName)
{
    Registry.RegisterEvent(FActorIOEvent()
        .SetId(EventId)
        .SetDisplayName(DisplayNameText)
        .SetTooltipText(TooltipText)
        .SetBlueprintDelegate(WorldContextObject, EventDispatcherName)
        .SetEventProcessor(WorldContextObject, EventProcessorName));
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
