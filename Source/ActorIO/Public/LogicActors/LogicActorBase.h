// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "ActorIOInterface.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
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

	/** The component that defines the transform (location, rotation, scale). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneComponent;

#if WITH_EDITORONLY_DATA
	/** Billboard component displayed in the editor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

public:

	/**
	 * Called before the logic actor is serialized.
	 * Can be used to update save properties before they are saved.
	 * This function is public to support direct serialization using UObject::SerializeScriptProperties.
	 */
	virtual void PreSerializeLogicActor(FArchive& Ar) {}

	/**
	 * Called after the logic actor is serialized.
	 * Can be used to apply the restored save properties on the actor.
	 * This function is public to support direct serialization using UObject::SerializeScriptProperties.
	 */
	virtual void PostSerializeLogicActor(FArchive& Ar) {}

public:

	//~Begin AActor Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	//~End AActor Interface

	//~ Begin IActorIOInterface Implementation
	virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override {}
	virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override {}
	//~ End IActorIOInterface Implementation
};