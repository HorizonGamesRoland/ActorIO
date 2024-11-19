// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOEvent.h"
#include "ActorIOFunction.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

struct FActorIOAction;
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

	int32 GetNumActions() const { return Actions.Num(); }

protected:

	void CreateActionBindings();

	void RemoveActionBindings();

public:

	static TArray<FActorIOEvent> GetEventsForObject(AActor* InObject);

	static TArray<FActorIOFunction> GetFunctionsForObject(AActor* InObject);

	UFUNCTION(BlueprintCallable, Category = "Actor IO", meta = (DefaultToSelf = "ContextObject"))
	static FActorIOEvent MakeIOEvent(UObject* ContextObject, FName EventId, FName EventDispatcherName);

protected:

	static TArray<FActorIOEvent> GetNativeEventsForObject(AActor* InObject);

	static TArray<FActorIOFunction> GetNativeFunctionsForObject(AActor* InObject);

public:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};