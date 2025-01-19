// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOInterface.h"
#include "GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "LogicActorBase.generated.h"

/** A dynamic multicast delegate with no params. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleActionDelegate);

/**
 * An actor that can be used to expose logic to the I/O system.
 */
UCLASS(Abstract, HideCategories = (Rendering, Input, Networking, Physics, Collision, HLOD))
class ACTORIO_API ALogicActorBase : public AActor, public IActorIOInterface
{
    GENERATED_BODY()

public:

	/** Default constructor. */
    ALogicActorBase();

protected:

	//~ Begin ALogicActorBase Interface
	virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override {}
	virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override {}
	//~ End ALogicActorBase Interface

protected:

#if WITH_EDITORONLY_DATA
	/** Billboard component displayed in the editor. */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

	/** Get the editor only billboard component of the actor. */
	UFUNCTION(BlueprintPure, Category = "Components", meta = (DevelopmentOnly))
	UBillboardComponent* GetEditorSpriteComponent() const;
};