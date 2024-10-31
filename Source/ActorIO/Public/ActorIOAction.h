// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOEvent.h"
#include "ActorIOFunction.h"
#include "UObject/NoExportTypes.h"
#include "ActorIOAction.generated.h"

class UActorIOComponent;

USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName SourceEvent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere)
	FName TargetFunction;

	UPROPERTY(EditAnywhere)
	FString FunctionArguments;

	FActorIOAction() :
		SourceEvent(FName()),
		TargetActor(nullptr),
		TargetFunction(FName()),
		FunctionArguments(FString())
	{}

	bool IsValid() const
	{
		return !SourceEvent.IsNone() && TargetActor && !TargetFunction.IsNone();
	}
};

UCLASS()
class ACTORIO_API UActorIOLink : public UObject
{
	GENERATED_BODY()

public:

	UActorIOLink();

protected:

	UPROPERTY()
	FActorIOAction LinkedAction;

	bool bWasExecuted;

	FScriptDelegate ActionDelegate;

public:

	void BindAction(const FActorIOAction& Action);

	void ClearAction();
		
	UActorIOComponent* GetOwnerIOComponent() const;

protected:

	void AttemptBindNativeAction();

	UFUNCTION()
	void ExecuteAction();

public:

	virtual void BeginDestroy() override;
};
