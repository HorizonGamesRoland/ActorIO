// Copyright Lim Young.

#pragma once

#include "CoreMinimal.h"
#include "ActorIO.h"
#include "UObject/Object.h"
#include "ActorIORegisterBase.generated.h"

/**
 * Base class for registering I/O events and functions for an actor.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class ACTORIO_API UActorIORegisterBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Called when building list of registered I/O events for the actor.
	 * This function is called every time the list of registered events is requested for the actor.
	 * Called in editor and at runtime!
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ActorIO|Register")
	void RegisterIOEvents(AActor* RegisteredActor, FActorIOEventList& EventRegistry);

	/**
	 * Called when building list of registered I/O functions for the actor.
	 * This function is called every time the list of registered functions is requested for the actor.
	 * Called in editor and at runtime!
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ActorIO|Register")
	void RegisterIOFunctions(AActor* RegisteredActor, FActorIOFunctionList& FunctionRegistry);

	/** Returns the class of actors that are required to register I/O events. */
	UFUNCTION(BlueprintNativeEvent, Category = "ActorIO|Register")
	TSubclassOf<AActor> GetActorsClassRequiredToRegister() const;

protected:
	/**
	 * Add a new I/O event to the actor's event list.
	 * Use this to expose a blueprint event dispatcher to the I/O system.
	 * Should only be called with UActorIORegisterBase derived classes.
	 *
	 * @param RegisteredActor The actor we are registering the event for.
	 * @param Registry The list of I/O events we are adding to.
	 * @param EventId Unique id of the event. Recommended format is ClassName::EventName.
	 * @param DisplayNameText Display name to use in the editor for this event.
	 * @param TooltipText Tooltip to use in the editor for this event.
	 * @param EventDispatcherName Name of the event dispatcher that should be exposed.
	 * @param EventProcessorName Name of a function that should be called when firing this event. Use this to handle named arguments (params) for this event.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO|Register",
		meta = (DisplayName = "Register I/O Event For Required Actor"))
	void BP_RegisterIOEventForRequiredActor(AActor* RegisteredActor, UPARAM(Ref)
	                                        FActorIOEventList& Registry, FName EventId, const FText& DisplayNameText,
	                                        const FText& TooltipText, FName EventDispatcherName,
	                                        FName EventProcessorName);

	/**
	 * Add a new I/O function to the actor's function list.
	 * Use this to expose a blueprint function to the I/O system.
	 * Should only be called with UActorIORegisterBase derived classes.
	 *
	 * @param RegisteredActor The actor we are registering the function for.
	 * @param Registry The list of I/O functions we are adding to.
	 * @param FunctionId Unique id of the function. Recommended format is ClassName::FunctionName.
	 * @param DisplayNameText Display name to use in the editor for this function.
	 * @param TooltipText Tooltip to use in the editor for this function.
	 * @param FunctionToExec Name of the blueprint function that should be exposed.
	 * @param SubobjectName Specific subobject to call the function on instead of the actor itself.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO|Register",
		meta = (DisplayName = "Register I/O Function For Required Actor"))
	void BP_RegisterIOFunctionForRequiredActor(AActor* RegisteredActor, UPARAM(Ref)
	                                           FActorIOFunctionList& Registry, FName FunctionId,
	                                           const FText& DisplayNameText, const FText& TooltipText,
	                                           FString FunctionToExec, FName SubobjectName);
};
