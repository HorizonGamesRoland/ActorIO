// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"

/** Delegate when expression values are requested. */
DECLARE_DELEGATE_RetVal(TArray<FString>, FOnGetExpressionValues);

/**
 * Description of a registered value provider in FActorIOExpressionValueProvider.
 */
struct ACTORIOEDITOR_API FActorIOExpressionValueProviderDesc
{
	/** Delegate that provides the expression values when requested. */
	FOnGetExpressionValues ValueProviderDelegate;
};

/**
 * Singleton class for providing pre-determined values for expressions.
 */
class ACTORIOEDITOR_API FActorIOExpressionValueProvider : public TSharedFromThis<FActorIOExpressionValueProvider>
{
public:

	/** @return Reference to the value provider singleton of the module. */
	static FActorIOExpressionValueProvider& Get();

	/**
	 * Set a value provider for the given ID.
	 *
	 * @param ID The id of the provider.
	 * @param ValueProviderDelegate Delegate that provides the expression values when requested.
	 */
	void SetValueProvider(const FString& ID, const FOnGetExpressionValues& ValueProviderDelegate);

	/**
	 * Remove the value provider for the given ID.
	 *
	 * @param ID The id of the provider.
	 */
	void RemoveValueProvider(const FString& ID);

	/** @return Description of the value provider of the given ID. */
	FActorIOExpressionValueProviderDesc* GetValueProvider(const FString& ID) { return ValueProvidersMap.Find(ID); }
	const FActorIOExpressionValueProviderDesc* GetValueProvider(const FString& ID) const { return ValueProvidersMap.Find(ID); }

	/** @return All registered value providers. */
	const TMap<FString, FActorIOExpressionValueProviderDesc>& GetValueProviders() const { return ValueProvidersMap; }

private:

	/** List of registered value providers. */
	TMap<FString, FActorIOExpressionValueProviderDesc> ValueProvidersMap;
};
