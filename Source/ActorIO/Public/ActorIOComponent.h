// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOEvent.h"
#include "ActorIOFunction.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

class UActorIOAction;

UCLASS(Blueprintable, ClassGroup = "Actor IO")
class ACTORIO_API UActorIOComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UActorIOComponent();

protected:

	UPROPERTY(Instanced, EditAnywhere) // <--- #TEMP: edit anywhere for debugging
	TArray<TObjectPtr<UActorIOAction>> Actions;

public:

	TArray<TObjectPtr<UActorIOAction>>& GetActions() { return Actions; }

	const TArray<TObjectPtr<UActorIOAction>>& GetActions() const { return Actions; }

	int32 GetNumActions() const { return Actions.Num(); }

protected:

	void BindActions();

	void UnbindActions();

	void RemoveInvalidActions();

public:

	static TArray<FActorIOEvent> GetEventsForObject(AActor* InObject);

	static TArray<FActorIOFunction> GetFunctionsForObject(AActor* InObject);

	UFUNCTION(BlueprintCallable, Category = "Actor IO", meta = (DefaultToSelf = "ContextObject"))
	static FActorIOEvent MakeIOEvent(UObject* ContextObject, FName EventId, FName EventDispatcherName);

protected:

	static TArray<FActorIOEvent> GetNativeEventsForObject(AActor* InObject);

	static TArray<FActorIOFunction> GetNativeFunctionsForObject(AActor* InObject);

public:

	static TArray<TObjectPtr<UActorIOAction>> GetInputActionsForObject(AActor* InObject);

	static int32 GetNumInputActionsForObject(AActor* InObject);

public:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};