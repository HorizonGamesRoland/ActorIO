// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/ScriptDelegateFwd.h"
#include "UObject/SparseDelegate.h"
#include "HAL/IConsoleManager.h"
#include "ActorIO.generated.h"

class UActorIOAction;
class UObject;

/** Actor I/O log category. */
ACTORIO_API DECLARE_LOG_CATEGORY_EXTERN(LogActorIO, Log, All);

/** Character to use as a separator for I/O action arguments. */
#define ARGUMENT_SEPARATOR TEXT(";")

/** Prefix to identify named arguments with. */
#define NAMEDARGUMENT_PREFIX TEXT("$")

/**
 * Exposes a dynamic delegate to the I/O system (e.g. OnTriggerEnter, OnValueChanged).
 * Actions bound to this event will be executed when the assigned delegate is triggered.
 * In classic terms this is the "output" part of the Input/Output system.
 */
USTRUCT()
struct ACTORIO_API FActorIOEvent
{
	GENERATED_BODY()

	/** Supported delegate types that can be assigned. */
	enum Type
	{
		Null,
		MulticastDelegate,
		SparseDelegate,
		BlueprintDelegate
	};

	/** Unique id of the event on a per class basis. */
	FName EventId;

	/** Display name to use in the editor. If empty, event id will be used. */
	FText DisplayName;

	/** Tooltip text to display in the editor. */
	FText TooltipText;

	/** The owner of the assigned delegate. */
	TWeakObjectPtr<UObject> DelegateOwner;

	/** The type of the assigned delegate. */
	Type DelegateType;

	/** Reference to a multicast script delegate. */
	FMulticastScriptDelegate* MulticastDelegatePtr;

	/** Reference to a sparse delegate. */
	FName SparseDelegateName;

	/** Reference to a blueprint exposed dynamic delegate. */
	FName BlueprintDelegateName;

	/**
	 * A function that is triggered before executing an action that's bound to this I/O event.
	 * Used to initialized values for named arguments.
	 * The signature of the function should be the same as the delegate that's assigned to the event.
	 */
	FScriptDelegate EventProcessor;

	/** Default constructor. */
	FActorIOEvent() :
		EventId(NAME_None),
		DisplayName(FText::GetEmpty()),
		TooltipText(FText::GetEmpty()),
		DelegateOwner(nullptr),
		DelegateType(FActorIOEvent::Type::Null),
		MulticastDelegatePtr(nullptr),
		SparseDelegateName(NAME_None),
		BlueprintDelegateName(NAME_None),
		EventProcessor(FScriptDelegate())
	{}

	/**
	 * Set the id of the event. Id must be unique on a per class basis.
	 * Recommended form is ClassName::EventName (e.g. AActor::OnDestroy).
	 */
	FActorIOEvent& SetId(FName InEventId)
	{
		EventId = InEventId;
		return *this;
	}

	/** Set the display name to use in the editor. If empty, event id will be used. */
	FActorIOEvent& SetDisplayName(const FText& InDisplayName)
	{
		DisplayName = InDisplayName;
		return *this;
	}

	/** Set the tooltip text to use in the editor. */
	FActorIOEvent& SetTooltipText(const FText& InTooltipText)
	{
		TooltipText = InTooltipText;
		return *this;
	}

	/** Set the assigned delegate to a multicast script delegate. */
	FActorIOEvent& SetMulticastDelegate(UObject* InDelegateOwner, FMulticastScriptDelegate* InMulticastDelegate)
	{
		DelegateOwner = InDelegateOwner;
		MulticastDelegatePtr = InMulticastDelegate;
		DelegateType = FActorIOEvent::Type::MulticastDelegate;
		return *this;
	}

	/** Set the assigned delegate to a sparse delegate. */
	FActorIOEvent& SetSparseDelegate(UObject* InDelegateOwner, FName InSparseDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		SparseDelegateName = InSparseDelegateName;
		DelegateType = FActorIOEvent::Type::SparseDelegate;
		return *this;
	}

	/** Set the assigned delegate to an event dispatcher that was created in blueprints. */
	FActorIOEvent& SetBlueprintDelegate(UObject* InDelegateOwner, FName InBlueprintDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		BlueprintDelegateName = InBlueprintDelegateName;
		DelegateType = FActorIOEvent::Type::BlueprintDelegate;
		return *this;
	}

	/**
	 * Assign an event processor to this I/O event that will be called before a bound action is executed.
	 * This can be used to initialized values for named arguments.
	 * The assigned function must be a UFUNCTION and its signature (param list) should be the same as the event's delegate!
	 */
	FActorIOEvent& SetEventProcessor(UObject* InFunctionOwner, FName InFunctionName)
	{
		EventProcessor.BindUFunction(InFunctionOwner, InFunctionName);
		return *this;
	}

	/** Equals operator. Required to support TArray.FindByKey(). */
	FORCEINLINE bool operator==(const FName InEventName) const
	{
		return EventId == InEventName;
	}

	/** Not equals operator. */
	FORCEINLINE bool operator!=(const FName InEventName) const
	{
		return EventId != InEventName;
	}
};

/**
 * List of I/O events registered with an object.
 * Use the register function to add elements to the list.
 */
USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOEventList
{
	GENERATED_BODY()

	/**
	 * The internal list of I/O events.
	 * Do not modify directly, use RegisterEvent() instead.
	 */
	UPROPERTY()
	TArray<FActorIOEvent> EventRegistry;

	/** Default constructor. */
	FActorIOEventList() :
		EventRegistry(TArray<FActorIOEvent>())
	{}

	/** Constructor starting with a list of I/O events. */
	FActorIOEventList(const TArray<FActorIOEvent>& InEvents) :
		EventRegistry(InEvents)
	{}

	/** Add a new I/O event to the list. */
	void RegisterEvent(const FActorIOEvent& InEvent)
	{
		const FActorIOEvent* ExistingEvent = EventRegistry.FindByKey(InEvent.EventId);
		if (ExistingEvent)
		{
			UE_LOG(LogActorIO, Error, TEXT("Could not register I/O event '%s' - An event with the same id already exists!"), *InEvent.EventId.ToString());
			return;
		}

		EventRegistry.Add(InEvent);
	}

	/** Attempt to get a registered I/O event by id. */
	FActorIOEvent* GetEvent(FName InEventId)
	{
		return EventRegistry.FindByKey(InEventId);
	}

	/** Attempt to get a registered I/O event by id (const version). */
	const FActorIOEvent* GetEvent(FName InEventId) const
	{
		return EventRegistry.FindByKey(InEventId);
	}
};

/**
 * Exposes a callable function to the I/O system (e.g. SetValue, PlayEffect, DestroyActor).
 * These functions will be called by actions when their event is triggered.
 * In classic terms this is the "input" part of the Input/Output system.
 */
USTRUCT()
struct ACTORIO_API FActorIOFunction
{
	GENERATED_BODY()

	/** Unique id of the function on a per class basis. */
	FName FunctionId;

	/** Display name to use in the editor. If empty, function id will be used. */
	FText DisplayName;

	/** Tooltip text to display in the editor. */
	FText TooltipText;

	/**
	 * The name of the function to execute.
	 * The function must be marked as UFUNCTION in C++.
	 */
	FString FunctionToExec;

	/**
	 * Specific subobject to call the function on instead of the actor itself.
	 * Can be used to avoid duplicating functions from components since the I/O system only communicates between actors.
	 */
	FName TargetSubobject;

	/** Default constructor. */
	FActorIOFunction() :
		FunctionId(NAME_None),
		DisplayName(FText::GetEmpty()),
		TooltipText(FText::GetEmpty()),
		FunctionToExec(FString()),
		TargetSubobject(NAME_None)
	{}

	/**
	 * Set the id of the function. Id must be unique on a per class basis.
	 * Recommended form is ClassName::FunctionName (e.g. AActor::DestroyActor).
	 */
	FActorIOFunction& SetId(FName InFunctionId)
	{
		FunctionId = InFunctionId;
		return *this;
	}

	/** Set the display name to use in the editor. If empty, function id will be used. */
	FActorIOFunction& SetDisplayName(const FText& InDisplayName)
	{
		DisplayName = InDisplayName;
		return *this;
	}

	/** Set the tooltip text to use in the editor. */
	FActorIOFunction& SetTooltipText(const FText& InTooltipText)
	{
		TooltipText = InTooltipText;
		return *this;
	}

	/**
	 * Set the name of the function to execute.
	 * The function must be marked as UFUNCTION in C++.
	 */
	FActorIOFunction& SetFunction(const FString& InFunctionName)
	{
		FunctionToExec = InFunctionName;
		return *this;
	}

	/**
	 * Set the specific subobject to call the function on instead of the actor itself.
	 * Can be used to avoid duplicating functions from components since the I/O system only communicates between actors.
	 * Only works with default subobjects.
	 */
	FActorIOFunction& SetSubobject(const FName& InSubobjectName)
	{
		TargetSubobject = InSubobjectName;
		return *this;
	}

	/** Equals operator. Required to support TArray.FindByKey(). */
	FORCEINLINE bool operator==(const FName InFunctionId) const
	{
		return FunctionId == InFunctionId;
	}

	/** Not equals operator. */
	FORCEINLINE bool operator!=(const FName InFunctionId) const
	{
		return FunctionId != InFunctionId;
	}
};

/**
 * List of I/O functions registered with an object.
 * Use the register function to add elements to the list.
 */
USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOFunctionList
{
	GENERATED_BODY()

	/**
	 * The internal list of I/O functions.
	 * Do not modify directly, use RegisterFunction() instead.
	 */
	UPROPERTY()
	TArray<FActorIOFunction> FunctionRegistry;

	/** Default constructor. */
	FActorIOFunctionList() :
		FunctionRegistry(TArray<FActorIOFunction>())
	{}

	/** Constructor starting with a list of I/O functions. */
	FActorIOFunctionList(const TArray<FActorIOFunction>& InFunctions) :
		FunctionRegistry(InFunctions)
	{}

	/** Add a new I/O function to the list. */
	void RegisterFunction(const FActorIOFunction& InFunction)
	{
		const FActorIOFunction* ExistingFunction = FunctionRegistry.FindByKey(InFunction.FunctionId);
		if (ExistingFunction)
		{
			UE_LOG(LogActorIO, Error, TEXT("Could not register I/O function '%s' - A function with the same id already exists!"), *InFunction.FunctionId.ToString());
			return;
		}

		FunctionRegistry.Add(InFunction);
	}

	/** Attempt to get a registered I/O function by id. */
	FActorIOFunction* GetFunction(FName InFunctionId)
	{
		return FunctionRegistry.FindByKey(InFunctionId);
	}

	/** Attempt to get a registered I/O function by id (const version). */
	const FActorIOFunction* GetFunction(FName InFunctionId) const
	{
		return FunctionRegistry.FindByKey(InFunctionId);
	}
};

/**
 * Context of an I/O action that the reflection system is about to execute.
 * Stores the original memory of the execute action call in case it is needed for named arguments.
 * Only valid between the action receiving the ProcessEvent call and sending the command to the target actor.
 * Use FActionExecutionContext::Get() to get the current context.
 */
USTRUCT()
struct ACTORIO_API FActionExecutionContext
{
	GENERATED_BODY()

	/** Reference to the action that is about to be executed. */
	UPROPERTY()
	TObjectPtr<UActorIOAction> ActionPtr;

	/**
	 * The original memory of the execute action call.
	 * Essentially these are the parameters that the delegate broadcasted to the action.
	 * Since actions are bound to delegates in a generic way, these parameters cannot be interpreted by the action.
	 * Instead, this memory is sent to the Event Processor of the I/O event that wraps the delegate.
	 * If the signature (param list) of the event processor matches the delegate, then we can import the values.
	 */
	void* ScriptParams;

	/**
	 * List of named arguments and their corresponding values.
	 * Elements should only be assigned from within an I/O event processor.
	 * Do not modify directly. Use SetNamedArgument() instead.
	 */
	TMap<FString, FString> NamedArguments;

	/** Default constructor. */
	FActionExecutionContext() :
		ActionPtr(nullptr),
		ScriptParams(nullptr),
		NamedArguments(TMap<FString, FString>())
	{}

	/** Get the current global execution context. */
	static FActionExecutionContext& Get(UObject* WorldContextObject);

	/** Enter a new execution context. */
	void EnterContext(UActorIOAction* InAction, void* InScriptParams);

	/** Leave the current execution context. */
	void ExitContext();

	/** @return Whether we are currently have an execution context. */
	bool HasContext() const;

	/**
	 * Add a named argument (parameter) to the current execution context.
	 * If it already exists then the value is simply updated.
	 * Should only be called from within an I/O event processor!
	 */
	void SetNamedArgument(const FString& InName, const FString& InValue);
};

/**
 * Interface to interact with the Actor I/O system.
 */
class ACTORIO_API IActorIO
{
public:

	/** @return List of registered I/O events of the given actor. */
	static FActorIOEventList GetEventsForObject(AActor* InObject);

	/** @return List of registered I/O functions of the given actor. */
	static FActorIOFunctionList GetFunctionsForObject(AActor* InObject);

	/** @return List of I/O actions currently loaded in the world that are targeting the given actor. */
	static const TArray<UActorIOAction*> GetInputActionsForObject(AActor* InObject);

	/** @return Number of I/O actions currently loaded in the world that are targeting the given actor. */
	static int32 GetNumInputActionsForObject(AActor* InObject);

	/** @return List of I/O actions owned by the given actor. */
	static const TArray<UActorIOAction*> GetOutputActionsForObject(AActor* InObject);

	/** @return Number of I/O actions owned by the given actor. */
	static int32 GetNumOutputActionsForObject(AActor* InObject);

	/** Perform basic checks to see if the given arguments can be imported into the function as parameters. */
	static bool ValidateFunctionArguments(UFunction* FunctionPtr, const FString& InArguments, FText& OutError);
};

/** [Console Variable] Whether to log I/O action execution messages. */
extern ACTORIO_API TAutoConsoleVariable<bool> CVarDebugIOActions;
/** [Console Variable] Whether to warn about missing or invalid target actor when executing I/O action. */
extern ACTORIO_API TAutoConsoleVariable<bool> CVarWarnAboutIOActionInvalidTarget;
/** [Console Variable] Whether to log named arguments to console when executing I/O action. */
extern ACTORIO_API TAutoConsoleVariable<bool> CVarLogIOActionNamedArgs;
/** [Console Variable] Whether to log the final command sent to the target actor after executing I/O action. */
extern ACTORIO_API TAutoConsoleVariable<bool> CVarLogIOActionFinalCommand;

/** Macro to get CVarDebugIOActions value on game thread. */
#define DebugIOActions CVarDebugIOActions.GetValueOnGameThread()
/** Macro to get CVarWarnAboutIOActionInvalidTarget value on game thread. */
#define WarnIOInvalidTarget CVarWarnAboutIOActionInvalidTarget.GetValueOnGameThread()
/** Macro to get CVarLogIOActionNamedArgs value on game thread. */
#define LogIONamedArgs CVarLogIOActionNamedArgs.GetValueOnGameThread()
/** Macro to get CVarLogIOActionFinalCommand value on game thread. */
#define LogIOFinalCommand CVarLogIOActionFinalCommand.GetValueOnGameThread()
