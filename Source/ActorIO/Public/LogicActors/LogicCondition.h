// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "LogicCondition.generated.h"

UCLASS(DefaultToInstanced, EditInlineNew)
class ACTORIO_API UActorIOExpression : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bFunctionIsKismetOp"))
	TSoftObjectPtr<AActor> ObjectPtr;

	UPROPERTY(EditAnywhere)
	FName FunctionId;

	// #temp
	UPROPERTY(EditAnywhere)
	bool bFunctionIsKismetOp;

	UPROPERTY(EditAnywhere)
	TArray<UActorIOExpression*> Args;

public:

	bool Evaluate(FString& OutResult);
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

	UPROPERTY(Instanced, EditInstanceOnly)
	UActorIOExpression* Expression;

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
