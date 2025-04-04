// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicCompare.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicCompare::ALogicCompare()
{
	InitialValue = FString();
	CompareValue = FString();
	CurrentValue = FString();

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_Compare"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicCompare::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnEquals"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnEquals", "OnEquals"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnEqualsTooltip", "Event when the current value equals the compare value."))
		.SetMulticastDelegate(this, &OnEquals)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnCompare")));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnNotEquals"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnNotEquals", "OnNotEquals"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnNotEqualsTooltip", "Event when the current value does not equal the compare value."))
		.SetMulticastDelegate(this, &OnNotEquals)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnCompare")));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnLessThen"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnLessThen", "OnLessThen"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnLessThenTooltip", "Event when the current value is less than the compare value. Only works with numeric values!"))
		.SetMulticastDelegate(this, &OnLessThen)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnCompare")));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCompare::OnGreaterThen"))
		.SetDisplayName(LOCTEXT("ALogicCompare.OnGreaterThen", "OnGreaterThen"))
		.SetTooltipText(LOCTEXT("ALogicCompare.OnGreaterThenTooltip", "Event when the current value is greater than the compare value. Only works with numeric values!"))
		.SetMulticastDelegate(this, &OnGreaterThen)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnCompare")));
}

void ALogicCompare::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::SetValue"))
		.SetDisplayName(LOCTEXT("ALogicCompare.SetValue", "SetValue"))
		.SetTooltipText(LOCTEXT("ALogicCompare.SetValueTooltip", "Set the current value without performing the comparison."))
		.SetFunction(TEXT("SetValue")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::SetValueAndCompare"))
		.SetDisplayName(LOCTEXT("ALogicCompare.SetValueAndCompare", "SetValueAndCompare"))
		.SetTooltipText(LOCTEXT("ALogicCompare.SetValueAndCompareTooltip", "Set the current value and compare it against the compare value."))
		.SetFunction(TEXT("SetValueAndCompare")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::SetCompareValue"))
		.SetDisplayName(LOCTEXT("ALogicCompare.SetCompareValue", "SetCompareValue"))
		.SetTooltipText(LOCTEXT("ALogicCompare.SetCompareValueTooltip", "Set the compare value."))
		.SetFunction(TEXT("SetCompareValue")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCompare::Compare"))
		.SetDisplayName(LOCTEXT("ALogicCompare.Compare", "Compare"))
		.SetTooltipText(LOCTEXT("ALogicCompare.CompareTooltip", "Compare the current value with the compare value."))
		.SetFunction(TEXT("Compare")));
}

void ALogicCompare::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
		CurrentValue = InitialValue;
	}
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
		const float NumericCompareValue = FCString::Atof(*CompareValue);

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

void ALogicCompare::ProcessEvent_OnCompare()
{
	FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
	ExecContext.SetNamedArgument(TEXT("$Value"), CurrentValue);
}

#undef LOCTEXT_NAMESPACE