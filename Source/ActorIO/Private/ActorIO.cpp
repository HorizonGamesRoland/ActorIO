// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIO.h"
#include "ActorIOSystem.h"

DEFINE_LOG_CATEGORY(LogActorIO)

FActionExecutionContext& FActionExecutionContext::Get(UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        UActorIOSystem* IOSystem = World->GetSubsystem<UActorIOSystem>();
        return IOSystem->GetExecutionContext();
    }

    checkNoEntry();
    static FActionExecutionContext InvalidContext;
    return InvalidContext;
}

void FActionExecutionContext::EnterContext(UActorIOAction* InAction, void* InScriptParams)
{
    check(!HasContext());
    ActionPtr = InAction;
    ScriptParams = InScriptParams;
}

void FActionExecutionContext::ExitContext()
{
    check(HasContext());
    ActionPtr = nullptr;
    ScriptParams = nullptr;
    NamedArguments.Reset();
}

bool FActionExecutionContext::HasContext() const
{
    return ActionPtr.Get() != nullptr;
}

void FActionExecutionContext::SetNamedArgument(const FString& InName, const FString& InValue)
{
    if (HasContext() && !InName.IsEmpty())
    {
        if (!InName.StartsWith(NAMEDARGUMENT_PREFIX))
        {
            UE_LOG(LogActorIO, Error, TEXT("ActionExecutionContext: Attempted to set named argument without the necessary '%s' prefix. Name was: %s"), NAMEDARGUMENT_PREFIX, *InName);
            return;
        }

        if (!InValue.IsEmpty())
        {
            FString& Arg = NamedArguments.FindOrAdd(InName);
            Arg = InValue;
        }
        else
        {
            NamedArguments.Remove(InName);
        }
    }
}
