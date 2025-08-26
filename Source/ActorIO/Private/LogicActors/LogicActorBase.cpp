// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicActorBase.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"
#include "Misc/EngineVersionComparison.h"

ALogicActorBase::ALogicActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = false;
	bNetLoadOnClient = true;

#if UE_VERSION_NEWER_THAN(5, 5, 0)
	SetNetUpdateFrequency(10.0f);
#else
	NetUpdateFrequency = 10.0f;
#endif

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SceneComponent->Mobility = EComponentMobility::Static;
	RootComponent = SceneComponent;

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->Mobility = EComponentMobility::Static;
	SpriteComponent->SetupAttachment(RootComponent);

	if (!IsRunningCommandlet() && (SpriteComponent != nullptr))
	{
		// Structure to hold one-time initialization.
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture;
			FName ID_LogicActor;
			FText NAME_LogicActor;
			FConstructorStatics()
				: SpriteTexture(TEXT("/ActorIO/AssetIcons/S_LogicActor"))
				, ID_LogicActor(TEXT("LogicActor"))
				, NAME_LogicActor(NSLOCTEXT("SpriteCategory", "LogicActor", "Logic Actors"))
			{}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.SpriteTexture.Get();
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_LogicActor;
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_LogicActor;
		SpriteComponent->bIsScreenSizeScaled = true;
	}
#endif

	bEnableAutoLODGeneration = false;
	SetReplicatingMovement(false);
	SetCanBeDamaged(false);
}
