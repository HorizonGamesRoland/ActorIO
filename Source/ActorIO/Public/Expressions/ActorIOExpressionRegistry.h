// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "StructUtils/InstancedStruct.h"

struct FActorIOExpressionBase;

/** Delegate when a new expression instance is requested. Keyword can be empty. */
DECLARE_DELEGATE_RetVal_OneParam(TInstancedStruct<FActorIOExpressionBase>, FOnGetExpressionInstance, const FString& /** Keyword */);

/**
 * Description of a registered expression in FActorIOExpressionRegistry.
 */
struct ACTORIO_API FActorIOExpressionDesc
{
	/** Display name of the expression. */
	FText DisplayName;

	/** Delegate that creates a new instance of the expression. */
	FOnGetExpressionInstance InstanceDelegate;

	/** List of keywords associated with the expression. (val, and, or, etc...) */
	TArray<FString> Keywords;
};

/**
 * Registry class for storing known expression types.
 */
class ACTORIO_API FActorIOExpressionRegistry : public TSharedFromThis<FActorIOExpressionRegistry>
{
public:

	/** @return The global registry instance of the module. */
	static FActorIOExpressionRegistry& Get();

	/** Register core expression types on module startup. */
	void Initialize();

	/** Cleanup before module shutdown. */
	void Shutdown();

	/**
	 * Register a new expression type.
	 * 
	 * @param TypeName The type name of the expression.
	 * @param InstanceDelegate Delegate that creates a new instance of the expression.
	 * @param Keywords List of keywords associated with the expression (val, and, or, etc...). Use comma "," as separator, and avoid whitespaces.
	 */
	void RegisterExpression(FName TypeName, const FText& DisplayName, const FOnGetExpressionInstance& InstanceDelegate, const FString& Keywords);

	/**
	 * Unregister the expression from the registry.
	 * 
	 * @param TypeName The type name of the expression.
	 */
	void UnregisterExpression(FName TypeName);

	/** @return Description of a registered expression. */
	FActorIOExpressionDesc* GetExpression(FName TypeName) { return ExpressionsMap.Find(TypeName); }
	const FActorIOExpressionDesc* GetExpression(FName TypeName) const { return ExpressionsMap.Find(TypeName); }

	/** @return All registered expressions. */
	const TMap<FName, FActorIOExpressionDesc>& GetExpressions() const { return ExpressionsMap; }

private:

	/** List of registered expressions. */
	TMap<FName, FActorIOExpressionDesc> ExpressionsMap;
};
