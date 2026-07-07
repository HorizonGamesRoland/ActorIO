// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCondition.generated.h"

UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class ACTORIO_API UActorIOExpression : public UObject
{
	GENERATED_BODY()

public:

	virtual bool Evaluate(FString& OutResult) PURE_VIRTUAL(UActorIOExpression::Evaluate(), return false;)
};

UCLASS()
class ACTORIO_API UActorIOExpressionLiteral : public UActorIOExpression
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere)
	FString StringValue;

public:

	virtual bool Evaluate(FString& OutResult) override;
};

UCLASS()
class ACTORIO_API UActorIOExpressionFunction : public UActorIOExpression
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bFunctionIsKismetOp"))
	TSoftObjectPtr<AActor> ObjectPtr;

	UPROPERTY(EditAnywhere)
	FName FunctionId;

	// #temp
	UPROPERTY(EditAnywhere)
	bool bFunctionIsKismetOp;

	UPROPERTY(Instanced, EditAnywhere)
	TArray<TObjectPtr<UActorIOExpression>> Args;

public:

	virtual bool Evaluate(FString& OutResult) override;
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionCondition
{
	GENERATED_BODY()

	UPROPERTY(Instanced, EditAnywhere)
	TObjectPtr<UActorIOExpressionFunction> Expression;

	FActorIOExpressionCondition() :
		Expression(nullptr)
	{}
};

/**
 * 
 */
UCLASS()
class ACTORIO_API ALogicCondition : public ALogicActorBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	ALogicCondition();

public:

	/** Event when the condition passes after 'Test' is called. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSimpleActionDelegate OnPass;

	/** Event when the condition fails after 'Test' is called. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSimpleActionDelegate OnFail;

protected:

	UPROPERTY(EditInstanceOnly)
	FActorIOExpressionCondition Condition;

	// #temp
	UPROPERTY(Instanced, EditInstanceOnly)
	TObjectPtr<UActorIOExpressionFunction> Expression;

public:

	//~ Begin ALogicActorBase Interface
	virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
	virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
	//~ End ALogicActorBase Interface

public:

	/** Test the condition and fire 'OnPass' or 'OnFail' based on the result. */
	UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicCondition")
	void Test();
};
