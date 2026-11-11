// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpressionValueProvider.h"
#include "ActorIOEditor.h"

FActorIOExpressionValueProvider& FActorIOExpressionValueProvider::Get()
{
	return *FActorIOEditor::Get().GetExpressionValueProvider();
}

void FActorIOExpressionValueProvider::SetValueProvider(const FString& ID, const FOnGetExpressionValues& ValueProviderDelegate)
{
	if (ID.IsEmpty() || !ValueProviderDelegate.IsBound())
	{
		ensureAlwaysMsgf(false, TEXT("Expression value provider ID, and delegate must be provided."));
		return;
	}

	FActorIOExpressionValueProviderDesc Desc;
	Desc.ValueProviderDelegate = ValueProviderDelegate;

	ValueProvidersMap.Add(ID, Desc);
}

void FActorIOExpressionValueProvider::RemoveValueProvider(const FString& ID)
{
	ValueProvidersMap.Remove(ID);
}
