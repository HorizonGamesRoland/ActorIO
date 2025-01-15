// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

class UActorIOAction;

/**
 * Component that manages I/O actions for the actor it is attached to.
 */
UCLASS(Blueprintable, ClassGroup = "Actor IO", DisplayName = "Actor I/O Component")
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
	virtual UActorIOAction* CreateNewAction();

	/** Removes the given action from the action list, and destroys it. */
	virtual void RemoveAction(UActorIOAction* InAction);

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