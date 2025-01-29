// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIO.h"
#include "ActorIOAction.h"
#include "ActorIOInterface.h"
#include "ActorIOSubsystemBase.h"

DEFINE_LOG_CATEGORY(LogActorIO)

FActionExecutionContext& FActionExecutionContext::Get(UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        UActorIOSubsystemBase* IOSubsystem = World->GetSubsystem<UActorIOSubsystemBase>();
        return IOSubsystem->ActionExecContext;
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
            UE_LOG(LogActorIO, Error, TEXT("ActionExecutionContext: Attempted to set named argument without the required '%s' prefix. Name was: %s"), NAMEDARGUMENT_PREFIX, *InName);
            return;
        }

        int32 InvalidCharIndex = INDEX_NONE;
        if (InName.FindChar('"', InvalidCharIndex) || InName.FindChar(';', InvalidCharIndex))
        {
            UE_LOG(LogActorIO, Error, TEXT("ActionExecutionContext: Attempted to set named argument with illegal character. Name was: %s"), *InName);
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

FActorIOEventList IActorIO::GetEventsForObject(AActor* InObject)
{
    FActorIOEventList OutEvents = FActorIOEventList();
    if (IsValid(InObject))
    {
        if (InObject->Implements<UActorIOInterface>())
        {
            IActorIOInterface* IOInterface = Cast<IActorIOInterface>(InObject);
            if (IOInterface)
            {
                IOInterface->RegisterIOEvents(OutEvents);
            }

            IActorIOInterface::Execute_K2_RegisterIOEvents(InObject, OutEvents);
        }

        UActorIOSubsystemBase* IOSubsystem = InObject->GetWorld()->GetSubsystem<UActorIOSubsystemBase>();
        IOSubsystem->GetNativeEventsForObject(InObject, OutEvents);
    }

    return OutEvents;
}

FActorIOFunctionList IActorIO::GetFunctionsForObject(AActor* InObject)
{
    FActorIOFunctionList OutFunctions = FActorIOFunctionList();
    if (IsValid(InObject))
    {
        if (InObject->Implements<UActorIOInterface>())
        {
            IActorIOInterface* IOInterface = Cast<IActorIOInterface>(InObject);
            if (IOInterface)
            {
                IOInterface->RegisterIOFunctions(OutFunctions);
            }

            IActorIOInterface::Execute_K2_RegisterIOFunctions(InObject, OutFunctions);
        }

        UActorIOSubsystemBase* IOSubsystem = InObject->GetWorld()->GetSubsystem<UActorIOSubsystemBase>();
        IOSubsystem->GetNativeFunctionsForObject(InObject, OutFunctions);
    }

    return OutFunctions;
}

TArray<TWeakObjectPtr<UActorIOAction>> IActorIO::GetInputActionsForObject(AActor* InObject)
{
    TArray<TWeakObjectPtr<UActorIOAction>> OutActions = TArray<TWeakObjectPtr<UActorIOAction>>();
    if (IsValid(InObject))
    {
        for (TObjectIterator<UActorIOAction> ActionItr; ActionItr; ++ActionItr)
        {
            UActorIOAction* Action = *ActionItr;
            if (IsValid(Action) && IsValid(Action->GetOwnerActor()) && Action->TargetActor == InObject)
            {
                OutActions.Add(Action);
            }
        }
    }

    return OutActions;
}

int32 IActorIO::GetNumInputActionsForObject(AActor* InObject)
{
    TArray<TWeakObjectPtr<UActorIOAction>> InputActions = GetInputActionsForObject(InObject);
    return InputActions.Num();
}
