// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicRelay.generated.h"

UCLASS()
class ACTORIO_API ALogicRelay : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicRelay();

public:

    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;
};