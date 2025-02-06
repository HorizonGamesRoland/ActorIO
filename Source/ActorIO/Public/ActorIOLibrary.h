// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#pragma once

#include "ActorIO.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActorIOLibrary.generated.h"

/**
 * Blueprint functions for interacting with the Actor I/O system.
 */
UCLASS(DisplayName = "Actor I/O Library")
class ACTORIO_API UActorIOLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Add a new I/O event to the actor's event list.
	 * Use this to expose a blueprint event dispatcher to the I/O system.
	 * Should only be called when the I/O interface is registering events to an actor.
	 *
	 * @param WorldContextObject Reference to the object where this function is being called.
	 * @param Registry The list of I/O events we are adding to.
	 * @param EventId Unique id of the event. Recommended format is ClassName::EventName.
	 * @param DisplayNameText Display name to use in the editor for this event.
	 * @param TooltipText Tooltip to use in the editor for this event.
	 * @param EventDispatcherName Name of the event dispatcher that should be exposed.
	 * @param EventProcessorName Name of a function that should be called when firing this event. Use this to handle named arguments (params) for this event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor IO", DisplayName = "Register I/O Event", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayNameText,TooltipText", AdvancedDisplay = "EventProcessorName", Keywords = "IO,Add"))
	static void K2_RegisterIOEvent(UObject* WorldContextObject, UPARAM(Ref) FActorIOEventList& Registry, FName EventId, const FText& DisplayNameText, const FText& TooltipText, FName EventDispatcherName, FName EventProcessorName);

	/**
	 * Add a new I/O function to the actor's function list.
	 * Use this to expose a blueprint function to the I/O system.
	 * Should only be called when the I/O interface is registering functions to an actor.
	 *
	 * @param WorldContextObject Reference to the object where this function is being called.
	 * @param Registry The list of I/O functions we are adding to.
	 * @param FunctionId Unique id of the function. Recommended format is ClassName::FunctionName.
	 * @param DisplayNameText Display name to use in the editor for this function.
	 * @param TooltipText Tooltip to use in the editor for this function.
	 * @param FunctionToExec Name of the blueprint function that should be exposed.
	 * @param SubobjectName Specific subobject to call the function on instead of the actor itself.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor IO", DisplayName = "Register I/O Function", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayNameText,TooltipText", AdvancedDisplay = "SubobjectName", Keywords = "IO,Add"))
	static void K2_RegisterIOFunction(UObject* WorldContextObject, UPARAM(Ref) FActorIOFunctionList& Registry, FName FunctionId, const FText& DisplayNameText, const FText& TooltipText, FString FunctionToExec, FName SubobjectName);

	/**
	 * Add a named argument (parameter) to the current execution context.
	 * If it already exists then the value is simply updated.
	 * Should only be called from within an I/O event processor!
	 * 
	 * @param WorldContextObject Reference to the object where this function is being called.
	 * @param ArgumentName Text to swap into the argument value. Text must start with '$'.
	 * @param ArgumentValue Value of the named argument.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO", DisplayName = "Set Named Argument", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ArgumentName,ArgumentValue", Keywords = "Param"))
	static void K2_SetNamedArgument(UObject* WorldContextObject, const FString& ArgumentName, const FString& ArgumentValue);
};
