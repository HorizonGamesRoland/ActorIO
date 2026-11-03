// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"

class FActorIOExpressionDetailBuilder;

/** Delegate when a new expression builder instance is requested. */
DECLARE_DELEGATE_RetVal(TSharedRef<FActorIOExpressionDetailBuilder>, FOnGetExpressionDetailBuilder);

/**
 * Description of a registered detail builder in FActorIOExpressionBuilderRegistry.
 */
struct ACTORIOEDITOR_API FActorIOExpressionBuilderDesc
{
	/** Delegate that creates a new instance of the expression builder. */
	FOnGetExpressionDetailBuilder DetailBuilderDelegate;

	/** Whether the expression should appear in the add menu of condition containers. */
	bool bCanAddToConditionContainer = true;

	/** Whether the expression should appear in the add menu of non-condition containers. */
	bool bCanAddToNonConditionContainer = true;
};

/**
 * Registry class for storing expression detail builders.
 */
class ACTORIOEDITOR_API FActorIOExpressionBuilderRegistry : public TSharedFromThis<FActorIOExpressionBuilderRegistry>
{
public:

	/** @return The global registry instance of the module. */
	static FActorIOExpressionBuilderRegistry& Get();

	/** Register builders for core expressions on module startup. */
	void Initialize();

	/** Cleanup before module shutdown. */
	void Shutdown();

	/**
	 * Register the detail builder of an expression.
	 *
	 * @param TypeName The type name of the expression.
	 * @param DetailBuilderDelegate Delegate that creates a new instance of the expression builder.
	 * @param bCanAddToConditionContainer Whether the expression should appear in the add menu of condition containers.
	 * @param bCanAddToNonConditionContainer Whether the expression should appear in the add menu of non-condition containers.
	 */
	void RegisterExpressionBuilder(FName TypeName, const FOnGetExpressionDetailBuilder& DetailBuilderDelegate, bool bCanAddToConditionContainer, bool bCanAddToNonConditionContainer);

	/**
	 * Unregister the detail builder of an expression.
	 *
	 * @param TypeName The type name of the expression.
	 */
	void UnregisterExpressionBuilder(FName TypeName);

	/** @return Description for the detail builder of an expression. */
	FActorIOExpressionBuilderDesc* GetExpressionBuilder(FName TypeName) { return ExpressionBuilderMap.Find(TypeName); }
	const FActorIOExpressionBuilderDesc* GetExpressionBuilder(FName TypeName) const { return ExpressionBuilderMap.Find(TypeName); }

	/** @return All registered detail builders. */
	const TMap<FName, FActorIOExpressionBuilderDesc>& GetExpressionBuilders() const { return ExpressionBuilderMap; }

private:

	/** List of registered detail builders. */
	TMap<FName, FActorIOExpressionBuilderDesc> ExpressionBuilderMap;
};
