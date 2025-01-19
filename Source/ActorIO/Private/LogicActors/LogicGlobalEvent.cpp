// Copyright 2025 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicGlobalEvent.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicGlobalEvent::ALogicGlobalEvent()
{
#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/S_GlobalEvent"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicGlobalEvent::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		// Do nothing in the editor.
		return;
	}

	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &ThisClass::OnWorldInitializedCallback);
	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &ThisClass::OnWorldTeardownCallback);
}

void ALogicGlobalEvent::BeginPlay()
{
	Super::BeginPlay();

	OnBeginPlay.Broadcast();
}

void ALogicGlobalEvent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FWorldDelegates::OnWorldInitializedActors.RemoveAll(this);
	FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void ALogicGlobalEvent::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicGlobalEvent::OnWorldInitialized"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.OnWorldInitialized", "OnWorldInitialized"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.OnWorldInitializedTooltip", "Event when the world has been initialized. Called after all actors have been initialized, but before 'BeginPlay'."))
		.SetMulticastDelegate(this, &OnWorldInitialized));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicGlobalEvent::OnBeginPlay"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.OnBeginPlay", "OnBeginPlay"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.OnBeginPlayTooltip", "Event when play begins for this actor."))
		.SetMulticastDelegate(this, &OnBeginPlay));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicGlobalEvent::OnWorldTeardown"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.OnWorldTeardown", "OnWorldTeardown"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.OnWorldTeardownTooltip", "Event when the world is being torn down. This means we are leaving the map. Called before 'EndPlay' is dispatched to all actors."))
		.SetMulticastDelegate(this, &OnWorldTeardown));
}

void ALogicGlobalEvent::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	// No functions.
}

void ALogicGlobalEvent::OnWorldInitializedCallback(const FActorsInitializedParams& ActorInitParams)
{
	OnWorldInitialized.Broadcast();
}

void ALogicGlobalEvent::OnWorldTeardownCallback(UWorld* World)
{
	OnWorldTeardown.Broadcast();
}

#undef LOCTEXT_NAMESPACE