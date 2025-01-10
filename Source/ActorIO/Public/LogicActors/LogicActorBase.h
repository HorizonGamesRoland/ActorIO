// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOInterface.h"
#include "GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "LogicActorBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleActionDelegate);

UCLASS(Abstract, HideCategories = (Rendering, Input, Networking, Physics, Collision, HLOD))
class ACTORIO_API ALogicActorBase : public AActor, public IActorIOInterface
{
    GENERATED_BODY()

public:

    ALogicActorBase();

protected:

#if WITH_EDITORONLY_DATA
	/** Billboard component displayed in the editor. */
	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

protected:

	/** Get the editor only billboard component of the actor. */
	UFUNCTION(BlueprintPure, Category = "Components", meta = (DevelopmentOnly))
	UBillboardComponent* GetEditorSpriteComponent() const;
};