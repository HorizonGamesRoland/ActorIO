// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIO.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOInterface.h"
#include "ActorIOSubsystemBase.h"
#include "GameFramework/Actor.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(LogActorIO);

//==================================
// Console Variables
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
// FActionExecutionContext
//==================================

FActionExecutionContext& FActionExecutionContext::Get(UObject* WorldContextObject)
{
    UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(WorldContextObject);
    return IOSubsystem->ActionExecContext;
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

//==================================
// IActorIO
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
        IOSubsystem->RegisterNativeEventsForObject(InObject, OutEvents);
        IOSubsystem->K2_RegisterNativeEventsForObject(InObject, OutEvents);
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
        IOSubsystem->RegisterNativeFunctionsForObject(InObject, OutFunctions);
        IOSubsystem->K2_RegisterNativeFunctionsForObject(InObject, OutFunctions);
    }

    return OutFunctions;
}

const TArray<UActorIOAction*> IActorIO::GetInputActionsForObject(AActor* InObject)
{
    // Internally there is no such thing as an input action.
    // Just actions pointing to actors.
    // Essentially this just gets all actions that point to the given actor.

    TArray<UActorIOAction*> OutActions = TArray<UActorIOAction*>();
    if (IsValid(InObject))
    {
        for (TObjectIterator<UActorIOAction> ActionItr; ActionItr; ++ActionItr)
        {
            UActorIOAction* Action = *ActionItr;
            if (IsValid(Action) && IsValid(Action->GetOwnerActor()) && Action->TargetActor.Get() == InObject)
            {
                OutActions.Add(Action);
            }
        }
    }

    return OutActions;
}

int32 IActorIO::GetNumInputActionsForObject(AActor* InObject)
{
    const TArray<UActorIOAction*> InputActions = GetInputActionsForObject(InObject);
    return InputActions.Num();
}

const TArray<UActorIOAction*> IActorIO::GetOutputActionsForObject(AActor* InObject)
{
    // Internally there is no such thing as an output action.
    // All actions are "outputs" as they always make things happen to other actors.

    TArray<UActorIOAction*> OutActions = TArray<UActorIOAction*>();
    if (IsValid(InObject))
    {
        UActorIOComponent* IOComponent = InObject->GetComponentByClass<UActorIOComponent>();
        if (IOComponent)
        {
            return IOComponent->GetActions();
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

bool IActorIO::ConfirmObjectIsAlive(UObject* InObject)
{
    if (!IsValid(InObject))
    {
        return false;
    }

    UWorld* World = InObject->GetWorld();
    if (!World || World->bIsTearingDown)
    {
        return false;
    }

    // Due to level streaming, we also need to make sure that the owning level is "visible" (active).
    // Objects that are being streamed out still appear to be valid, even though they are not properly part of a level/world anymore (until GC'ed).
    // I suspect this happens because of LevelStreaming.ShouldReuseUnloadedButStillAroundLevels.
    // @see https://forums.unrealengine.com/t/unloaded-actor-not-being-destroyed/2536205
    ULevel* Level = InObject->GetTypedOuter<ULevel>(); // same as AActor::GetLevel
    if (!Level || !Level->bIsVisible)
    {
        return false;
    }

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
