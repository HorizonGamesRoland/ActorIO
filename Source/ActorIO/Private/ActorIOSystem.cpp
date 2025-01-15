// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Volume.h"
#include "Engine/TriggerBase.h"
#include "Engine/Light.h"
#include "Particles/Emitter.h"
#include "NiagaraActor.h"
#include "Sound/AmbientSound.h"

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
    if (InObject->IsA<ATriggerBase>())
    {
        RegisteredEvents.Add(FActorIOEvent()
            .SetId(TEXT("ATriggerBase::OnTriggerEnter"))
            .SetDisplayName(FText::FromString(TEXT("OnTriggerEnter")))
            .SetTooltipText(FText::FromString(TEXT("Event when an actor enters the trigger area.")))
            .SetSparseDelegate(InObject, TEXT("OnActorBeginOverlap")));

        RegisteredEvents.Add(FActorIOEvent()
            .SetId(TEXT("ATriggerBase::OnTriggerExit"))
            .SetDisplayName(FText::FromString(TEXT("OnTriggerExit")))
            .SetTooltipText(FText::FromString(TEXT("Event when an actor leaves the trigger area.")))
            .SetSparseDelegate(InObject, TEXT("OnActorEndOverlap")));
    }
}

void UActorIOSystem::GetNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& RegisteredFunctions)
{
    if (InObject->IsA<ALight>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ALight::SetLightIntensity"))
            .SetDisplayName(FText::FromString(TEXT("SetLightIntensity")))
            .SetTooltipText(FText::FromString(TEXT("Set intensity of the light.")))
            .SetFunction(TEXT("SetIntensity"))
            .SetSubobject(TEXT("LightComponent0")));

        // #TODO: Test how this works
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ALight::SetLightColor"))
            .SetDisplayName(FText::FromString(TEXT("SetLightColor")))
            .SetTooltipText(FText::FromString(TEXT("Set color of the light.")))
            .SetFunction(TEXT("SetLightFColor"))
            .SetSubobject(TEXT("LightComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ALight::SetVisibility"))
            .SetDisplayName(FText::FromString(TEXT("SetVisibility")))
            .SetTooltipText(FText::FromString(TEXT("Set visibility of the light. Use this to turn light on/off.")))
            .SetFunction(TEXT("SetVisibility"))
            .SetSubobject(TEXT("LightComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ALight::SetCastShadows"))
            .SetDisplayName(FText::FromString(TEXT("SetCastShadows")))
            .SetTooltipText(FText::FromString(TEXT("Set light shadow casting on/off.")))
            .SetFunction(TEXT("SetCastShadows"))
            .SetSubobject(TEXT("LightComponent0")));
    }

    if (InObject->IsA<AEmitter>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AEmitter::Activate"))
            .SetDisplayName(FText::FromString(TEXT("Activate")))
            .SetTooltipText(FText::FromString(TEXT("Activate the particle system.")))
            .SetFunction(TEXT("Activate"))
            .SetSubobject(TEXT("ParticleSystemComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AEmitter::Deactivate"))
            .SetDisplayName(FText::FromString(TEXT("Deactivate")))
            .SetTooltipText(FText::FromString(TEXT("Deactivate the particle system.")))
            .SetFunction(TEXT("Deactivate"))
            .SetSubobject(TEXT("ParticleSystemComponent0")));
    }

    if (InObject->IsA<ANiagaraActor>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ANiagaraActor::Activate"))
            .SetDisplayName(FText::FromString(TEXT("Activate")))
            .SetTooltipText(FText::FromString(TEXT("Activate the particle system.")))
            .SetFunction(TEXT("Activate"))
            .SetSubobject(TEXT("NiagaraComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("ANiagaraActor::Deactivate"))
            .SetDisplayName(FText::FromString(TEXT("Deactivate")))
            .SetTooltipText(FText::FromString(TEXT("Deactivate the particle system.")))
            .SetFunction(TEXT("Deactivate"))
            .SetSubobject(TEXT("NiagaraComponent0")));
    }

    if (InObject->IsA<AAmbientSound>())
    {
        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::FadeIn"))
            .SetDisplayName(FText::FromString(TEXT("FadeIn")))
            .SetTooltipText(FText::FromString(TEXT("Smoothly start playing the sound with a fade.")))
            .SetFunction(TEXT("FadeIn"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::FadeOut"))
            .SetDisplayName(FText::FromString(TEXT("FadeOut")))
            .SetTooltipText(FText::FromString(TEXT("Smoothly stop playing the sound with a fade.")))
            .SetFunction(TEXT("FadeOut"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::AdjustVolume"))
            .SetDisplayName(FText::FromString(TEXT("AdjustVolume")))
            .SetTooltipText(FText::FromString(TEXT("Smoothly adjust the volume of the sound with a fade.")))
            .SetFunction(TEXT("AdjustVolume"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::Play"))
            .SetDisplayName(FText::FromString(TEXT("Play")))
            .SetTooltipText(FText::FromString(TEXT("Start playing the sound. Start time can be given.")))
            .SetFunction(TEXT("Play"))
            .SetSubobject(TEXT("AudioComponent0")));

        RegisteredFunctions.Add(FActorIOFunction()
            .SetId(TEXT("AAmbientSound::Stop"))
            .SetDisplayName(FText::FromString(TEXT("Stop")))
            .SetTooltipText(FText::FromString(TEXT("Stop playing the sound.")))
            .SetFunction(TEXT("Stop"))
            .SetSubobject(TEXT("AudioComponent0")));
    }

    // #TODO: Add support for volumes

    //RegisteredFunctions.Add(FActorIOFunction()
    //    .SetId(TEXT("AActor::SetActorHiddenInGame"))
    //    .SetDisplayName(FText::FromString(TEXT("SetActorHiddenInGame")))
    //    .SetTooltipText(FText::FromString(TEXT("Changes actor hidden state.")))
    //    .SetFunction(TEXT("SetActorHiddenInGame")));

    RegisteredFunctions.Add(FActorIOFunction()
        .SetId(TEXT("AActor::Destroy"))
        .SetDisplayName(FText::FromString(TEXT("Destroy")))
        .SetTooltipText(FText::FromString(TEXT("Destroy the actor.")))
        .SetFunction(TEXT("K2_DestroyActor")));
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
