// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOSubsystemBase.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOAction.h"
#include "ActorIOSettings.h"
#include "LogicActors/LogicActorBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/BlockingVolume.h"
#include "Engine/TriggerVolume.h"
#include "Engine/TriggerBase.h"
#include "Engine/Light.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/Emitter.h"
#include "Sound/AmbientSound.h"
#include "Sound/AudioVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 7, 0)
#include "Misc/StringOutputDevice.h"
#endif

#define LOCTEXT_NAMESPACE "ActorIO"

UActorIOSubsystemBase::UActorIOSubsystemBase()
{
    PendingMessages = TArray<FActorIOMessage>();
    ActionExecContext = FActionExecutionContext();
}

UActorIOSubsystemBase* UActorIOSubsystemBase::Get(UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
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

void UActorIOSubsystemBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (int32 MessageIdx = PendingMessages.Num() - 1; MessageIdx >= 0; --MessageIdx)
    {
        FActorIOMessage& Message = PendingMessages[MessageIdx];
        Message.TimeRemaining -= DeltaTime;

        if (Message.TimeRemaining <= 0.0f)
        {
            ProcessMessage(Message);
            PendingMessages.RemoveAt(MessageIdx);
        }
    }
}

bool UActorIOSubsystemBase::IsTickable() const
{
    return PendingMessages.Num() > 0;
}

TStatId UActorIOSubsystemBase::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UActorIOSubsystemBase, STATGROUP_Tickables);
}

void UActorIOSubsystemBase::QueueMessage(FActorIOMessage& InMessage)
{
    if (InMessage.TimeRemaining <= 0.0f)
    {
        ProcessMessage(InMessage);
    }
    else
    {
        PendingMessages.Add(InMessage);
    }
}

void UActorIOSubsystemBase::ProcessMessage(FActorIOMessage& InMessage)
{
    AActor* ActorPtr = InMessage.TargetPtr.Get();

    FString ErrorReason;
    if (!IActorIO::ConfirmObjectIsAlive(ActorPtr, ErrorReason))
    {
        IActorIO::ExecutionError(DebugIOActions, ELogVerbosity::Warning, FString::Printf(TEXT("Target was invalid when executing I/O function '%s'. Reason: %s"), *InMessage.FunctionId.ToString(), *ErrorReason));
        return;
    }

    FActorIOFunctionList ValidFunctions = IActorIO::GetFunctionsForObject(ActorPtr);
    FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(InMessage.FunctionId);
    if (!TargetFunction)
    {
        IActorIO::ExecutionError(DebugIOActions, ELogVerbosity::Error, FString::Printf(TEXT("Could not find I/O function '%s' on target actor '%s'."), *InMessage.FunctionId.ToString(), *ActorPtr->GetActorNameOrLabel()));
        return;
    }

    if (TargetFunction->FunctionToExec.IsEmpty())
    {
        IActorIO::ExecutionError(DebugIOActions, ELogVerbosity::Error, FString::Printf(TEXT("I/O function '%s' points to an empty func name."), *InMessage.FunctionId.ToString()));
        return;
    }

    // Figure out which object the final command should be sent to.
    // In most cases this will be the target actor itself.
    // However, the I/O function may want it to be executed on a subobject of the actor instead.
    UObject* TargetObject = ActorPtr;
    if (!TargetFunction->TargetSubobject.IsNone())
    {
        TargetObject = ActorPtr->GetDefaultSubobjectByName(TargetFunction->TargetSubobject);
        if (!TargetObject)
        {
            IActorIO::ExecutionError(DebugIOActions, ELogVerbosity::Error, FString::Printf(TEXT("I/O function '%s' target subobject '%s' not found on actor '%s'."), *InMessage.FunctionId.ToString(), *TargetFunction->TargetSubobject.ToString(), *ActorPtr->GetActorNameOrLabel()));
            return;
        }
    }

    // Get the quoted name of the function to call on the target actor.
    // Quotes are needed to support function names with whitespaces.
    FString FunctionName = TargetFunction->FunctionToExec;
    if (FunctionName.Len() > 0)
    {
        if (FunctionName[0] != '"')
        {
            FunctionName.InsertAt(0, '"');
        }

        const int32 LastCharIndex = FunctionName.Len() - 1;
        if (FunctionName[LastCharIndex] != '"')
        {
            FunctionName.AppendChar('"');
        }
    }

    // Build the final command that is executed on the target actor.
    // Format is: FunctionName Arg1 Arg2 Arg3 (...)
    FString Command = FunctionName + InMessage.Arguments;

    UE_CLOG(LogIOFinalCommand, LogActorIO, Log, TEXT("Executing command: %s (Target: %s)"), *Command, *TargetObject->GetName());

    FStringOutputDevice Ar;
    ExecuteCommand(TargetObject, *Command, Ar, this);

    // Log execution errors.
    if (!Ar.IsEmpty())
    {
        IActorIO::ExecutionError(DebugIOActions, ELogVerbosity::Error, Ar);
    }
}

bool UActorIOSubsystemBase::ExecuteCommand(UObject* Target, const TCHAR* Str, FOutputDevice& Ar, UObject* Executor)
{
    /**
     * THIS IS A MODIFIED VERSION OF UObject::CallFunctionByNameWithString
     *
     * Always review the original function when a new engine version is released and update this code if needed.
     * If everything is working update the UE version comparison below. Current value represents the latest reviewed engine version.
     *
     * List of changes:
     *
     *   - Skip importing value for 'out' properties that are not passed by 'ref'.
     *   - Skip CPP param default value initialization because it only works in editor and not packaged games.
     *   - FindFunction and ProcessEvent are called on Target.
     *   - Add IsValid check for Target before finding UFunction.
     *   - Use Ar.Logf instead of UE_LOG(LogScriptCore) because LogScriptCore is static and its verbosity cannot be changed.
     *   - Return success/failure properly.
     */

#if UE_VERSION_NEWER_THAN(5, 7, 999) // <- patch version doesn't matter so use 999 to pass the check
#error "Review latest implementation of UObject::CallFunctionByNameWithString then update UE version comparison."
#endif

    // Find an exec function.
    FString MsgStr;
    if (!FParse::Token(Str, MsgStr, true))
    {
        Ar.Logf(TEXT("ExecuteCommand: Not Parsed '%s'"), Str);
        return false;
    }
    const FName Message = FName(*MsgStr, FNAME_Find);
    if (Message == NAME_None)
    {
        Ar.Logf(TEXT("ExecuteCommand: Name not found '%s'"), Str);
        return false;
    }
    if (!IsValid(Target))
    {
        Ar.Logf(TEXT("ExecuteCommand: Target not found"));
        return false;
    }
    UFunction* Function = Target->FindFunction(Message);
    if (nullptr == Function)
    {
        Ar.Logf(TEXT("ExecuteCommand: Function not found '%s'"), Str);
        return false;
    }

    FProperty* LastParameter = nullptr;

    // find the last parameter
    for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
    {
        LastParameter = *It;
    }

    // Parse all function parameters.
    uint8* Parms = (uint8*)FMemory_Alloca_Aligned(Function->ParmsSize, Function->GetMinAlignment());
    FMemory::Memzero(Parms, Function->ParmsSize);

    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        FProperty* LocalProp = *It;
        checkSlow(LocalProp);
        if (!LocalProp->HasAnyPropertyFlags(CPF_ZeroConstructor))
        {
            LocalProp->InitializeValue_InContainer(Parms);
        }
    }

    const uint32 ExportFlags = PPF_None;
    bool bFailed = 0;
    int32 NumParamsEvaluated = 0;
    for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It, NumParamsEvaluated++)
    {
        FProperty* PropertyParam = *It;
        checkSlow(PropertyParam); // Fix static analysis warning
        if (NumParamsEvaluated == 0 && Executor)
        {
            FObjectPropertyBase* Op = CastField<FObjectPropertyBase>(*It);
            if (Op && Executor->IsA(Op->PropertyClass))
            {
                // First parameter is implicit reference to object executing the command.
                Op->SetObjectPropertyValue(Op->ContainerPtrToValuePtr<uint8>(Parms), Executor);
                continue;
            }
        }

        /*
         * SKIP IMPORTING VALUE FOR 'OUT' PROPERTIES THAT ARE NOT PASSED BY 'REF'
         * 
         * In Unreal's reflection system, out properties and reference properties are differentiated.
         * Out params that are NOT passed by ref only appear on return nodes.
         * They do not have an input value, instead they are initialized to zero/null and the function itself will give it a value when returning it.
         * However, if the property is passed by ref then it will also appear as an input, and we will have to initialize it ourselves.
         * 
         * In C++ this differentiation translates to:
         *  - "FString& OutString" <- an out param that is not passed by ref and will only appear on return nodes (even though the value is passed by ref in C++).
         *  - "UPARAM(Ref) FString& OutString" <- an out param that is passed by ref, and will behave the same as regular C++ code.
         * 
         * So to properly support out params, we need to skip importing values for these params if they are NOT passed by ref.
         * At this point we've already initialized a value for these params above.
         */
        if (PropertyParam->HasAnyPropertyFlags(CPF_OutParm) && !PropertyParam->HasAnyPropertyFlags(CPF_ReferenceParm))
        {
            continue;
        }

        // Keep old string around in case we need to pass the whole remaining string
        const TCHAR* RemainingStr = Str;

        // Parse a new argument out of Str
        FString ArgStr;
        FParse::Token(Str, ArgStr, true);

        // if ArgStr is empty but we have more params to read parse the function to see if these have defaults, if so set them
        bool bFoundDefault = false;
        bool bFailedImport = true;

        /*
         * SKIP INITIALIZE CPP FUNCTION PARAM DEFAULT VALUE
         *
         * This only works in the editor because values are being read from Function->GetMetaData (which is editor only).
         * We need to skip this because it would lead to different outcomes in editor vs packaged game.
         * I'm not sure how blueprint nodes do this.. I suspect they read this value at edit time and store it for execution.
         * In theory we could cache these values at cook time for use at runtime (?)
         *
#if WITH_EDITOR
        if (!FCString::Strcmp(*ArgStr, TEXT("")))
        {
            const FName DefaultPropertyKey(*(FString(TEXT("CPP_Default_")) + PropertyParam->GetName()));
            const FString& PropertyDefaultValue = Function->GetMetaData(DefaultPropertyKey);
            if (!PropertyDefaultValue.IsEmpty())
            {
                bFoundDefault = true;

                const TCHAR* Result = It->ImportText_InContainer(*PropertyDefaultValue, Parms, nullptr, ExportFlags);
                bFailedImport = (Result == nullptr);
            }
        }
#endif
        */

        if (!bFoundDefault)
        {
            // if this is the last string property and we have remaining arguments to process, we have to assume that this
            // is a sub-command that will be passed to another exec (like "cheat giveall weapons", for example). Therefore
            // we need to use the whole remaining string as an argument, regardless of quotes, spaces etc.
            if (PropertyParam == LastParameter && PropertyParam->IsA<FStrProperty>() && FCString::Strcmp(Str, TEXT("")) != 0)
            {
                ArgStr = FString(RemainingStr).TrimStart();
            }

            const TCHAR* Result = It->ImportText_InContainer(*ArgStr, Parms, nullptr, ExportFlags);
            bFailedImport = (Result == nullptr);
        }

        if (bFailedImport)
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Message"), FText::FromName(Message));
            Arguments.Add(TEXT("PropertyName"), FText::FromName(It->GetFName()));
            Arguments.Add(TEXT("FunctionName"), FText::FromName(Function->GetFName()));
            Ar.Logf(TEXT("%s"), *FText::Format(NSLOCTEXT("Core", "BadProperty", "'{Message}': Bad or missing property '{PropertyName}' when trying to call {FunctionName}"), Arguments).ToString());
            bFailed = true;

            break;
        }
    }

    if (!bFailed)
    {
        Target->ProcessEvent(Function, Parms);
    }

    //!!destructframe see also UObject::ProcessEvent
    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        It->DestroyValue_InContainer(Parms);
    }

    return !bFailed;
}

void UActorIOSubsystemBase::RegisterNativeEventsForObject(AActor* InObject, FActorIOEventList& EventRegistry)
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
    // Sequence Actors
    //==================================

    // In the case of ALevelSequenceActor, we are comparing the class name directly to avoid dependency to LevelSequence module.
    // Note that this does not support class inheritance, so it only works for exact classes.
    // Since LevelSequenceActor is just a component wrapper, I do not think anyone will ever subclass it anyways.
    if (InObject->GetClass()->GetFName() == TEXT("LevelSequenceActor") || InObject->GetClass()->GetFName() == TEXT("ReplicatedLevelSequenceActor"))
    {
        // The callback events are in the ULevelSequencePlayer subobject of the ALevelSequenceActor.
        // We can simply get it as a UObject and use reflection data to bind to it (via SetBlueprintDelegate).
        UObject* LevelSequencePlayer = InObject->GetDefaultSubobjectByName(TEXT("AnimationPlayer"));

        EventRegistry.RegisterEvent(FActorIOEvent()
            .SetId(TEXT("ALevelSequenceActor::OnPlay"))
            .SetDisplayName(LOCTEXT("LevelSequenceActor.OnPlay", "OnPlay"))
            .SetTooltipText(LOCTEXT("LevelSequenceActor.OnPlayTooltip", "Event when the level sequence is started."))
            .SetBlueprintDelegate(LevelSequencePlayer, TEXT("OnPlay")));

        EventRegistry.RegisterEvent(FActorIOEvent()
            .SetId(TEXT("ALevelSequenceActor::OnStop"))
            .SetDisplayName(LOCTEXT("LevelSequenceActor.OnStop", "OnStop"))
            .SetTooltipText(LOCTEXT("LevelSequenceActor.OnStopTooltip", "Event when the level sequence is stopped."))
            .SetBlueprintDelegate(LevelSequencePlayer, TEXT("OnStop")));

        EventRegistry.RegisterEvent(FActorIOEvent()
            .SetId(TEXT("ALevelSequenceActor::OnFinished"))
            .SetDisplayName(LOCTEXT("LevelSequenceActor.OnFinished", "OnFinished"))
            .SetTooltipText(LOCTEXT("LevelSequenceActor.OnFinishedTooltip", "Event when the level sequence finishes naturally (without explicitly calling stop)."))
            .SetBlueprintDelegate(LevelSequencePlayer, TEXT("OnFinished")));
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
            .SetSparseDelegate(InObject, TEXT("OnEndPlay"))
            .SetEventProcessor(this, TEXT("ProcessEvent_OnActorDestroyed")));
    }
}

void UActorIOSubsystemBase::RegisterNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& FunctionRegistry)
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

    // In the case of ANiagaraActor, we are comparing the class name directly to avoid dependency to Niagara module.
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
    // Text Render Actors
    //==================================

    if (InObject->IsA<ATextRenderActor>())
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ATextRenderActor::SetText"))
            .SetDisplayName(LOCTEXT("TextRenderActor.SetText", "SetText"))
            .SetTooltipText(LOCTEXT("TextRenderActor.SetTextTooltip", "Change the displayed text."))
            .SetFunction(TEXT("K2_SetText"))
            .SetSubobject(TEXT("NewTextRenderComponent")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ATextRenderActor::SetTextRenderColor"))
            .SetDisplayName(LOCTEXT("TextRenderActor.SetTextRenderColor", "SetTextRenderColor"))
            .SetTooltipText(LOCTEXT("TextRenderActor.SetTextRenderColorTooltip", "Set color of the text."))
            .SetFunction(TEXT("SetTextRenderColor"))
            .SetSubobject(TEXT("NewTextRenderComponent")));
    }

    //==================================
    // Sequence Actors
    //==================================

    // In the case of ALevelSequenceActor, we are comparing the class name directly to avoid dependency to LevelSequence module.
    // Note that this does not support class inheritance, so it only works for exact classes.
    // Since LevelSequenceActor is just a component wrapper, I do not think anyone will ever subclass it anyways.
    if (InObject->GetClass()->GetFName() == TEXT("LevelSequenceActor"))
    {
        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ALevelSequenceActor::Play"))
            .SetDisplayName(LOCTEXT("LevelSequenceActor.Play", "Play"))
            .SetTooltipText(LOCTEXT("LevelSequenceActor.PlayTooltip", "Start playing the sequence."))
            .SetFunction(TEXT("Play"))
            .SetSubobject(TEXT("AnimationPlayer")));

        FunctionRegistry.RegisterFunction(FActorIOFunction()
            .SetId(TEXT("ALevelSequenceActor::Stop"))
            .SetDisplayName(LOCTEXT("LevelSequenceActor.Stop", "Stop"))
            .SetTooltipText(LOCTEXT("LevelSequenceActor.StopTooltip", "Go to end of the sequence and stop. Adheres to 'When Finished' section rules."))
            .SetFunction(TEXT("GoToEndAndStop"))
            .SetSubobject(TEXT("AnimationPlayer")));
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

void UActorIOSubsystemBase::ProcessEvent_OnActorDestroyed(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
    ActionExecContext.SetNamedArgument(TEXT("$Actor"), IsValid(Actor) ? Actor->GetPathName() : FString());
    if (EndPlayReason != EEndPlayReason::Destroyed)
    {
        // Abort the action if end play was not caused by destroying the actor.
        ActionExecContext.AbortAction();
    }
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
