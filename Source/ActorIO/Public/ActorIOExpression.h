// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "UObject/WeakObjectPtr.h"
#include "ActorIOExpression.generated.h"

UENUM()
enum class EActorIOExpressionType : uint8
{
	Invalid,
	Literal,
	Function
};

class ACTORIO_API FActorIOExpressionBase
{
public:

	FActorIOExpressionBase();
	virtual ~FActorIOExpressionBase() = default;

	virtual EActorIOExpressionType GetType() const { return EActorIOExpressionType::Invalid; }
	virtual FName GetSubType() const { return NAME_None; }
	virtual bool Evaluate(FString& OutResult) { return false; }
	virtual void Serialize(FArchive& Ar);

	void SetParent(FActorIOExpressionBase* InExpr) { ParentExpr = InExpr; }
	FActorIOExpressionBase* GetParent() const { return ParentExpr; }

protected:

	FActorIOExpressionBase* ParentExpr;
};

class ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
public:

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Literal; }
	virtual bool Evaluate(FString& OutResult) override;
	virtual void Serialize(FArchive& Ar) override;

	void SetLiteralValue(const FString& InValue) { LiteralValue = InValue; }
	const FString& GetLiteralValue() const { return LiteralValue; }

protected:

	FString LiteralValue;
};

class ACTORIO_API FActorIOFunctionExpressionBase: public FActorIOExpressionBase
{
public:

	virtual ~FActorIOFunctionExpressionBase();

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Function; }
	virtual void Serialize(FArchive& Ar) override;

	void AddArgument(FActorIOExpressionBase* InExpr);
	void RemoveArgument(FActorIOExpressionBase* InExpr);
	void SetArgumentAt(int32 Index, FActorIOExpressionBase* InExpr);
	void ResetArguments();

	const TArray<FActorIOExpressionBase*>& GetArguments() const { return Args; }
	FActorIOExpressionBase* GetArgumentAt(int32 Index) const;

protected:

	TArray<FActorIOExpressionBase*> Args;
};

class ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
public:

	virtual FName GetSubType() const { return FName("KismetFunction"); }
	virtual bool Evaluate(FString& OutResult) override;
	virtual void Serialize(FArchive& Ar) override;

	void SetFunctionClass(UClass* InClassPtr);
	void SetFunctionId(FName InFunctionId);

	const TWeakObjectPtr<UClass>& GetClass() const { return ClassPtr; }
	FName GetFunctionId() const { return FunctionId; }
	UFunction* GetUFunction();
	TArray<FProperty*> GetUFunctionParams();

protected:

	void UpdateArguments();

	TWeakObjectPtr<UClass> ClassPtr;

	FName FunctionId;
};

class ACTORIO_API FActorIOGroupExpression : public FActorIOFunctionExpressionBase
{
public:

	virtual FName GetSubType() const { return FName("Group"); }
	virtual bool Evaluate(FString& OutResult) override;
};

USTRUCT()
struct ACTORIO_API FActorIOScriptCondition
{
	GENERATED_BODY()

	FActorIOGroupExpression Expr;

	//bool operator==(const FActorIOScriptCondition& Other) const = default;

	bool Serialize(FArchive& Ar);
};

template<>
struct TStructOpsTypeTraits<FActorIOScriptCondition> : public TStructOpsTypeTraitsBase2<FActorIOScriptCondition>
{
	enum
	{
		WithSerializer = true,
		//WithIdenticalViaEquality = true
	};
};

class ACTORIO_API FActorIOExpresionHelper
{
public:

	static FActorIOExpressionBase* CreateExpressionFromType(EActorIOExpressionType InType, FName InSubType);
};
