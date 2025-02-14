// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOSubsystemBase.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOAction.h"
#include "ActorIOSettings.h"
#include "LogicActors/LogicActorBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/BlockingVolume.h"
#include "Engine/TriggerVolume.h"
#include "Engine/TriggerBase.h"
#include "Engine/Light.h"
#include "Particles/Emitter.h"
#include "Sound/AmbientSound.h"
#include "Sound/AudioVolume.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "ActorIO"

UActorIOSubsystemBase::UActorIOSubsystemBase()
{
    ActionExecContext = FActionExecutionContext();
}

UActorIOSubsystemBase* UActorIOSubsystemBase::Get(UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert))
    {
        UActorIOSubsystemBase* IOSubsystem = World->GetSubsystem<UActorIOSubsystemBase>();
        if (IOSubsystem)
        {
            return IOSubsystem;
        }
        else
        {
            // In the case of game worlds, do not exit gracefully with nullptr.
            // This should be treated as a critical failure.
            // The game must have a valid I/O subsystem when actions are being used.
            checkf(!World->IsGameWorld(), TEXT("Could not get I/O subsystem from game world! Ensure that I/O subsystem class is valid in 'Project Settings -> Actor I/O'."));
        }
    }

    return nullptr;
}

void UActorIOSubsystemBase::GetNativeEventsForObject(AActor* InObject, FActorIOEventList& EventRegistry)
{
    //==================================
    // Trigger Actors
    //==================================

    if (InObject->IsA<ATriggerBase>())
    {
        EventRegistry.RegisterEvent(FActorIOEvent()
            .SetId(TEXT("ATriggerBase::OnTriggerEnter"))
            .SetDisplayName(LOCTEXT("TriggerBase.OnTriggerEnter", "OnTriggerEnter"))
            .SetTooltipText(LOCTEXT("TriggerBase.OnTriggerEnterTooltip", "Event when an actor enters the trigger area."))
            .SetSparseDelegate(InObject, TEXT("OnActorBeginOverlap"))
            .SetEventProcessor(this, TEXT("ProcessEvent_OnActorOverlap")));

        EventRegistry.RegisterEvent(FActorIOEvent()
            .SetId(TEXT("ATriggerBase::OnTriggerExit"))
            .SetDisplayName(LOCTEXT("TriggerBase.OnTriggerExit", "OnTriggerExit"))
            .SetTooltipText(LOCTEXT("TriggerBase.OnTriggerExitTooltip", "Event when an actor leaves the trigger area."))
            .SetSparseDelegate(InObject, TEXT("OnActorEndOverlap"))
            .SetEventProcessor(this, TEXT("ProcessEvent_OnActorOverlap")));
    }

    //==================================
    // Non Logic Actors
    //==================================

    if (!InObject->IsA<ALogicActorBase>())
    {
        EventRegistry.RegisterEvent(FActorIOEvent()
            .SetId(TEXT("AActor::OnDestroyed"))
            .SetDisplayName(LOCTEXT("Actor.OnDestroyed", "OnDestroyed"))
            .SetTooltipText(LOCTEXT("Actor.OnDestroyedTooltip", "Event when the actor is getting destroyed."))
            .SetSparseDelegate(InObject, TEXT("OnDestroyed"))
            .SetEventProcessor(this, TEXT("ProcessEvent_OnActorDestroyed")));
    }
}

void UActorIOSubsystemBase::GetNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& FunctionRegistry)
{
    //==================================
    // Trigger Actors
    //==================================

    if (InObject->IsA<ATriggerBase>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ATriggerBase::SetEnabled"))
            .SetDisplayName(LOCTEXT("TriggerBase.SetEnabled", "SetEnabled"))
            .SetTooltipText(LOCTEXT("TriggerBase.SetEnabledTooltip", "Change whether collision is enabled for the trigger."))
            .SetFunction(TEXT("SetActorEnableCollision")));
    }

    //==================================
    // Light Actors
    //==================================

    if (InObject->IsA<ALight>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ALight::SetLightIntensity"))
            .SetDisplayName(LOCTEXT("Light.SetLightIntensity", "SetLightIntensity"))
            .SetTooltipText(LOCTEXT("Light.SetLightIntensityTooltip", "Set intensity of the light."))
            .SetFunction(TEXT("SetIntensity"))
            .SetSubobject(TEXT("LightComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ALight::SetLightColor"))
            .SetDisplayName(LOCTEXT("Light.SetLightColor", "SetLightColor"))
            .SetTooltipText(LOCTEXT("Light.SetLightColorTooltip", "Set color of the light."))
            .SetFunction(TEXT("SetLightColor"))
            .SetSubobject(TEXT("LightComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ALight::SetVisibility"))
            .SetDisplayName(LOCTEXT("Light.SetVisibility", "SetVisibility"))
            .SetTooltipText(LOCTEXT("Light.SetVisibilityTooltip", "Set visibility of the light. Use this to turn light on/off."))
            .SetFunction(TEXT("SetVisibility"))
            .SetSubobject(TEXT("LightComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ALight::SetCastShadows"))
            .SetDisplayName(FText::FromString(TEXT("SetCastShadows")))
            .SetTooltipText(FText::FromString(TEXT("Set light shadow casting on/off.")))
            .SetFunction(TEXT("SetCastShadows"))
            .SetSubobject(TEXT("LightComponent0")));
    }

    //==================================
    // Effect Actors
    //==================================

    if (InObject->IsA<AEmitter>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AEmitter::Activate"))
            .SetDisplayName(LOCTEXT("Emitter.Activate", "Activate"))
            .SetTooltipText(LOCTEXT("Emitter.ActivateTooltip", "Activate the particle system."))
            .SetFunction(TEXT("Activate"))
            .SetSubobject(TEXT("ParticleSystemComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AEmitter::Deactivate"))
            .SetDisplayName(LOCTEXT("Emitter.Deactivate", "Deactivate"))
            .SetTooltipText(LOCTEXT("Emitter.DeactivateTooltip", "Deactivate the particle system."))
            .SetFunction(TEXT("Deactivate"))
            .SetSubobject(TEXT("ParticleSystemComponent0")));
    }

    // In the case of ANiagaraActor, we are comparing the class name directly.
    // This is to avoid dependency to the Niagara module.
    // Note that this does not support class inheritance, so it only works for exact classes.
    // Since NiagaraActor is just a component wrapper, I do not think anyone will ever subclass it anyways.
    if (InObject->GetClass()->GetFName() == TEXT("NiagaraActor"))
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ANiagaraActor::Activate"))
            .SetDisplayName(LOCTEXT("NiagaraActor.Activate", "Activate"))
            .SetTooltipText(LOCTEXT("NiagaraActor.ActivateTooltip", "Activate the particle system."))
            .SetFunction(TEXT("Activate"))
            .SetSubobject(TEXT("NiagaraComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ANiagaraActor::Deactivate"))
            .SetDisplayName(LOCTEXT("NiagaraActor.Deactivate", "Deactivate"))
            .SetTooltipText(LOCTEXT("NiagaraActor.DeactivateTooltip", "Deactivate the particle system."))
            .SetFunction(TEXT("Deactivate"))
            .SetSubobject(TEXT("NiagaraComponent0")));
    }

    //==================================
    // Sound Actors
    //==================================

    if (InObject->IsA<AAmbientSound>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::FadeIn"))
            .SetDisplayName(LOCTEXT("AmbientSound.FadeIn", "FadeIn"))
            .SetTooltipText(LOCTEXT("AmbientSound.FadeInTooltip", "Smoothly start playing the sound with a fade."))
            .SetFunction(TEXT("FadeIn"))
            .SetSubobject(TEXT("AudioComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::FadeOut"))
            .SetDisplayName(LOCTEXT("AmbientSound.FadeOut", "FadeOut"))
            .SetTooltipText(LOCTEXT("AmbientSound.FadeOutTooltip", "Smoothly stop playing the sound with a fade."))
            .SetFunction(TEXT("FadeOut"))
            .SetSubobject(TEXT("AudioComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::AdjustVolume"))
            .SetDisplayName(LOCTEXT("AmbientSound.AdjustVolume", "AdjustVolume"))
            .SetTooltipText(LOCTEXT("AmbientSound.AdjustVolumeTooltip", "Smoothly adjust the volume of the sound with a fade."))
            .SetFunction(TEXT("AdjustVolume"))
            .SetSubobject(TEXT("AudioComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::Play"))
            .SetDisplayName(LOCTEXT("AmbientSound.Play", "Play"))
            .SetTooltipText(LOCTEXT("AmbientSound.PlayTooltip", "Start playing the sound. Start time can be given."))
            .SetFunction(TEXT("Play"))
            .SetSubobject(TEXT("AudioComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::Stop"))
            .SetDisplayName(LOCTEXT("AmbientSound.Stop", "Stop"))
            .SetTooltipText(LOCTEXT("AmbientSound.StopTooltip", "Stop playing the sound."))
            .SetFunction(TEXT("Stop"))
            .SetSubobject(TEXT("AudioComponent0")));
    }

    //==================================
    // Volume Actors
    //==================================

    if (InObject->IsA<ABlockingVolume>() || InObject->IsA<ACameraBlockingVolume>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ABlockingVolume::SetEnabled"))
            .SetDisplayName(LOCTEXT("BlockingVolume.SetEnabled", "SetEnabled"))
            .SetTooltipText(LOCTEXT("BlockingVolume.SetEnabledTooltip", "Change whether collision is enabled for the volume actor."))
            .SetFunction(TEXT("SetActorEnableCollision")));
    }

    if (InObject->IsA<AAudioVolume>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AAudioVolume::SetEnabled"))
            .SetDisplayName(LOCTEXT("AudioVolume.SetEnabled", "SetEnabled"))
            .SetTooltipText(LOCTEXT("AudioVolume.SetEnabledTooltip", "Set whether the audio volume is enabled or not."))
            .SetFunction(TEXT("SetEnabled")));
    }

    //==================================
    // Mesh Actors
    //==================================

    if (InObject->IsA<AStaticMeshActor>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AStaticMeshActor::SetEnableCollision"))
            .SetDisplayName(LOCTEXT("StaticMeshActor.SetEnableCollision", "SetEnableCollision"))
            .SetTooltipText(LOCTEXT("StaticMeshActor.SetEnableCollisionTooltip", "Set whether collision is enabled for the actor."))
            .SetFunction(TEXT("SetActorEnableCollision")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AStaticMeshActor::SetSimulatePhysics"))
            .SetDisplayName(LOCTEXT("StaticMeshActor.SetSimulatePhysics", "SetSimulatePhysics"))
            .SetTooltipText(LOCTEXT("StaticMeshActor.SetSimulatePhysicsTooltip", "Set physics simulation on/off."))
            .SetFunction(TEXT("SetSimulatePhysics"))
            .SetSubobject(TEXT("StaticMeshComponent0")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AStaticMeshActor::SetHiddenInGame"))
            .SetDisplayName(LOCTEXT("StaticMeshActor.SetHiddenInGame", "SetHiddenInGame"))
            .SetTooltipText(LOCTEXT("StaticMeshActor.SetHiddenInGameTooltip", "Set whether the actor is hidden or not."))
            .SetFunction(TEXT("SetActorHiddenInGame")));
    }

    //==================================
    // Non Logic Actors
    //==================================

    if (!InObject->IsA<ALogicActorBase>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("AActor::Destroy"))
            .SetDisplayName(LOCTEXT("Actor.Destroy", "Destroy"))
            .SetTooltipText(LOCTEXT("Actor.DestroyTooltip", "Destroy the actor."))
            .SetFunction(TEXT("K2_DestroyActor")));
    }
}

void UActorIOSubsystemBase::GetGlobalNamedArguments(FActionExecutionContext& ExecutionContext)
{
    // Make the local player pawn always accessible as an argument for all functions.
    const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    ExecutionContext.SetNamedArgument(TEXT("$Player"), IsValid(PlayerPawn) ? PlayerPawn->GetPathName() : FString());

    // Give blueprint layer a chance to add global named arguments.
    K2_GetGlobalNamedArguments();
}

void UActorIOSubsystemBase::ProcessEvent_OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    ActionExecContext.SetNamedArgument(TEXT("$Actor"), IsValid(OtherActor) ? OtherActor->GetPathName() : FString());
}

void UActorIOSubsystemBase::ProcessEvent_OnActorDestroyed(AActor* DestroyedActor)
{
    ActionExecContext.SetNamedArgument(TEXT("$Actor"), IsValid(DestroyedActor) ? DestroyedActor->GetPathName() : FString());
}

bool UActorIOSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
{
    // Determine whether this specific subsystem should be created or not.
    // Subsystems are registered with the engine automatically, but we only want one specific subsystem.

    if (!Super::ShouldCreateSubsystem(Outer))
    {
        return false;
    }

    UClass* ThisClass = GetClass();

    const UActorIOSettings* IOSettings = UActorIOSettings::Get();
    if (IOSettings->ActorIOSubsystemClass != nullptr)
    {
        // A subsystem class is provided so only create if we are that class.
        return ThisClass == IOSettings->ActorIOSubsystemClass;
    }
    else
    {
        // No subsystem class was provided so use the base implementation.
        UE_LOG(LogActorIO, Error, TEXT("No Actor I/O Subsystem is specified in Actor I/O settings! Reverting to default implementation."));
        return ThisClass == UActorIOSubsystemBase::StaticClass();
    }
}

#undef LOCTEXT_NAMESPACE
