// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Tickable.h"
#include "ActorIOEditorSubsystem.generated.h"

UCLASS()
class ACTORIOEDITOR_API UActorIOEditorSubsystem : public UEditorSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

private:

	UPROPERTY()
	TObjectPtr<AActor> SelectedActor;

public:

	//~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual bool IsTickableWhenPaused() const { return true; }
	virtual bool IsTickableInEditor() const { return true; }
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject Interface
};
