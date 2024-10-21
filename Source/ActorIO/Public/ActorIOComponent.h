// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOTypes.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActorIOTestDelegate, int32, TestNum);

UCLASS(Blueprintable, ClassGroup = ActorIO, meta = (BlueprintSpawnableComponent))
class ACTORIO_API UActorIOComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	UPROPERTY()
	TArray<FActorIOAction> ActionBindings;

	UPROPERTY()
	TArray<FActorIOEvent> MappedEvents;

	UPROPERTY()
	TArray<FActorIOFunction> MappedFunctions;





	UPROPERTY(BlueprintAssignable)
	FActorIOTestDelegate TestEvent;

public:

	void AddAction(const FActorIOAction& NewAction);

	void RemoveAction(const FGuid& ActionId);

	FActorIOAction* FindAction(const FGuid& ActionId);

protected:

	void InitializeMappings();

	void BindAction(const FActorIOAction& Action);

	void RemoveActionBindings();

protected:

	UFUNCTION()
	void SomethingOnTestEvent();

public:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};