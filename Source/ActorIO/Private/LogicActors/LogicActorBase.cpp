// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicActorBase.h"
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

	bLogicActorIsReady = false;
}

void ALogicActorBase::BeginPlay()
{
	Super::BeginPlay();

	ULevel* Level = GetLevel();
	if (Level && Level->IsPersistentLevel())
	{
		ReadyForPlay();
	}
	else
	{
		DelegateHandle_OnLevelAddedToWorld = FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAddedToWorldCallback);
	}
}

void ALogicActorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FWorldDelegates::LevelAddedToWorld.Remove(DelegateHandle_OnLevelAddedToWorld);

	Super::EndPlay(EndPlayReason);
}

void ALogicActorBase::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	if (!bAllowTickBeforeBeginPlay && bLogicActorIsReady)
	{
		// Do not tick until logic actor has begun play.
		return;
	}

	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void ALogicActorBase::ReadyForPlay()
{
	bLogicActorIsReady = true;
	K2_ReadyForPlay();
}

void ALogicActorBase::OnLevelAddedToWorldCallback(ULevel* InLevel, UWorld* InWorld)
{
	UWorld* MyWorld = GetWorld();
	if (!MyWorld || InWorld != MyWorld)
	{
		// Do nothing if this was not called for our world.
		// This can happen when running multiple PIE instances.
		return;
	}

	ULevel* MyLevel = GetLevel();
	if (InLevel == MyLevel)
	{
		// Callback was for the actors own level, so we are ready.
		// At this point the level has received the "bVisible" state, meaning it is now active.
		// @see IActorIO::ConfirmObjectIsAlive

		FWorldDelegates::LevelAddedToWorld.Remove(DelegateHandle_OnLevelAddedToWorld);
		ReadyForPlay();
	}
}
