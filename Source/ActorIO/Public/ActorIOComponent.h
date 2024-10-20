// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Components/ActorComponent.h"
#include "ActorIOComponent.generated.h"

USTRUCT()
struct FActorIOAction
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties")
	FGuid ActionId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties")
	FName SourceEvent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties")
	FName TargetFunction;

	FDelegateHandle ActionDelegateHandle;

	FActorIOAction() :
		ActionId(FGuid::NewGuid()),
		SourceEvent(FName()),
		TargetActor(nullptr),
		TargetFunction(FName()),
		ActionDelegateHandle(FDelegateHandle())
	{}

	bool IsValid() const
	{
		return ActionId.IsValid() && !SourceEvent.IsNone() && TargetActor && !TargetFunction.IsNone();
	}
};

UCLASS()
class ACTORIO_API UActorIOComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY()
	TArray<FActorIOAction> ActionBindings;

public:

	void AddAction(const FActorIOAction& NewAction);

	void RemoveAction(const FGuid& ActionId);

	bool GetAction(const FGuid& ActionId, FActorIOAction& OutAction);

	bool HasAction(const FGuid& ActionId) const;

protected:

	void BindActions();

	void RemoveActionBindings();

public:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};