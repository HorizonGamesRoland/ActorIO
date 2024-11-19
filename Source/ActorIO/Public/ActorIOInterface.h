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

class ACTORIO_API IActorIOInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register IO Events", meta = (ForceAsFunction))
	void RegisterIOEvents(UPARAM(Ref) TArray<FActorIOEvent>& RegisteredEvents);
	virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) {}

	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register IO Functions", meta = (ForceAsFunction))
	void RegisterIOFunctions(UPARAM(Ref) TArray<FActorIOFunction>& RegisteredFunctions);
	virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) {}
};