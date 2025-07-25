// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "UObject/NoExportTypes.h"
#include "ActorIOAction.generated.h"

class UActorIOComponent;

/**
 * An action that is executed when the assigned I/O event is triggered.
 * When executed, it calls the designated function on the target actor with the given parameters.
 * This is basically the message bus of the I/O system.
 */
UCLASS(DefaultToInstanced, EditInlineNew, Within = ActorIOComponent, DisplayName = "Actor I/O Action")
class ACTORIO_API UActorIOAction : public UObject
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UActorIOAction();

public:

	/** Id of the I/O event to bind to. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	FName EventId;

	/** The execution target. The designated function will be called on this actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	TObjectPtr<AActor> TargetActor;

	/** Id of the I/O function to call on the target actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	FName FunctionId;

	/**
	 * Parameters that are sent with the function.
	 * Format is: Arg1; Arg2; Arg3; ...
	 * Also supports named arguments in the form of $ArgName.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	FString FunctionArguments;

	/** Time before the function is called on the target actor, after the action is executed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	float Delay;

	/**
	 * Whether the action can only be executed once.
	 * If false, the action will be executed every time the assigned I/O event is triggered.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	bool bExecuteOnlyOnce;

protected:

	/** Whether the action was executed before. */
	bool bWasExecuted;

	/** Whether the action is bound to the assigned I/O event. */
	bool bIsBound;

	/** The delegate that is bound to the assigned I/O event. */
	FScriptDelegate ActionDelegate;

public:

	/** Bind the action to the assigned I/O event. */
	void BindAction();

	/** Unbind the action. */
	void UnbindAction();

	/** Get the I/O component that owns this action. */
	UFUNCTION(BlueprintPure, Category = "Action")
	UActorIOComponent* GetOwnerIOComponent() const;

	/** Get the parent actor of the I/O component that owns this action. */
	UFUNCTION(BlueprintPure, Category = "Action")
	AActor* GetOwnerActor() const;

	/**
	 * Get the object that the final command will be sent to by this action.
	 * In most cases this will be the target actor, but the I/O function may want it to be executed on a subobject rather then the actor itself.
	 * This function is public because we might want to access the target object from outside this class as well.
	 * 
	 * @param TargetFunction Optimization in case we already know which I/O function is called by this action.
	 */
	UObject* ResolveTargetObject(const FActorIOFunction* TargetFunction = nullptr) const;

	/**
	 * Get the UFunction that this action will be calling based on its parameters.
	 * This function is public because we might want to access the UFunction from outside this class as well.
	 *
	 * @param TargetFunction Optimization in case we already know which I/O function is called by this action.
	 * @param TargetObject Optimization in case we already know which object is targeted by this action.
	 */
	UFunction* ResolveUFunction(const FActorIOFunction* TargetFunction = nullptr, UObject* TargetObject = nullptr) const;

protected:

	/**
	 * Name of the function that is bound to the assigned I/O event.
	 * This function will be used as a notify so that we can keep track of the global action execution state.
	 * By default this is assigned to the "ReceiveExecuteAction" function below.
	 */
	static FName ExecuteActionSignalName;

	/**
	 * Entry point when the assigned I/O event is triggered.
	 * This function does not do anything on its own.
	 * It is simply used as a notify so that we can keep track of the global action execution state.
	 */
	UFUNCTION()
	void ReceiveExecuteAction();

	/** Executes the action. */
	void ExecuteAction(FActionExecutionContext& ExecutionContext);

	/**
	 * Sends the final command to the target actor.
	 * The command contains the function name and parameters that will be processed by Unreal Script.
	 */
	void SendCommand(UObject* Target, FString Command);

public:

	//~ Begin UObject Interface
	virtual void ProcessEvent(UFunction* Function, void* Parms) override;
	//~ End UObject Interface
};
