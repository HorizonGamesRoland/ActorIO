// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Tickable.h"
#include "ActorIOEditorSubsystem.generated.h"

class AActor;
class UActorIOAction;

UCLASS()
class ACTORIOEDITOR_API UActorIOEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

private:

	UPROPERTY()
	TObjectPtr<AActor> SelectedActor;

public:

	AActor* GetSelectedActor() const { return SelectedActor.Get(); }

protected:

	void OnSelectionChanged(UObject* NewSelection);

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
