// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#include "LogicActors/LogicCase.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicCase::ALogicCase()
{
	Cases = TArray<FString>();
	Cases.Init(FString(), 4);

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_Case"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicCase::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase01"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase01", "OnCase01"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase01Tooltip", "Event when the input value equals the case value at index 0."))
		.SetMulticastDelegate(this, &OnCase01));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase02"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase02", "OnCase02"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase02Tooltip", "Event when the input value equals the case value at index 1."))
		.SetMulticastDelegate(this, &OnCase02));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase03"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase03", "OnCase03"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase03Tooltip", "Event when the input value equals the case value at index 2."))
		.SetMulticastDelegate(this, &OnCase03));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase04"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase04", "OnCase04"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase04Tooltip", "Event when the input value equals the case value at index 3."))
		.SetMulticastDelegate(this, &OnCase04));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase05"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase05", "OnCase05"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase05Tooltip", "Event when the input value equals the case value at index 4."))
		.SetMulticastDelegate(this, &OnCase05));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase06"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase06", "OnCase06"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase06Tooltip", "Event when the input value equals the case value at index 5."))
		.SetMulticastDelegate(this, &OnCase06));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase07"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase07", "OnCase07"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase07Tooltip", "Event when the input value equals the case value at index 6."))
		.SetMulticastDelegate(this, &OnCase07));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase08"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase08", "OnCase08"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase08Tooltip", "Event when the input value equals the case value at index 7."))
		.SetMulticastDelegate(this, &OnCase08));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase09"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase09", "OnCase09"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase09Tooltip", "Event when the input value equals the case value at index 8."))
		.SetMulticastDelegate(this, &OnCase09));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase10"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase10", "OnCase10"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase10Tooltip", "Event when the input value equals the case value at index 9."))
		.SetMulticastDelegate(this, &OnCase10));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase11"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase11", "OnCase11"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase11Tooltip", "Event when the input value equals the case value at index 10."))
		.SetMulticastDelegate(this, &OnCase11));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase12"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase12", "OnCase12"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase12Tooltip", "Event when the input value equals the case value at index 11."))
		.SetMulticastDelegate(this, &OnCase12));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase13"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase13", "OnCase13"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase13Tooltip", "Event when the input value equals the case value at index 12."))
		.SetMulticastDelegate(this, &OnCase13));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase14"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase14", "OnCase14"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase14Tooltip", "Event when the input value equals the case value at index 13."))
		.SetMulticastDelegate(this, &OnCase14));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase15"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase15", "OnCase15"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase15Tooltip", "Event when the input value equals the case value at index 14."))
		.SetMulticastDelegate(this, &OnCase15));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCase::OnCase16"))
		.SetDisplayName(LOCTEXT("LogicCase.OnCase16", "OnCase16"))
		.SetTooltipText(LOCTEXT("LogicCase.OnCase16Tooltip", "Event when the input value equals the case value at index 15."))
		.SetMulticastDelegate(this, &OnCase16));
}

void ALogicCase::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCase::Test"))
		.SetDisplayName(LOCTEXT("LogicCase.Test", "Test"))
		.SetTooltipText(LOCTEXT("LogicCase.TestTooltip", "Compare the value to the case values, and fire the appropriate 'OnCase' event."))
		.SetFunction(TEXT("Test")));
}

void ALogicCase::Test(FString InValue)
{
	int32 MatchingCaseIdx = INDEX_NONE;
	if (!InValue.IsEmpty())
	{
		for (int32 Idx = 0; Idx != Cases.Num(); ++Idx)
		{
			if (InValue == Cases[Idx] && Idx < 16)
			{
				MatchingCaseIdx = Idx;
				break;
			}
		}
	}

	switch (MatchingCaseIdx)
	{
		case 0:
			OnCase01.Broadcast();
			break;
		case 1:
			OnCase02.Broadcast();
			break;
		case 2:
			OnCase03.Broadcast();
			break;
		case 3:
			OnCase04.Broadcast();
			break;
		case 4:
			OnCase05.Broadcast();
			break;
		case 5:
			OnCase06.Broadcast();
			break;
		case 6:
			OnCase07.Broadcast();
			break;
		case 7:
			OnCase08.Broadcast();
			break;
		case 8:
			OnCase09.Broadcast();
			break;
		case 9:
			OnCase10.Broadcast();
			break;
		case 10:
			OnCase11.Broadcast();
			break;
		case 11:
			OnCase12.Broadcast();
			break;
		case 12:
			OnCase13.Broadcast();
			break;
		case 13:
			OnCase14.Broadcast();
			break;
		case 14:
			OnCase15.Broadcast();
			break;
		case 15:
			OnCase16.Broadcast();
			break;
		default:
			OnDefault.Broadcast();
			break;
	}
}

#undef LOCTEXT_NAMESPACE
