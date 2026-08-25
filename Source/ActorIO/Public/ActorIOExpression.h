// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "ActorIOExpression.generated.h"

enum class ACTORIO_API EActorIOExpressionType : uint8
{
	Invalid,
	Literal,
	Function
};

struct ACTORIO_API FActorIOExpressionBase
{
	virtual ~FActorIOExpressionBase() = default;

	FActorIOExpressionBase* ParentExpr;

	virtual EActorIOExpressionType GetType() const { return EActorIOExpressionType::Invalid; }
	virtual FName GetSubType() const { return NAME_None; }
	virtual bool Evaluate(FString& OutResult) { return false; }

	void SetParent(FActorIOExpressionBase* InExpr) { ParentExpr = InExpr; }
	FActorIOExpressionBase* GetParent() const { return ParentExpr; }
};

struct ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
	FString LiteralValue;

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Literal; }
	virtual bool Evaluate(FString& OutResult) override;
};

struct ACTORIO_API FActorIOFunctionExpressionBase: public FActorIOExpressionBase
{
	TArray<FActorIOExpressionBase*> Args;

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Function; }

	void AddArgument(FActorIOExpressionBase* InExpr);
	void RemoveArgument(FActorIOExpressionBase* InExpr);
	void SetArgumentAt(int32 Index, FActorIOExpressionBase* InExpr);
	void ResetArguments();
	FActorIOExpressionBase* GetArgumentAt(int32 Index) const;
};

struct ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
	UClass* ClassPtr;

	FName FunctionId;

	virtual FName GetSubType() const { return FName(TEXT("KismetFunction")); }
	virtual bool Evaluate(FString& OutResult) override;

	void SetFunctionClass(UClass* InClassPtr);

	void SetFunctionId(FName InFunctionId);

	void UpdateArguments();

	UFunction* GetUFunction();

	TArray<FProperty*> GetUFunctionParams();
};

struct ACTORIO_API FActorIOGroupExpression : public FActorIOFunctionExpressionBase
{
	virtual FName GetSubType() const { return FName(TEXT("Group")); }
	virtual bool Evaluate(FString& OutResult) override;
};

USTRUCT()
struct ACTORIO_API FActorIOScriptCondition
{
	GENERATED_BODY()

	FActorIOGroupExpression Expression;
};
