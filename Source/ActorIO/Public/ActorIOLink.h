// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOTypes.h"
#include "UObject/NoExportTypes.h"
#include "ActorIOLink.generated.h"

class UActorIOComponent;

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
