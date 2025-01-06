// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "UObject/NoExportTypes.h"
#include "ActorIOAction.generated.h"

class UActorIOComponent;

UCLASS(DefaultToInstanced, EditInlineNew)
class ACTORIO_API UActorIOAction : public UObject
{
	GENERATED_BODY()

public:

	UActorIOAction();

public:

	UPROPERTY(EditAnywhere, Category = "Action")
	FName EventId;

	UPROPERTY(EditAnywhere, Category = "Action")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, Category = "Action")
	FName FunctionId;

	UPROPERTY(EditAnywhere, Category = "Action")
	FString FunctionArguments;

	UPROPERTY(EditAnywhere, Category = "Action")
	float Delay;

	UPROPERTY(EditAnywhere, Category = "Action")
	bool bExecuteOnlyOnce;

protected:

	bool bWasExecuted;

	bool bIsBound;

	FScriptDelegate ActionDelegate;

public:

	void BindAction();

	void UnbindAction();

	UActorIOComponent* GetOwnerIOComponent() const;

	AActor* GetOwnerActor() const;

public:

	static FName ExecuteActionSignalName;

protected:

	UFUNCTION()
	void ReceiveExecuteAction();

	bool CanExecuteAction(FActionExecutionContext& ExecutionContext);

	void ExecuteAction(FActionExecutionContext& ExecutionContext);

	void SendCommand(FString Command);

public:

	virtual void ProcessEvent(UFunction* Function, void* Parms) override;
};
