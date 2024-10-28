// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOTypes.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

class UActorIOLink;

UCLASS(Blueprintable, ClassGroup = ActorIO, meta = (BlueprintSpawnableComponent))
class ACTORIO_API UActorIOComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UActorIOComponent();

protected:

	UPROPERTY(EditAnywhere) // <--- #TEMP: edit anywhere for debugging
	TArray<FActorIOAction> Actions;

	UPROPERTY()
	TArray<TObjectPtr<UActorIOLink>> ActionBindings;

public:

	TArray<FActorIOAction>& GetActions() { return Actions; }

	const TArray<FActorIOAction>& GetActions() const { return Actions; }

	const TArray<FActorIOEvent> GetEvents() const;

	const TArray<FActorIOFunction> GetFunctions() const;

protected:

	void CreateActionBindings();

	void RemoveActionBindings();



	UFUNCTION()
	void TestHelloWorld();

public:

	static TArray<FActorIOEvent> GetNativeEventsForObject(UObject* InObject);

public:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};