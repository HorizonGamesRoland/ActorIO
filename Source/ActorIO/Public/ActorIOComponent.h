// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

class UActorIOAction;

/**
 * The component that manages I/O actions for the actor.
 * Acts as an interface for the I/O system.
 */
UCLASS(Blueprintable, ClassGroup = "Actor IO")
class ACTORIO_API UActorIOComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UActorIOComponent();

protected:

	/** List of actions tied to the component. */
	UPROPERTY(Instanced, EditInstanceOnly)
	TArray<TObjectPtr<UActorIOAction>> Actions;

public:

	/** Creates a new I/O action and adds it to the action list. */
	UActorIOAction* CreateNewAction();

	/** Removes the given action from the action list, and destroys it. */
	void RemoveAction(UActorIOAction* InAction);

	/** Removes all entries from the action list that are nullptr. */
	void RemoveInvalidActions();

	/** @return Weak reference to all I/O actions. */
	TArray<TWeakObjectPtr<UActorIOAction>> GetActions() const;

	/** @return Number of actions. */
	int32 GetNumActions() const { return Actions.Num(); }

	/** @return Delay of the longest delayed action. */
	float GetDurationOfLongestDelay() const;

protected:

	/** Binds all actions to their assigned I/O events. */
	void BindActions();

	/** Unbinds all actions. */
	void UnbindActions();

public:

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//~ End UActorComponent Interface
};