// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "UObject/Interface.h"
#include "ActorIOInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UActorIOInterface : public UInterface
{
	GENERATED_BODY()
};

class ACTORIO_API IActorIOInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, CallInEditor, Category = "Actor IO", DisplayName = "Register IO Events", meta = (ForceAsFunction))
	void RegisterIOEvents(UPARAM(Ref) FActorIOEventList& RegisteredEvents);
	virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, CallInEditor, Category = "Actor IO", DisplayName = "Register IO Functions", meta = (ForceAsFunction))
	void RegisterIOFunctions(UPARAM(Ref) FActorIOFunctionList& RegisteredFunctions);
	virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) {}
};