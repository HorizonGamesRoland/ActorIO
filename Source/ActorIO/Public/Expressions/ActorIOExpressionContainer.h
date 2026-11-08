// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "StructUtils/InstancedStruct.h"
#include "ActorIOExpressionContainer.generated.h"

struct FActorIOExpressionBase;

/**
 * Container for storing expressions in a custom flat tree structure.
 * The tree consists of two arrays, storing expression instances and their parent relations separately.
 * Expressions can have any number of children, but some expressions may apply limitations.
 * There is only ever one root expression in the tree with its parent index being -1.
 * Elements of the tree are ordered, ensuring child expressions are always below their parents and the root is always at the top.
 * 
 * This complicated tree structure was necessary due to Unreal's property system not supporting nested properties with the same names.
 * @see FPropertyValueImpl::GenerateArrayIndexMapToObjectNode
 */
USTRUCT()
struct ACTORIO_API FActorIOExpressionContainer
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	FActorIOExpressionContainer();

	/**
	 * Initialize the expression tree from a string.
	 * Empties the container if any elements are present.
	 */
	bool InitFromString(const FString& Str);

	/** @return String representation of the expressions tree. */
	FString ToString() const;

	/**
	 * Add an expression instance to the tree.
	 * 
	 * @param Expr The expression instance to add.
	 * @param ParentIdx Index of the parent who should own the expression. Should be INDEX_NONE when adding the root expression.
	 * @param SlotIdx Index of where the expression should be under the parent. If < 0, it is added to the end.
	 * @return Index of the expression after successfully adding it to the tree. Otherwise INDEX_NONE.
	 */
	int32 AddExpression(const TInstancedStruct<FActorIOExpressionBase>& Expr, int32 ParentIdx, int32 SlotIdx = INDEX_NONE);

	/**
	 * Add an expression to the tree by string.
	 *
	 * @param Str The expression to add in string form.
	 * @param ParentIdx Index of the parent who should own the expression.
	 * @param SlotIdx Index of where the expression should be under the parent. If < 0, it is added to the end.
	 * @return Index of the expression after successfully adding it to the tree. Otherwise INDEX_NONE.
	 */
	int32 AddExpressionFromString(const FString& Str, int32 ParentIdx, int32 SlotIdx = INDEX_NONE);

	/** Remove the expression (and their childs) at the given index. */
	void RemoveExpression(int32 ExprIdx);

	/** Remove all child expressions of the expression at the given index. */
	void RemoveChildExpressions(int32 ExprIdx);

	/** Remove all elements from the expression tree. */
	void Empty();

	/**
	 * Move the expression (and their childs) at the given index to a new place in the tree.
	 * Note that this rebuilds the entire tree!
	 *
	 * @param ExprIdx Index of the expression to move.
	 * @param ParentIdx Index of the parent who should own the expression.
	 * @param SlotIdx Index of where the expression should be under the parent. If < 0, it is moved to the end.
	 * @return Index of the expression after successfully moving in the tree. Otherwise INDEX_NONE.
	 */
	int32 MoveExpressionTo(int32 ExprIdx, int32 ParentIdx, int32 SlotIdx);

	/** @return All expression instances of the tree. */
	TArray<TInstancedStruct<FActorIOExpressionBase>>& GetExpressions() { return Expressions; }
	const TArray<TInstancedStruct<FActorIOExpressionBase>>& GetExpressions() const { return Expressions; }

	/** @return Number of expressions in the tree. */
	int32 GetNumExpressions() const { return Expressions.Num(); }

	/** @return Index of the expression. */
	int32 GetExpressionIdx(const FActorIOExpressionBase* InExpr) const;

	/** @return Index of the expression that owns the expression at the given index. Returns INDEX_NONE for the root element. */
	int32 GetExpressionParentIdx(int32 Idx) const;

	/** @return Slot index where the expression is under the parent. */
	int32 GetExpressionSlotIdx(int32 Idx) const;

	/** @return Whether the expression tree is empty. */
	bool IsEmpty() const;

	/** @return Whether the expression at the given index is valid. */
	bool IsValidIndex(int32 Idx) const;

	/** @return The expression instance at the given index. */
	TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) { return Expressions[Idx]; }
	const TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) const { return Expressions[Idx]; }

	/** @return The actual struct data of the expression instance at the given index. */
	template<typename T>
	T* GetExpressionPtr(int32 Idx)
	{
		if (Expressions.IsValidIndex(Idx) && Expressions[Idx].IsValid()) {
			return Expressions[Idx].GetMutablePtr<T>();
		}

		return nullptr;
	}

	/** @return The actual struct data of the expression instance at the given index. */
	template<typename T>
	const T* GetExpressionPtr(int32 Idx) const
	{
		if (Expressions.IsValidIndex(Idx) && Expressions[Idx].IsValid()) {
			return Expressions[Idx].GetPtr<T>();
		}
			
		return nullptr;
	}

	/** @return The actual struct data of the expression instance at the given index. */
	FORCEINLINE FActorIOExpressionBase* GetExpressionPtr(int32 Idx) { return GetExpressionPtr<FActorIOExpressionBase>(Idx); }
	FORCEINLINE const FActorIOExpressionBase* GetExpressionPtr(int32 Idx) const { return GetExpressionPtr<FActorIOExpressionBase>(Idx); }

	/** @return Number of expressions owned by the expression at the given index. */
	int32 GetNumChildExpressions(int32 ExprIdx, bool bRecursive = false) const;

	/** @return Indexes of expressions owned by the expression at the given index. */
	TArray<int32> GetChildExpressionIdxs(int32 ExprIdx) const;

	/** @return Whether the expression at the given index is nested anywhere within the ancestor expression. */
	bool IsExpressionDescendantOf(int32 ExprIdx, int32 AncestorIdx) const;

	/**
	 * Assign this container as the owner of all expressions in the tree.
	 * Called automatically during transaction and load serializations.
	 */
	void FixupContainerReferences();

	/** Event after container references were set for all expressions in the tree. */
	FSimpleMulticastDelegate& OnFixupContainerReferences() { return FixupContainerReferencesEvent; }

	/**
	 * Set whether this container is intended to store and evaluate conditions.
	 * This fundamentally changes evaluation and editor UI behaviour.
	 * If enabled, function expressions can be negated and only functions with bool return value are allowed.
	 */
	void SetIsConditionContainer(bool bEnabled) { bIsConditionContainer = bEnabled; }

	/** @return Whether this container is intended to store and evaluate conditions. */
	bool IsConditionContainer() const { return bIsConditionContainer; }

	/**
	 * Helper function to evaluate all expressions in the tree.
	 *
	 * @return Whether expressions were evaluated successfully. If condition container, value is the result of the condition.
	 */
	bool Evaluate(UObject* Executor);

	//~ Begin TStructOpsTypeTraits Interface
	bool Serialize(FArchive& Ar);
	void PostSerialize(const FArchive& Ar);
	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);
	//~ End TStructOpsTypeTraits Interface

	/** Check all expressions in the tree for any errors. Stops at the first error found. */
	bool HasAnyErrors(FString& OutErrorCode) const;

	/** Log all expressions and their parent indexes to the console. */
	void DebugDumpContainerData() const;

protected:

	/**
	 * First half of the expression tree, the expression instances themselves.
	 * Expressions are wrapped in TInstancedStruct, so that they can be allocated and stored dynamically.
	 * Marked as EditAnywhere to generate an IPropertyHandle, used by the container property customization.
	 */
	UPROPERTY(EditAnywhere)
	TArray<TInstancedStruct<FActorIOExpressionBase>> Expressions;

	/**
	 * Second half of the expression tree, the expression parent relations.
	 * Elements in this array represent expressions from the other array at the same index one to one.
	 * The value represent the index of the expression that owns this expression element.
	 */
	UPROPERTY()
	TArray<int32> ParentIndexMapping;

	/**
	 * Whether this container is intended to store and evaluate conditions.
	 * This fundamentally changes evaluation and editor UI behaviour.
	 * If enabled, function expressions can be negated and only functions with bool return value are allowed.
	 */
	UPROPERTY()
	bool bIsConditionContainer;

	// #todo: container change event?

	/** Delegate called after container references were set for all expressions in the tree. */
	FSimpleMulticastDelegate FixupContainerReferencesEvent;
};

/** Custom struct traits of FActorIOExpressionContainer. */
template<>
struct TStructOpsTypeTraits<FActorIOExpressionContainer> : public TStructOpsTypeTraitsBase2<FActorIOExpressionContainer>
{
	enum
	{
		WithSerializer = true,
		WithPostSerialize = true,
		WithImportTextItem = true
	};
};
