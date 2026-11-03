// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Expressions/ActorIOExpressionRegistry.h"
#include "Expressions/ActorIOExpressions.h"
#include "ActorIOSubsystemBase.h"
#include "ActorIOModule.h"

#define LOCTEXT_NAMESPACE "ActorIO"

FActorIOExpressionRegistry& FActorIOExpressionRegistry::Get()
{
	return *FModuleManager::GetModuleChecked<FActorIOModule>("ActorIO").GetExpressionRegistry();
}

void FActorIOExpressionRegistry::Initialize()
{
	RegisterExpression(TEXT("Literal"), LOCTEXT("LiteralExpressionName", "Literal"), FOnGetExpressionInstance::CreateStatic(FActorIOLiteralExpression::MakeInstance), TEXT("val"));
	RegisterExpression(TEXT("KismetFunction"), LOCTEXT("KismetFunctionExpressionName", "Kismet Function"), FOnGetExpressionInstance::CreateStatic(FActorIOKismetFunctionExpression::MakeInstance), TEXT("func"));
	RegisterExpression(TEXT("Compare"), LOCTEXT("CompareExpressionName", "Compare"), FOnGetExpressionInstance::CreateStatic(FActorIOCompareExpression::MakeInstance), TEXT("eq,ne,lt,le,gt,ge"));
	RegisterExpression(TEXT("Group"), LOCTEXT("GroupExpressionName", "Group"), FOnGetExpressionInstance::CreateStatic(FActorIOGroupExpression::MakeInstance), TEXT("and,or"));
}

void FActorIOExpressionRegistry::Shutdown()
{
}

void FActorIOExpressionRegistry::RegisterExpression(FName TypeName, const FText& DisplayName, const FOnGetExpressionInstance& InstanceDelegate, const FString& Keywords)
{
	if (TypeName == NAME_None || !InstanceDelegate.IsBound() || Keywords.IsEmpty())
	{
		ensureAlwaysMsgf(false, TEXT("Expression type, instance delegate, and at least one keyword must be provided."));
		return;
	}

	FActorIOExpressionDesc Desc;
	Desc.DisplayName = DisplayName;
	Desc.InstanceDelegate = InstanceDelegate;
	Keywords.ParseIntoArray(Desc.Keywords, TEXT(","));

	ExpressionsMap.Add(TypeName, Desc);
}

void FActorIOExpressionRegistry::UnregisterExpression(FName TypeName)
{
	ExpressionsMap.Remove(TypeName);
}

#undef LOCTEXT_NAMESPACE
