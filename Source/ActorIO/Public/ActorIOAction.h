// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "UObject/NoExportTypes.h"
#include "ActorIOAction.generated.h"

class UActorIOComponent;
struct FActionExecutionContext;

UCLASS(DefaultToInstanced, EditInlineNew)
class ACTORIO_API UActorIOAction : public UObject
{
	GENERATED_BODY()

public:

	UActorIOAction();

public:

	UPROPERTY(EditAnywhere)
	FName EventId;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere)
	FName FunctionId;

	UPROPERTY(EditAnywhere)
	FString FunctionArguments;

	UPROPERTY(EditAnywhere)
	float Delay;

	UPROPERTY(EditAnywhere)
	bool bExecuteOnlyOnce;

public:

	static FName ExecuteActionSignalName;

protected:

	bool bWasExecuted;

	bool bIsBound;

	FScriptDelegate ActionDelegate;

public:

	void BindAction();

	void UnbindAction();

	UActorIOComponent* GetOwnerIOComponent() const;

	AActor* GetOwnerActor() const;

protected:

	UFUNCTION()
	void ReceiveExecuteAction();

	bool CanExecuteAction(FActionExecutionContext& ExecutionContext);

	void ProcessAction(FActionExecutionContext& ExecutionContext);

	void ExecuteAction(FString Command);

public:

	virtual void ProcessEvent(UFunction* Function, void* Parms) override;
};
