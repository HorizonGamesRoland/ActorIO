// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "ActorIOExpression.h"
#include "LogicCondition.generated.h"

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
