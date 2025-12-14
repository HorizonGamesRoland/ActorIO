// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

class UActorIOAction;

/**
 * Component that manages I/O actions for the actor it is attached to.
 */
UCLASS(ClassGroup = "Actor IO", DisplayName = "Actor I/O Component")
class ACTORIO_API UActorIOComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UActorIOComponent();

protected:

	/** List of I/O actions managed by the component for the owning actor. */
	UPROPERTY(Instanced, EditInstanceOnly, EditFixedSize, NoClear, Category = "ActorIO")
	TArray<TObjectPtr<UActorIOAction>> Actions;

public:

	/** Creates a new I/O action and adds it to the action list. */
	UActorIOAction* CreateNewAction();

	/** Removes the given action from the action list, and destroys it. */
	void RemoveAction(UActorIOAction* InAction);

	/** Removes all entries from the action list that are nullptr. */
	void RemoveInvalidActions();

	/**
	 * Rearranges the action list by moving the action from original index to new index.
	 * Used for drag & drop in the editor.
	 */
	void MoveAction(int32 OriginalIndex, int32 NewIndex);

	/** @return List I/O actions managed by the component. */
	const TArray<TObjectPtr<UActorIOAction>>& GetActions() const { return Actions; }

	/** @return Number of actions. */
	int32 GetNumActions() const { return Actions.Num(); }

protected:

	/** Binds all actions to their assigned I/O events. */
	void BindActions();

	/** Unbinds all actions. */
	void UnbindActions();

public:

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	//~ End UActorComponent Interface
};