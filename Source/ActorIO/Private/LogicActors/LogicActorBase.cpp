// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#include "LogicActors/LogicActorBase.h"

ALogicActorBase::ALogicActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = false;
	bNetLoadOnClient = true;
	SetNetUpdateFrequency(10.0f);

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	RootComponent = SpriteComponent;

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

UBillboardComponent* ALogicActorBase::GetEditorSpriteComponent() const
{
#if WITH_EDITORONLY_DATA
	return SpriteComponent.Get();
#else
	return nullptr;
#endif
}
