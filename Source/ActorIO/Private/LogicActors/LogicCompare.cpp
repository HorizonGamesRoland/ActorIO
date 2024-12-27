// Copyright 2024 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicCompare.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicCompare::ALogicCompare()
{
	InitialValue = FString();
	CompareValue = FString();
	CurrentValue = FString();
}

void ALogicCompare::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		// Do nothing in the editor.
		return;
	}

	CurrentValue = InitialValue;
}

void ALogicCompare::RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnEquals"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnEquals", "OnEquals"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnEqualsTooltip", "Event when the current value equals the compare value."))
		.SetMulticastDelegate(this, &OnEquals));

	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnNotEquals"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnNotEquals", "OnNotEquals"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnNotEqualsTooltip", "Event when the current value does not equal the compare value."))
		.SetMulticastDelegate(this, &OnNotEquals));

	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnLessThen"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnLessThen", "OnLessThen"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnLessThenTooltip", "Event when the current value is less than the compare value. Only works with numeric values!"))
		.SetMulticastDelegate(this, &OnLessThen));

	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnGreaterThen"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnGreaterThen", "OnGreaterThen"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnGreaterThenTooltip", "Event when the current value is greater than the compare value. Only works with numeric values!"))
		.SetMulticastDelegate(this, &OnGreaterThen));
}

void ALogicCompare::RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::SetValue"))
		.SetDisplayName(LOCTEXT("ALogicCompare.SetValue", "SetValue"))
		.SetTooltipText(LOCTEXT("ALogicCompare.SetValueTooltip", "Set the current value without performing the comparison."))
		.SetFunction(TEXT("SetValue")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::SetValueAndCompare"))
		.SetDisplayName(LOCTEXT("ALogicCompare.SetValueAndCompare", "SetValueAndCompare"))
		.SetTooltipText(LOCTEXT("ALogicCompare.SetValueAndCompareTooltip", "Set the current value and compare it against the compare value."))
		.SetFunction(TEXT("SetValueAndCompare")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::SetCompareValue"))
		.SetDisplayName(LOCTEXT("ALogicCompare.SetCompareValue", "SetCompareValue"))
		.SetTooltipText(LOCTEXT("ALogicCompare.SetCompareValueTooltip", "Set the compare value."))
		.SetFunction(TEXT("SetCompareValue")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::Compare"))
		.SetDisplayName(LOCTEXT("ALogicCompare.Compare", "Compare"))
		.SetTooltipText(LOCTEXT("ALogicCompare.CompareTooltip", "Compare the current value with the compare value."))
		.SetFunction(TEXT("Compare")));
}

void ALogicCompare::SetValue(FString InValue)
{
	CurrentValue = InValue;
}

void ALogicCompare::SetValueAndCompare(FString InValue)
{
	CurrentValue = InValue;
	Compare();
}

void ALogicCompare::SetCompareValue(FString InValue)
{
	CompareValue = InValue;
}

void ALogicCompare::Compare()
{
	if (CurrentValue == CompareValue)
	{
		OnEquals.Broadcast();
	}
	else
	{
		OnNotEquals.Broadcast();
	}

	if (CurrentValue.IsNumeric() && CompareValue.IsNumeric())
	{
		const float NumericCurrentValue = FCString::Atof(*CurrentValue);
		const float NumericCompareValue = FCString::Atof(*CurrentValue);

		if (NumericCurrentValue < NumericCompareValue)
		{
			OnLessThen.Broadcast();
		}
		else if (NumericCurrentValue > NumericCompareValue)
		{
			OnGreaterThen.Broadcast();
		}
	}
}

#undef LOCTEXT_NAMESPACE