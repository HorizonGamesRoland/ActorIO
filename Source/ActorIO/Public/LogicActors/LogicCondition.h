// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "StructUtils/InstancedStruct.h"
#include "LogicCondition.generated.h"

USTRUCT()
struct ACTORIO_API FActorIOExpressionBase
{
	GENERATED_BODY()

	virtual ~FActorIOExpressionBase() = default;

	virtual bool Evaluate(FString& OutResult) PURE_VIRTUAL(UActorIOExpression::Evaluate(), return false;)
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionLiteral : public FActorIOExpressionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString LiteralValue;

	virtual bool Evaluate(FString& OutResult) override;
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionFunction : public FActorIOExpressionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bFunctionIsKismetOp"))
	TSoftObjectPtr<AActor> ObjectPtr;

	UPROPERTY(EditAnywhere)
	FName FunctionId;

	// #temp
	UPROPERTY(EditAnywhere)
	bool bFunctionIsKismetOp;

	UPROPERTY(EditAnywhere)
	TArray<TInstancedStruct<FActorIOExpressionBase>> Args;

	virtual bool Evaluate(FString& OutResult) override;
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionConditionProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TInstancedStruct<FActorIOExpressionFunction> Expression;
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
	FActorIOExpressionConditionProperty Condition;

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
