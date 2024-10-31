// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Delegates/Delegate.h"
#include "ActorIOEvent.generated.h"

USTRUCT()
struct ACTORIO_API FActorIOEvent
{
	GENERATED_BODY()

	FName EventId;

	FText DisplayName;

	FText TooltipText;

	FMulticastScriptDelegate* MulticastDelegateRef;

	FName SparseDelegateName;

	FActorIOEvent() :
		EventId(NAME_None),
		DisplayName(FText::GetEmpty()),
		TooltipText(FText::GetEmpty()),
		MulticastDelegateRef(nullptr),
		SparseDelegateName(NAME_None)
	{}

	FActorIOEvent& SetId(FName InEventId)
	{
		EventId = InEventId;
		return *this;
	}

	FActorIOEvent& SetDisplayName(const FText& InDisplayName)
	{
		DisplayName = InDisplayName;
		return *this;
	}

	FActorIOEvent& SetTooltipText(const FText& InTooltipText)
	{
		TooltipText = InTooltipText;
		return *this;
	}

	FActorIOEvent& SetMulticastDelegate(FMulticastScriptDelegate* InDelegate)
	{
		MulticastDelegateRef = InDelegate;
		return *this;
	}

	FActorIOEvent& SetSparseDelegateName(FName InDelegateName)
	{
		SparseDelegateName = InDelegateName;
		return *this;
	}

	bool operator==(const FName InEventName) const
	{
		return EventId == InEventName;
	}
};




UENUM()
enum class EActorIONativeEvents : uint32
{
	ActorBeginOverlap,

	ActorEndOverlap
};

static FName ToName(EActorIONativeEvents InEnumValue)
{
	return UEnum::GetValueAsName(InEnumValue);
}
