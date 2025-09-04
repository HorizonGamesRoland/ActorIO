// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UObject/WeakObjectPtr.h"
#include "ActorIOEditorSubsystem.generated.h"

class AActor;
class UActorIOComponent;

/**
 * Editor subsystem for integrating the level editor, and other editor features with the Actor I/O plugin.
 */
UCLASS()
class ACTORIOEDITOR_API UActorIOEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:

	/** @return The editor subsystem of the Actor I/O plugin. */
	static UActorIOEditorSubsystem* Get();

	/** @return The actor that is currently selected in the editor. */
	AActor* GetSelectedActor() const;

	/** Add an actor I/O component to the given actor. */
	UActorIOComponent* AddIOComponentToActor(AActor* TargetActor, bool bSelectActor);

private:

	/** The currently selected actor in the level editor. */
	TWeakObjectPtr<AActor> SelectedActor;

	/** Handle for when a new object is selected in the editor. */
	FDelegateHandle DelegateHandle_SelectionChanged;
	/** Handle for when an actor is deleted in the level editor. */
	FDelegateHandle DelegateHandle_DeleteActorsBegin;
	/** Handle for when an actor is cut in the level editor. */
	FDelegateHandle DelegateHandle_CutActorsBegin;
	/** Handle for when an actor is replaced in the level editor. */
	FDelegateHandle DelegateHandle_ActorReplaced;
	/** Handle for when a blueprint is compiled in the editor. */
	FDelegateHandle DelegateHandle_BlueprintCompiled;

private:

	/** Called when a new object is selected in the editor. */
	void OnObjectSelectionChanged(UObject* NewSelection);

	/** Called when an actor is cut or deleted in the level editor. */
	void OnDeleteOrCutActorsBegin();

	/** Called when an actor is replaced in the level editor. */
	void OnActorReplaced(AActor* OldActor, AActor* NewActor);

	/** Called when a blueprint is compiled in the editor. */
	void OnBlueprintCompiled();

public:

	//~ Begin UEditorSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End UEditorSubsystem Interface
};
