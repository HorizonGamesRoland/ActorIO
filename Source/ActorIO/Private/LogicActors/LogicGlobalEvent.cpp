// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicGlobalEvent.h"
#include "ActorIOSubsystemBase.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/LevelScriptActor.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicGlobalEvent::ALogicGlobalEvent()
{
#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_GlobalEvent"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicGlobalEvent::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicGlobalEvent::OnWorldInitialized"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.OnWorldInitialized", "OnWorldInitialized"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.OnWorldInitializedTooltip", "Event when the game world is initialized. Only called once, after loading the persistent level, before 'BeginPlay' is dispatched."))
		.SetMulticastDelegate(this, &OnWorldInitialized));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicGlobalEvent::OnBeginPlay"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.OnBeginPlay", "OnBeginPlay"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.OnBeginPlayTooltip", "Event when 'BeginPlay' is called for this actor."))
		.SetMulticastDelegate(this, &OnActorBeginPlay));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicGlobalEvent::OnEndPlay"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.OnEndPlay", "OnEndPlay"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.OnEndPlayTooltip", "Event when 'EndPlay' is called for this actor."))
		.SetMulticastDelegate(this, &OnActorEndPlay));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicGlobalEvent::OnWorldTeardown"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.OnWorldTeardown", "OnWorldTeardown"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.OnWorldTeardownTooltip", "Event when the game world is being torn down. This means we are leaving the map. Called before 'EndPlay' is dispatched to all actors."))
		.SetMulticastDelegate(this, &OnWorldTeardown));
}

void ALogicGlobalEvent::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicGlobalEvent::CallLevelBlueprintFunction"))
		.SetDisplayName(LOCTEXT("ALogicGlobalEvent.CallLevelBlueprintFunction", "CallLevelBlueprintFunction"))
		.SetTooltipText(LOCTEXT("ALogicGlobalEvent.CallLevelBlueprintFunctionTooltip", "Call a function on the level blueprint of the level. Can pass in function params in following format: \"FuncName Param1 Param2 Param3 ...\""))
		.SetFunction(TEXT("CallLevelBlueprintFunction")));
}

void ALogicGlobalEvent::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
		FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &ThisClass::OnWorldInitializedCallback);
		FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &ThisClass::OnWorldTeardownCallback);
	}
}

void ALogicGlobalEvent::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginPlay.Broadcast();
}

void ALogicGlobalEvent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FWorldDelegates::OnWorldInitializedActors.RemoveAll(this);
	FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);

	OnActorEndPlay.Broadcast();

	Super::EndPlay(EndPlayReason);
}

void ALogicGlobalEvent::OnWorldInitializedCallback(const FActorsInitializedParams& ActorInitParams)
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld == ActorInitParams.World)
	{
		OnWorldInitialized.Broadcast();
	}
}

void ALogicGlobalEvent::OnWorldTeardownCallback(UWorld* World)
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld == World)
	{
		OnWorldTeardown.Broadcast();
	}
}

void ALogicGlobalEvent::CallLevelBlueprintFunction(FString Command)
{
	ALevelScriptActor* LevelScriptActor = GetLevel()->GetLevelScriptActor();
	if (IsValid(LevelScriptActor))
	{
		FStringOutputDevice Ar;

		// Invoke the function.
		UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(this);
		IOSubsystem->ExecuteCommand(LevelScriptActor, *Command, Ar, this);

		// Log execution errors.
		if (!Ar.IsEmpty())
		{
			UE_LOG(LogActorIO, Error, TEXT("%s"), *Ar);
		}
	}
}

#undef LOCTEXT_NAMESPACE