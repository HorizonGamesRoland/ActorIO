// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicBranch.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicBranch::ALogicBranch()
{
	bInitialValue = false;
	bCurrentValue = false;

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_Branch"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicBranch::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicBranch::OnTrue"))
		.SetDisplayName(LOCTEXT("LogicBranch.OnTrue", "OnTrue"))
		.SetTooltipText(LOCTEXT("LogicBranch.OnTrueTooltip", "Event when the stored boolean value is true when 'Test' is called."))
		.SetMulticastDelegate(this, &OnTrue));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicBranch::OnFalse"))
		.SetDisplayName(LOCTEXT("LogicBranch.OnFalse", "OnFalse"))
		.SetTooltipText(LOCTEXT("LogicBranch.OnFalseTooltip", "Event when the stored boolean value is false when 'Test' is called."))
		.SetMulticastDelegate(this, &OnFalse));
}

void ALogicBranch::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::SetValue"))
		.SetDisplayName(LOCTEXT("LogicBranch.SetValue", "SetValue"))
		.SetTooltipText(LOCTEXT("LogicBranch.SetValueTooltip", "Set the boolean value without performing the comparison. Use this to hold a value for a future test."))
		.SetFunction(TEXT("SetValue")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::SetValueAndTest"))
		.SetDisplayName(LOCTEXT("LogicBranch.SetValueAndTest", "SetValueAndTest"))
		.SetTooltipText(LOCTEXT("LogicBranch.SetValueAndTestTooltip", "Set the boolean value and test it, firing 'OnTrue' or 'OnFalse' based on the new value."))
		.SetFunction(TEXT("SetValueAndTest")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::Toggle"))
		.SetDisplayName(LOCTEXT("LogicBranch.Toggle", "Toggle"))
		.SetTooltipText(LOCTEXT("LogicBranch.ToggleTooltip", "Toggle the boolean value between true and false without performing the comparison."))
		.SetFunction(TEXT("Toggle")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::ToggleAndTest"))
		.SetDisplayName(LOCTEXT("LogicBranch.ToggleAndTest", "ToggleAndTest"))
		.SetTooltipText(LOCTEXT("LogicBranch.ToggleAndTestTooltip", "Toggle the boolean value and test it, firing 'OnTrue' or 'OnFalse' based on the new value."))
		.SetFunction(TEXT("ToggleAndTest")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::Test"))
		.SetDisplayName(LOCTEXT("LogicBranch.Test", "Test"))
		.SetTooltipText(LOCTEXT("LogicBranch.TestTooltip", "Test the boolean value and fire 'OnTrue' or 'OnFalse' based on the value."))
		.SetFunction(TEXT("Test")));
}

void ALogicBranch::GetLocalNamedArguments(FActionExecutionContext& ExecutionContext)
{
	ExecutionContext.SetNamedArgument(TEXT("$Value"), bCurrentValue ? TEXT("True") : TEXT("False"));
}

void ALogicBranch::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
		bCurrentValue = bInitialValue;
	}
}

void ALogicBranch::SetValue(bool bValue)
{
	bCurrentValue = bValue;
}

void ALogicBranch::SetValueAndTest(bool bValue)
{
	bCurrentValue = bValue;
	Test();
}

void ALogicBranch::Toggle()
{
	bCurrentValue = !bCurrentValue;
}

void ALogicBranch::ToggleAndTest()
{
	bCurrentValue = !bCurrentValue;
	Test();
}

void ALogicBranch::Test()
{
	if (bCurrentValue)
	{
		OnTrue.Broadcast();
	}
	else
	{
		OnFalse.Broadcast();
	}
}

#undef LOCTEXT_NAMESPACE
