// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/BlockingVolume.h"
#include "Engine/TriggerVolume.h"
#include "Engine/TriggerBase.h"
#include "Engine/Light.h"
#include "Particles/Emitter.h"
#include "NiagaraActor.h"
#include "Sound/AmbientSound.h"
#include "Sound/AudioVolume.h"

#define LOCTEXT_NAMESPACE "ActorIO"

UActorIOSystem::UActorIOSystem()
{
    ActionExecContext = FActionExecutionContext();
}

FActorIOEventList UActorIOSystem::GetEventsForObject(AActor* InObject)
{
    FActorIOEventList OutEvents = FActorIOEventList();
    if (IsValid(InObject))
    {
        if (InObject->Implements<UActorIOInterface>())
        {
            IActorIOInterface::Execute_RegisterIOEvents(InObject, OutEvents);
        }

        GetNativeEventsForObject(InObject, OutEvents);
    }
    
    return OutEvents;
}

FActorIOFunctionList UActorIOSystem::GetFunctionsForObject(AActor* InObject)
{
    FActorIOFunctionList OutFunctions = FActorIOFunctionList();
    if (IsValid(InObject))
    {
        if (InObject->Implements<UActorIOInterface>())
        {
            IActorIOInterface::Execute_RegisterIOFunctions(InObject, OutFunctions);
        }

        GetNativeFunctionsForObject(InObject, OutFunctions);
    }

    return OutFunctions;
}

TArray<TWeakObjectPtr<UActorIOAction>> UActorIOSystem::GetInputActionsForObject(AActor* InObject)
{
    TArray<TWeakObjectPtr<UActorIOAction>> OutActions = TArray<TWeakObjectPtr<UActorIOAction>>();
    if (IsValid(InObject))
    {
        for (TObjectIterator<UActorIOAction> ActionItr; ActionItr; ++ActionItr)
        {
            UActorIOAction* Action = *ActionItr;
            if (IsValid(Action) && IsValid(Action->GetOwnerActor()) && Action->TargetActor == InObject)
            {
                OutActions.Add(Action);
            }
        }
    }

    return OutActions;
}

int32 UActorIOSystem::GetNumInputActionsForObject(AActor* InObject)
{
    TArray<TWeakObjectPtr<UActorIOAction>> InputActions = GetInputActionsForObject(InObject);
    return InputActions.Num();
}

void UActorIOSystem::GetNativeEventsForObject(AActor* InObject, FActorIOEventList& RegisteredEvents)
{
    UWorld* ObjectWorld = InObject->GetWorld();
    UActorIOSystem* IOSystem = ObjectWorld->GetSubsystem<UActorIOSystem>();

    if (InObject->IsA<ATriggerBase>())
    {
        RegisteredEvents.Add(FActorIOEvent()
            .SetId(TEXT("ATriggerBase::OnTriggerEnter"))
            .SetDisplayName(LOCTEXT("TriggerBase.OnTriggerEnter", "OnTriggerEnter"))
            .SetTooltipText(LOCTEXT("TriggerBase.OnTriggerEnterTooltip", "Event when an actor enters the trigger area."))
            .SetSparseDelegate(InObject, TEXT("OnActorBeginOverlap"))
            .SetEventProcessor(IOSystem, TEXT("ProcessEvent_OnActorOverlap")));

        RegisteredEvents.Add(FActorIOEvent()
            .SetId(TEXT("ATriggerBase::OnTriggerExit"))
            .SetDisplayName(LOCTEXT("TriggerBase.OnTriggerExit", "OnTriggerExit"))
            .SetTooltipText(LOCTEXT("TriggerBase.OnTriggerExitTooltip", "Event when an actor leaves the trigger area."))
            .SetSparseDelegate(InObject, TEXT("OnActorEndOverlap"))
            .SetEventProcessor(IOSystem, TEXT("ProcessEvent_OnActorOverlap")));
    }
}

void UActorIOSystem::GetNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& RegisteredFunctions)
{
    //==================================
    // Light Actors
    //==================================

    if (InObject->IsA<ALight>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ALight::SetLightIntensity"))
            .SetDisplayName(LOCTEXT("Light.SetLightIntensity", "SetLightIntensity"))
            .SetTooltipText(LOCTEXT("Light.SetLightIntensityTooltip", "Set intensity of the light."))
            .SetFunction(TEXT("SetIntensity"))
            .SetSubobject(TEXT("LightComponent0")));

        // #TODO: Test how this works
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ALight::SetLightColor"))
            .SetDisplayName(LOCTEXT("Light.SetLightColor", "SetLightColor"))
            .SetTooltipText(LOCTEXT("Light.SetLightColorTooltip", "Set color of the light."))
            .SetFunction(TEXT("SetLightFColor"))
            .SetSubobject(TEXT("LightComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ALight::SetVisibility"))
            .SetDisplayName(LOCTEXT("Light.SetVisibility", "SetVisibility"))
            .SetTooltipText(LOCTEXT("Light.SetVisibilityTooltip", "Set visibility of the light. Use this to turn light on/off."))
            .SetFunction(TEXT("SetVisibility"))
            .SetSubobject(TEXT("LightComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
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
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AEmitter::Activate"))
            .SetDisplayName(LOCTEXT("Emitter.Activate", "Activate"))
            .SetTooltipText(LOCTEXT("Emitter.ActivateTooltip", "Activate the particle system."))
            .SetFunction(TEXT("Activate"))
            .SetSubobject(TEXT("ParticleSystemComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AEmitter::Deactivate"))
            .SetDisplayName(LOCTEXT("Emitter.Deactivate", "Deactivate"))
            .SetTooltipText(LOCTEXT("Emitter.DeactivateTooltip", "Deactivate the particle system."))
            .SetFunction(TEXT("Deactivate"))
            .SetSubobject(TEXT("ParticleSystemComponent0")));
    }

    if (InObject->IsA<ANiagaraActor>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ANiagaraActor::Activate"))
            .SetDisplayName(LOCTEXT("NiagaraActor.Activate", "Activate"))
            .SetTooltipText(LOCTEXT("NiagaraActor.ActivateTooltip", "Activate the particle system."))
            .SetFunction(TEXT("Activate"))
            .SetSubobject(TEXT("NiagaraComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
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
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::FadeIn"))
            .SetDisplayName(LOCTEXT("AmbientSound.FadeIn", "FadeIn"))
            .SetTooltipText(LOCTEXT("AmbientSound.FadeInTooltip", "Smoothly start playing the sound with a fade."))
            .SetFunction(TEXT("FadeIn"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::FadeOut"))
            .SetDisplayName(LOCTEXT("AmbientSound.FadeOut", "FadeOut"))
            .SetTooltipText(LOCTEXT("AmbientSound.FadeOutTooltip", "Smoothly stop playing the sound with a fade."))
            .SetFunction(TEXT("FadeOut"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::AdjustVolume"))
            .SetDisplayName(LOCTEXT("AmbientSound.AdjustVolume", "AdjustVolume"))
            .SetTooltipText(LOCTEXT("AmbientSound.AdjustVolumeTooltip", "Smoothly adjust the volume of the sound with a fade."))
            .SetFunction(TEXT("AdjustVolume"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::Play"))
            .SetDisplayName(LOCTEXT("AmbientSound.Play", "Play"))
            .SetTooltipText(LOCTEXT("AmbientSound.PlayTooltip", "Start playing the sound. Start time can be given."))
            .SetFunction(TEXT("Play"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
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
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ABlockingVolume::SetEnabled"))
            .SetDisplayName(LOCTEXT("BlockingVolume.SetEnabled", "SetEnabled"))
            .SetTooltipText(LOCTEXT("BlockingVolume.SetEnabledTooltip", "Change whether collision is enabled for the volume actor."))
            .SetFunction(TEXT("SetActorEnableCollision")));
    }

    if (InObject->IsA<AAudioVolume>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
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
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AStaticMeshActor::SetEnableCollision"))
            .SetDisplayName(LOCTEXT("StaticMeshActor.SetEnableCollision", "SetEnableCollision"))
            .SetTooltipText(LOCTEXT("StaticMeshActor.SetEnableCollisionTooltip", "Set whether collision is enabled for the actor."))
            .SetFunction(TEXT("SetActorEnableCollision")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AStaticMeshActor::SetSimulatePhysics"))
            .SetDisplayName(LOCTEXT("StaticMeshActor.SetSimulatePhysics", "SetSimulatePhysics"))
            .SetTooltipText(LOCTEXT("StaticMeshActor.SetSimulatePhysicsTooltip", "Set physics simulation on/off."))
            .SetFunction(TEXT("SetSimulatePhysics"))
            .SetSubobject(TEXT("StaticMeshComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AStaticMeshActor::SetHiddenInGame"))
            .SetDisplayName(LOCTEXT("StaticMeshActor.SetHiddenInGame", "SetHiddenInGame"))
            .SetTooltipText(LOCTEXT("StaticMeshActor.SetHiddenInGameTooltip", "Set whether the actor is hidden or not."))
            .SetFunction(TEXT("SetActorHiddenInGame")));
    }

    //==================================
    // Base Actor
    //==================================

    RegisteredFunctions.Add(FActorIOFunction()
        .SetId(TEXT("AActor::Destroy"))
        .SetDisplayName(LOCTEXT("Actor.Destroy", "Destroy"))
        .SetTooltipText(LOCTEXT("Actor.DestroyTooltip", "Destroy the actor."))
        .SetFunction(TEXT("K2_DestroyActor")));
}

void UActorIOSystem::ProcessEvent_OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    ActionExecContext.SetNamedArgument(TEXT("$Actor"), IsValid(OtherActor) ? OtherActor->GetPathName() : FString());
}

void UActorIOSystem::RegisterIOEvent(UObject* WorldContextObject, TArray<FActorIOEvent>& RegisterTo, FName EventId, const FText& DisplayName, const FText& TooltipText, FName EventDispatcherName)
{
    RegisterTo.Add(FActorIOEvent()
        .SetId(EventId)
        .SetDisplayName(DisplayName)
        .SetTooltipText(TooltipText)
        .SetBlueprintDelegate(WorldContextObject, EventDispatcherName));
}

void UActorIOSystem::RegisterIOFunction(UObject* WorldContextObject, TArray<FActorIOFunction>& RegisterTo, FName FunctionId, const FText& DisplayName, const FText& TooltipText, FString FunctionToExec, FName SubobjectName)
{
    RegisterTo.Add(FActorIOFunction()
        .SetId(FunctionId)
        .SetDisplayName(DisplayName)
        .SetTooltipText(TooltipText)
        .SetFunction(FunctionToExec)
        .SetSubobject(SubobjectName));
}

#undef LOCTEXT_NAMESPACE
