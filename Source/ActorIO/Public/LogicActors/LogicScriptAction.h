// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "ActorIOExpressions.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicScriptAction.generated.h"

/**
 * 
 */
UCLASS()
class ACTORIO_API ALogicScriptAction : public ALogicActorBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	ALogicScriptAction();

public:

	/** Event when the condition passes after 'Test' is called. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSimpleActionDelegate OnExecute;

protected:

	UPROPERTY(EditInstanceOnly, Category = "Script Action", meta = (HideRootExpression))
	FActorIOExpressionContainer Actions;

public:

	//~ Begin ALogicActorBase Interface
	virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
	virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	//~ End ALogicActorBase Interface

public:

	/** Execute the actions and fire the 'OnExecute' event. */
	UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicScriptAction")
	bool Execute();
};
