// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Delegates/Delegate.h"
#include "UObject/SparseDelegate.h"
#include "ActorIOEvent.generated.h"

USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FText TooltipText;

	UPROPERTY()
	TObjectPtr<UObject> DelegateOwner;

	FMulticastScriptDelegate* MulticastDelegatePtr;

	FName SparseDelegateName;

	FName BlueprintDelegateName;

	FActorIOEvent() :
		EventId(NAME_None),
		DisplayName(FText::GetEmpty()),
		TooltipText(FText::GetEmpty()),
		DelegateOwner(nullptr),
		MulticastDelegatePtr(nullptr),
		SparseDelegateName(NAME_None),
		BlueprintDelegateName(NAME_None)
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

	FActorIOEvent& SetMulticastDelegate(TObjectPtr<UObject> InDelegateOwner, FMulticastScriptDelegate* InMulticastDelegate)
	{
		DelegateOwner = InDelegateOwner;
		MulticastDelegatePtr = InMulticastDelegate;
		return *this;
	}

	FActorIOEvent& SetSparseDelegate(TObjectPtr<UObject> InDelegateOwner, FName InSparseDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		SparseDelegateName = InSparseDelegateName;
		return *this;
	}

	FActorIOEvent& SetBlueprintDelegate(TObjectPtr<UObject> InDelegateOwner, FName InBlueprintDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		BlueprintDelegateName = InBlueprintDelegateName;
		return *this;
	}

	bool operator==(const FName InEventName) const
	{
		return EventId == InEventName;
	}
};
