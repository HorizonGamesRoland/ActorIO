// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOInterface.h"
#include "GameFramework/Actor.h"
#include "LogicActorBase.generated.h"

UCLASS(Abstract, NotBlueprintable, HideCategories = (Rendering, Input, Networking, Physics, Collision, HLOD))
class ACTORIO_API ALogicActorBase : public AActor, public IActorIOInterface
{
    GENERATED_BODY()

public:

    ALogicActorBase();

#if WITH_EDITORONLY_DATA
protected:
	/** Billboard Component displayed in the editor. */
	UPROPERTY()
	TObjectPtr<class UBillboardComponent> SpriteComponent;
public:
	/** @return Billboard Component of the actor. */
	class UBillboardComponent* GetSpriteComponent() const { return SpriteComponent.Get(); };
#endif
};