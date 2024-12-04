// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

class UActorIOAction;
struct FActorIOMessage;

UCLASS(Blueprintable, ClassGroup = "Actor IO")
class ACTORIO_API UActorIOComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UActorIOComponent();

protected:

	UPROPERTY(Instanced, EditAnywhere) // <--- #TEMP: edit anywhere for debugging
	TArray<TObjectPtr<UActorIOAction>> Actions;

	UPROPERTY()
	TSet<FActorIOMessage> MessageStack;

public:

	UActorIOAction* CreateNewAction();

	void RemoveAction(UActorIOAction* InAction);

	void RemoveInvalidActions();

	TArray<TWeakObjectPtr<UActorIOAction>> GetActions() const;

	int32 GetNumActions() const { return Actions.Num(); }

protected:

	void BindActions();

	void UnbindActions();

public:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};