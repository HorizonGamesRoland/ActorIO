// Copyright 2025 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicCounter.h"
#include "ActorIOSystem.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicCounter::ALogicCounter()
{
	InitialValue = 0;
	TargetValue = 0;
	bClampValue = false;
	CurrentValue = 0;

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/S_Counter"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicCounter::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCounter::OnValueChanged"))
		.SetDisplayName(LOCTEXT("ALogicCounter.OnValueChanged", "OnValueChanged"))
		.SetTooltipText(LOCTEXT("ALogicCounter.OnValueChangedTooltip", "Event when the current value is changed."))
		.SetMulticastDelegate(this, &OnValueChanged));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCounter::OnTargetValueReached"))
		.SetDisplayName(LOCTEXT("ALogicCounter.OnTargetValueReached", "OnTargetValueReached"))
		.SetTooltipText(LOCTEXT("ALogicCounter.OnTargetValueReachedeTooltip", "Event when the current value equals or greater then the target value."))
		.SetMulticastDelegate(this, &OnTargetValueReached));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCounter::OnGetValue"))
		.SetDisplayName(LOCTEXT("ALogicCounter.OnGetValue", "OnGetValue"))
		.SetTooltipText(LOCTEXT("ALogicCounter.OnGetValueTooltip", "Event when the current value is read using the 'GetValue' function."))
		.SetMulticastDelegate(this, &OnGetValue)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnGetValue")));
}

void ALogicCounter::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCounter::Add"))
		.SetDisplayName(LOCTEXT("ALogicCounter.Add", "Add"))
		.SetTooltipText(LOCTEXT("ALogicCounter.AddTooltip", "Add to the current value."))
		.SetFunction(TEXT("Add")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCounter::Subtract"))
		.SetDisplayName(LOCTEXT("ALogicCounter.Subtract", "Subtract"))
		.SetTooltipText(LOCTEXT("ALogicCounter.SubtractTooltip", "Subtract from the current value."))
		.SetFunction(TEXT("Subtract")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCounter::SetValue"))
		.SetDisplayName(LOCTEXT("ALogicCounter.SetValue", "SetValue"))
		.SetTooltipText(LOCTEXT("ALogicCounter.SetValueTooltip", "Set the current value directly."))
		.SetFunction(TEXT("SetValue")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCounter::SetTargetValue"))
		.SetDisplayName(LOCTEXT("ALogicCounter.SetTargetValue", "SetTargetValue"))
		.SetTooltipText(LOCTEXT("ALogicCounter.SetTargetValueTooltip", "Set a new target value. Fires 'OnTargetValueReached' if current value equals or greater then the new target value."))
		.SetFunction(TEXT("SetTargetValue")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCounter::GetValue"))
		.SetDisplayName(LOCTEXT("ALogicCounter.GetValue", "GetValue"))
		.SetTooltipText(LOCTEXT("ALogicCounter.GetValueTooltip", "Fire the 'OnGetValue' event with the current value."))
		.SetFunction(TEXT("GetValue")));
}

void ALogicCounter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
		CurrentValue = InitialValue;
		if (bClampValue)
		{
			CurrentValue = FMath::Clamp(CurrentValue, 0, TargetValue);
		}
	}
}

void ALogicCounter::Add(int32 Amount)
{
	if (Amount == 0)
	{
		Amount = 1;
	}
	
	CurrentValue += Amount;

	if (bClampValue)
	{
		CurrentValue = FMath::Clamp(CurrentValue, 0, TargetValue);
	}

	if (CurrentValue >= TargetValue)
	{
		OnTargetValueReached.Broadcast(CurrentValue);
	}
}

void ALogicCounter::Subtract(int32 Amount)
{
	if (Amount == 0)
	{
		Amount = 1;
	}

	CurrentValue -= Amount;

	if (bClampValue)
	{
		CurrentValue = FMath::Clamp(CurrentValue, 0, TargetValue);
	}

	if (CurrentValue >= TargetValue)
	{
		OnTargetValueReached.Broadcast(CurrentValue);
	}
}

void ALogicCounter::SetValue(int32 Value)
{
	CurrentValue = Value;

	if (bClampValue)
	{
		CurrentValue = FMath::Clamp(CurrentValue, 0, TargetValue);
	}

	if (CurrentValue >= TargetValue)
	{
		OnTargetValueReached.Broadcast(CurrentValue);
	}
}

void ALogicCounter::SetTargetValue(int32 Value)
{
	TargetValue = Value;

	if (bClampValue)
	{
		CurrentValue = FMath::Clamp(CurrentValue, 0, TargetValue);
	}

	if (CurrentValue >= TargetValue)
	{
		OnTargetValueReached.Broadcast(CurrentValue);
	}
}

int32 ALogicCounter::GetValue() const
{
	OnGetValue.Broadcast(CurrentValue);
	return CurrentValue;
}

void ALogicCounter::ProcessEvent_OnGetValue(int32 Value)
{
	FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
	ExecContext.SetNamedArgument(TEXT("$Value"), FString::FromInt(Value));
}

#undef LOCTEXT_NAMESPACE