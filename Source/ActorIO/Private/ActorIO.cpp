// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIO.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOInterface.h"
#include "ActorIOSubsystemBase.h"
#include "GameFramework/Actor.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(LogActorIO);

//==================================
//~ Begin Console Variables
//==================================

TAutoConsoleVariable<bool> CVarDebugIOActions(
    TEXT("ActorIO.DebugActions"), true,
    TEXT("<bool> Enable I/O action execution messages."), ECVF_Default);

TAutoConsoleVariable<bool> CVarWarnAboutIOActionInvalidTarget(
    TEXT("ActorIO.WarnAboutInvalidTarget"), true,
    TEXT("<bool> Warn about missing or invalid target actor when executing I/O action."), ECVF_Default);

TAutoConsoleVariable<bool> CVarLogIOActionNamedArgs(
    TEXT("ActorIO.LogNamedArgs"), false,
    TEXT("<bool> Log named arguments to console when executing I/O action."), ECVF_Default);

TAutoConsoleVariable<bool> CVarLogIOActionFinalCommand(
    TEXT("ActorIO.LogFinalCommand"), false,
    TEXT("<bool> Log the final command sent to the target actor after executing I/O action."), ECVF_Default);

//==================================
//~ Begin FActionExecutionContext
//==================================

FActionExecutionContext& FActionExecutionContext::Get(UObject* WorldContextObject)
{
    UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(WorldContextObject);
    return IOSubsystem->GetExecutionContext();
}

void FActionExecutionContext::EnterContext(UActorIOAction* InAction, void* InScriptParams)
{
    check(!HasContext());
    ActionPtr = InAction;
    ScriptParams = InScriptParams;
    NamedArguments.Reset();
    bAborted = false;
    bProcessResult = false;
}

void FActionExecutionContext::ExitContext()
{
    check(HasContext());
    ActionPtr = nullptr;
    ScriptParams = nullptr;
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

void FActionExecutionContext::AbortAction()
{
    if (HasContext())
    {
        bAborted = true;
    }
}

//==================================
//~ Begin IActorIO
//==================================

FActorIOEventList IActorIO::GetEventsForObject(AActor* InObject)
{
    // Build list of registered I/O events for the object.
    // Always re-constructing the list because storing it would not be good perf/memory tradeoff for most cases.
    // In the future we might want to add a "cached" mode for frequently spawned actors.

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

        UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(InObject);
        if (IOSubsystem)
        {
            IOSubsystem->RegisterNativeEventsForObject(InObject, OutEvents);
            IOSubsystem->K2_RegisterNativeEventsForObject(InObject, OutEvents);
        }
    }

    return OutEvents;
}

FActorIOFunctionList IActorIO::GetFunctionsForObject(AActor* InObject)
{
    // Build list of registered I/O functions for the object.
    // Always re-constructing the list because storing it would not be good perf/memory tradeoff for most cases.
    // In the future we might want to add a "cached" mode for frequently spawned actors.

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

        UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(InObject);
        if (IOSubsystem)
        {
            IOSubsystem->RegisterNativeFunctionsForObject(InObject, OutFunctions);
            IOSubsystem->K2_RegisterNativeFunctionsForObject(InObject, OutFunctions);
        }
    }

    return OutFunctions;
}

const TArray<TWeakObjectPtr<UActorIOAction>> IActorIO::GetInputActionsForObject(AActor* InObject)
{
    // Internally there is no such thing as an input action.
    // Just actions pointing to actors.
    // Essentially this just gets all actions that point to the given actor.

    TArray<TWeakObjectPtr<UActorIOAction>> OutActions = TArray<TWeakObjectPtr<UActorIOAction>>();
    if (IsValid(InObject))
    {
        for (TObjectIterator<UActorIOAction> ActionItr; ActionItr; ++ActionItr)
        {
            UActorIOAction* Action = *ActionItr;
            if (IsValid(Action) && IsValid(Action->GetOwnerActor()))
            {
                // According to TObjectIterator description, we need to make sure that we
                // don't include objects from different worlds (e.g. PIE sessions).
                if (Action->GetWorld() == InObject->GetWorld())
                {
                    if (Action->TargetActor.Get() == InObject)
                    {
                        // Convert to weak ptr so that we cannot modify action's lifetime in editor.
                        OutActions.Emplace(Action);
                    }
                }
            }
        }
    }

    return OutActions;
}

int32 IActorIO::GetNumInputActionsForObject(AActor* InObject)
{
    const TArray<TWeakObjectPtr<UActorIOAction>> InputActions = GetInputActionsForObject(InObject);
    return InputActions.Num();
}

const TArray<TWeakObjectPtr<UActorIOAction>> IActorIO::GetOutputActionsForObject(AActor* InObject)
{
    // Internally there is no such thing as an output action.
    // All actions are "outputs" as they always make things happen to other actors.

    TArray<TWeakObjectPtr<UActorIOAction>> OutActions = TArray<TWeakObjectPtr<UActorIOAction>>();
    if (IsValid(InObject))
    {
        UActorIOComponent* IOComponent = InObject->GetComponentByClass<UActorIOComponent>();
        if (IOComponent)
        {
            for (const TObjectPtr<UActorIOAction>& Action : IOComponent->GetActions())
            {
                // Convert to weak ptr so that we cannot modify action's lifetime in editor.
                OutActions.Emplace(Action);
            }
        }
    }

    return OutActions;
}

int32 IActorIO::GetNumOutputActionsForObject(AActor* InObject)
{
    if (IsValid(InObject))
    {
        UActorIOComponent* IOComponent = InObject->GetComponentByClass<UActorIOComponent>();
        if (IOComponent)
        {
            return IOComponent->GetNumActions();
        }
    }

    return 0;
}

bool IActorIO::ConfirmObjectIsAlive(UObject* InObject, FString& OutError)
{
    if (!IsValid(InObject))
    {
        OutError = TEXT("Object is null, or pending kill (unloaded or destroyed?).");
        return false;
    }

    // Due to level streaming, we also need to make sure that the actor has a valid world.
    // Objects that are being streamed out still appear to be valid, even though they are not properly part of a level/world anymore (until GC'ed).
    // I suspect this happens because of LevelStreaming.ShouldReuseUnloadedButStillAroundLevels.
    // @see https://forums.unrealengine.com/t/unloaded-actor-not-being-destroyed/2536205
    UWorld* World = InObject->GetWorld();
    if (!World)
    {
        OutError = TEXT("Object does not have a valid world (streaming in progress?).");
        return false;
    }

    // In case we still encounter crashes or errors then we should probably check for level activation as well.
    // However, if we do use this then we'll need to make sure that actions are not executing too early/late.
    // For example, during 'BeginPlay' of streamed actors the level is not active.
    // Same goes for 'EndPlay'. At that the time the level is no longer active.
    // Also, we cannot simply check 'Level->IsPersistentLevel' because with World Partition all actors are on dynamic streaming levels.
    // Let's try to avoid this requirement.
#if 0
    ULevel* Level = InObject->GetTypedOuter<ULevel>(); // same as AActor::GetLevel
    if (!Level || !Level->bIsVisible)
    {
        OutError = TEXT("Object is not part of an active level (streaming in progress?).");
        return false;
    }
#endif

    return true;
}

bool IActorIO::ValidateFunctionArguments(UFunction* FunctionPtr, const FString& InArguments, FText& OutError)
{
    if (!ensure(FunctionPtr))
    {
        // Do nothing if function ptr is null.
        return false;
    }

    TArray<FString> Arguments;
    InArguments.ParseIntoArray(Arguments, ARGUMENT_SEPARATOR, true);

    int32 NumParamsExpected = 0;
    for (TFieldIterator<FProperty> It(FunctionPtr); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        FProperty* FunctionProp = *It;
        checkSlow(FunctionProp);

        // Do not count return property.
        if (FunctionProp->HasAnyPropertyFlags(CPF_ReturnParm))
        {
            continue;
        }

        // Do not count output params, but only if they are not passed by ref
        // since in that case the value is also an input param.
        if (FunctionProp->HasAnyPropertyFlags(CPF_OutParm) && !FunctionProp->HasAnyPropertyFlags(CPF_ReferenceParm))
        {
            continue;
        }

        // #TODO: Is it possible to actually try and import the value into the FProperty and catch errors?

        NumParamsExpected++;
    }

    if (Arguments.Num() > NumParamsExpected)
    {
        OutError = NSLOCTEXT("ActorIO", "ArgsValidation_TooManyParams", "Too many parameters");
        return false;
    }
    else if (Arguments.Num() < NumParamsExpected)
    {
        OutError = NSLOCTEXT("ActorIO", "ArgsValidation_NotEnoughParams", "Not enough parameters");
        return false;
    }

    return true;
}

void IActorIO::ExecutionError(bool bCondition, ELogVerbosity::Type InVerbosity, const FString& InMessage)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) || USE_LOGGING_IN_SHIPPING // Do not Print in Shipping or Test unless explicitly enabled.
    check(InVerbosity == ELogVerbosity::Warning || InVerbosity == ELogVerbosity::Error); // Only warnings and errors.
    if (bCondition)
    {
        if (InVerbosity == ELogVerbosity::Warning)
        {
            UE_LOG(LogActorIO, Warning, TEXT("%s"), *InMessage);
        }
        else
        {
            UE_LOG(LogActorIO, Error, TEXT("%s"), *InMessage);
        }

        if (GEngine && GAreScreenMessagesEnabled)
        {
            const float DisplayTime = 3.0f;
            const FColor DisplayColor = InVerbosity == ELogVerbosity::Warning ? FColor::Yellow : FColor::Red;
            GEngine->AddOnScreenDebugMessage(INDEX_NONE, DisplayTime, DisplayColor, InMessage);
        }
    }
#endif
}
