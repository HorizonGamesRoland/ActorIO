// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "Expressions/ActorIOExpressionBase.h"
#include "Templates/SubclassOf.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActorIOExpressions.generated.h"

/**
 * Expression that evaluates to a literal value.
 */
USTRUCT()
struct ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Literal; }
	virtual FName GetTypeName() const override { return FName("Literal"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue) override;
	virtual FString CheckForErrors() const override;
	//~ End FActorIOExpressionBase Interface

	/** Set the literal value of the expression. */
	void SetLiteralValue(const FString& InValue) { LiteralValue = InValue; }

	/** @return The literal value of the expression. */
	const FString& GetLiteralValue() const { return LiteralValue; }

	/** @return The literal value of the expression as a specific type. */
	TOptional<int32> GetLiteralValueAsInt32() const;
	TOptional<float> GetLiteralValueAsFloat() const;
	TOptional<bool> GetLiteralValueAsBool() const;
	TOptional<FVector> GetLiteralValueAsVector() const;
	TOptional<FVector2D> GetLiteralValueAsVector2D() const;
	TOptional<FRotator> GetLiteralValueAsRotator() const;

	/** Set the literal value of the expression to the default based on the property type. */
	void SetLiteralValueToDefault();

	/** @return Default value to use for the given property type. */
	TOptional<FString> GetDefaultValueForPropType(const FString& InPropType) const;

	/** @return Whether the current value of the expression can be parsed as a given property type (e.g. int32, float, bool, etc...) */
	bool CheckValueValidForPropType(const FString& InPropType) const;

protected:

	/** String value of the expression. */
	UPROPERTY()
	FString LiteralValue;
};

/**
 * Expression that executes a UFunction from a BlueprintFunctionLibrary and returns its result.
 * Child expressions map to the parameters of the selected UFunction.
 * If negated, UFunction with bool return value will be inverted.
 */
USTRUCT()
struct ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual FName GetTypeName() const override { return FName("KismetFunction"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue) override;
	virtual FString CheckForErrors() const override;
	//~ End FActorIOExpressionBase Interface

	/** Set the class that contains the function. */
	void SetFunctionClass(UClass* InClassPtr, bool bResetFunctionName = false);

	/** Set the name of the function to execute. */
	void SetFunctionName(FName InFunctionName);

	/** @return Class that contains the function. */
	const UClass* GetFunctionClass() const { return FunctionClass.Get(); }

	/** @return Name of the function to execute. */
	FName GetFunctionName() const { return FunctionName; }

	/** @return UFunction that will be executed by the expression. */
	UFunction* ResolveUFunction() const;

	/** @return Whether child expressions map to the parameters of the selected UFunction. */
	bool CheckArgumentsNeedUpdate() const;

	/**
	 * Updates child expressions to match the parameters of the selected UFunction.
	 * This will add/remove child expressions to match the parameter count.
	 * Also handles cases where the UFunction's parameter order/type has changed.
	 */
	void UpdateArguments();

protected:

	/** The class that contains the function. */
	UPROPERTY()
	TSubclassOf<UBlueprintFunctionLibrary> FunctionClass;

	/** Name of the function to execute. */
	UPROPERTY()
	FName FunctionName;
};

/** Valid comparison types for FActorIOCompareExpression. */
UENUM()
enum class EActorIOComparisonOp : uint8
{
	Less,
	LessOrEqual,
	Equal,
	NotEqual,
	GreaterOrEqual,
	Greater,
};

/**
 * Expression that compares two values.
 * If negated, inverts the result of the comparison.
 */
USTRUCT()
struct ACTORIO_API FActorIOCompareExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	FActorIOCompareExpression();

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual FName GetTypeName() const override { return FName("Compare"); }
	virtual bool Init() override;
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue) override;
	virtual FString CheckForErrors() const override;
	//~ End FActorIOExpressionBase Interface

	/** Set the operator to use when comparing the values. */
	void SetComparisonOp(const EActorIOComparisonOp& Op) { ComparisonOp = Op; }

	/** @return The operator to use when comparing the values. */
	const EActorIOComparisonOp& GetComparisonOp() const { return ComparisonOp; }

	/** Convert an operator to string. */
	static FString OpToKeyword(const EActorIOComparisonOp& Op);

	/** Convert a string into an operator. */
	static TOptional<EActorIOComparisonOp> OpFromKeyword(const FString& Keyword);

	/** @return Whether child expressions are exactly two. */
	bool CheckArgumentCountValid() const;

protected:

	/** The operator to use when comparing the values. */
	UPROPERTY()
	EActorIOComparisonOp ComparisonOp;
};

/**
 * Expression that evaluates a list of child expressions sequentially.
 * If the owning container is storing conditions, childs are evaluated in an AND/OR relation based on negation.
 */
USTRUCT()
struct ACTORIO_API FActorIOGroupExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Group; }
	virtual FName GetTypeName() const override { return FName("Group"); }
	virtual bool CanEditChildExpressions() const override { return true; }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue) override;
	//~ End FActorIOExpressionBase Interface
};
