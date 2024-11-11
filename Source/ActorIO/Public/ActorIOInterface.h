// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOEvent.h"
#include "ActorIOFunction.h"
#include "UObject/Interface.h"
#include "ActorIOInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UActorIOInterface : public UInterface
{
	GENERATED_BODY()
};

class IActorIOInterface
{
	GENERATED_BODY()

public:

	void GetActorIOEvents(TArray<FActorIOEvent>& OutEvents) const;

	void GetActorIOFunctions(TArray<FActorIOFunction>& OutFunctions) const;

protected:

	virtual void RegisterIOEvents(TArray<FActorIOEvent>& RegisteredEvents) const {}

	virtual void RegisterIOFunctions(TArray<FActorIOFunction>& RegisteredFunctions) const {}

	UFUNCTION(BlueprintImplementableEvent, Category = "Actor IO", DisplayName = "Register IO Events")
	void K2_RegisterIOEvents(UPARAM(Ref) TArray<FActorIOEvent>& RegisteredEvents) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Actor IO", DisplayName = "Register IO Functions")
	void K2_RegisterIOFunctions(UPARAM(Ref) TArray<FActorIOFunction>& RegisteredFunctions) const;
};