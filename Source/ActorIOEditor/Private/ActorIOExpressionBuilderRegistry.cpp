// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpressionBuilderRegistry.h"
#include "ActorIOExpressionCustomization.h"
#include "ActorIOEditor.h"

FActorIOExpressionBuilderRegistry& FActorIOExpressionBuilderRegistry::Get()
{
	return *FActorIOEditor::Get().GetExpressionBuilderRegistry();
}

void FActorIOExpressionBuilderRegistry::RegisterExpressionBuilder(FName TypeName, const FOnGetExpressionDetailBuilder& DetailBuilderDelegate, bool bCanAddToConditionContainer, bool bCanAddToNonConditionContainer)
{
	if (TypeName == NAME_None || !DetailBuilderDelegate.IsBound())
	{
		ensureAlwaysMsgf(false, TEXT("Expression type, and detail builder delegate must be provided."));
		return;
	}

	FActorIOExpressionBuilderDesc Desc;
	Desc.DetailBuilderDelegate = DetailBuilderDelegate;
	Desc.bCanAddToConditionContainer = bCanAddToConditionContainer;
	Desc.bCanAddToNonConditionContainer = bCanAddToNonConditionContainer;

	ExpressionBuilderMap.Add(TypeName, Desc);
}

void FActorIOExpressionBuilderRegistry::UnregisterExpressionBuilder(FName TypeName)
{
	ExpressionBuilderMap.Remove(TypeName);
}
