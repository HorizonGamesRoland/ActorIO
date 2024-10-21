// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOTypes.h"
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

	virtual void GetActorIOEvents(TArray<FActorIOEvent>& OutEvents) {}

	virtual void GetActorIOFunctions(TArray<FActorIOFunction>& OutFunctions) {}
};