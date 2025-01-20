// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/ScriptDelegateFwd.h"
#include "UObject/SparseDelegate.h"
#include "ActorIO.generated.h"

class UActorIOAction;
class UObject;

/** Custom log category of the Actor IO plugin. */
ACTORIO_API DECLARE_LOG_CATEGORY_EXTERN(LogActorIO, Log, All);

/** Character to use as a separator for I/O action arguments. */
#define ARGUMENT_SEPARATOR TEXT(" ")

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

	/** Unique id of the event on a per class basis. */
	FName EventId;

	/** Display name to use in the editor. If empty, event id will be used. */
	FText DisplayName;

	/** Tooltip text to display in the editor. */
	FText TooltipText;

	/** The owner of the assigned delegate. */
	UPROPERTY()
	TObjectPtr<UObject> DelegateOwner;

	/** Reference to a multicast script delegate. */
	FMulticastScriptDelegate* MulticastDelegatePtr;

	/** Reference to a sparse delegate. */
	FName SparseDelegateName;

	/** Reference to an event dispatcher created in blueprints. */
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
		return *this;
	}

	/** Set the assigned delegate to a sparse delegate. */
	FActorIOEvent& SetSparseDelegate(UObject* InDelegateOwner, FName InSparseDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		SparseDelegateName = InSparseDelegateName;
		return *this;
	}

	/** Set the assigned delegate to an event dispatcher that was created in blueprints. */
	FActorIOEvent& SetBlueprintDelegate(UObject* InDelegateOwner, FName InBlueprintDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		BlueprintDelegateName = InBlueprintDelegateName;
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

	bool operator==(const FName InEventName) const
	{
		return EventId == InEventName;
	}

	bool operator!=(const FName InEventName) const
	{
		return EventId != InEventName;
	}
};

/**
 * List of I/O events registered with an object.
 * Use the register function to add elements to this.
 */
USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOEventList
{
	GENERATED_BODY()

	/**
	 * The internal list of I/O events.
	 * Do not modify directly, use the register function instead.
	 */
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
		// #TODO: Raise error in editor if event already contained?
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

	bool operator==(const FName InFunctionId) const
	{
		return FunctionId == InFunctionId;
	}

	bool operator!=(const FName InFunctionId) const
	{
		return FunctionId != InFunctionId;
	}
};

/**
 * List of I/O functions registered with an object.
 * Use the register function to add elements to this.
 */
USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOFunctionList
{
	GENERATED_BODY()

	/**
	 * The internal list of I/O functions.
	 * Do not modify directly, use the register function instead.
	 */
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
		// #TODO: Raise error in editor if function already contained?
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
