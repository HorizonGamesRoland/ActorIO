// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "StructUtils/InstancedStruct.h"
#include "Templates/SubclassOf.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActorIOExpressions.generated.h"

/** Valid expression archetypes. */
UENUM()
enum class EActorIOExpressionType : uint8
{
	Invalid,
	Literal,
	Function,
	Group
};

/**
 * Base class for an actor I/O expression, to be wrapped in a TInstancedStruct<>.
 * Each expression can evaluate to a string value at runtime.
 * Can contain any number of child expressions.
 * Expressions exist within an ActorIOExpressionContainer where the actual tree structure lives.
 */
USTRUCT()
struct ACTORIO_API FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	/** Default constructor and virtual destructor for inheritance. */
	FActorIOExpressionBase();
	virtual ~FActorIOExpressionBase() = default;

	/** @return The archetype of the expression. */
	virtual EActorIOExpressionType GetType() const { return EActorIOExpressionType::Invalid; }

	/** @return Type name used to differentiate the same type of expressions. */
	virtual FName GetTypeName() const { return NAME_None; }

	/**
	 * Initializer called by the ActorIOExpressionContainer when the expression is added to the tree.
	 * At this point the expression container is set, and child expressions can be created.
	 * 
	 * @return If false, the expression will be immediately removed from the expression tree.
	 */
	virtual bool Init() { return true; }

	/**
	 * Initializer called by the ActorIOExpressionContainer when the expression is added to the tree by string.
	 * Called after Init().
	 *
	 * @return If false, the expression will be immediately removed from the expression tree.
	 */
	virtual bool InitFromString(const FString& Str) { return false; }

	/** @return String representation of the expression. */
	virtual FString ToString() const { return TEXT(""); }

	/**
	 * Whether the expression can contain child expressions.
	 * Not a hard limitation, used mostly by the editor UI.
	 */
	virtual bool CanContainChildExpressions() const { return false; }

	/**
	 * Whether child expressions can be added/removed by the user.
	 * If true, child expressions can be edited manually (e.g. expression group).
	 * If false, child expressions are managed by this expression (e.g. function parameters).
	 * Not a hard limitation, used mostly by the editor UI.
	 */
	virtual bool CanEditChildExpressions() const { return false; }

	/**
	 * Initializes child expressions so that they match the given string.
	 * Generates expressions from the ground up, previous childs are removed.
	 */
	virtual bool InitChildExpressionsFromString(const FString& Str) { return false; }

	/** @return String representation of child expressions, delimited by ',' */
	virtual FString ChildExpressionToString() const { return TEXT(""); }

	/**
	 * Evaluates the expression to a string value.
	 * The out string value is the actual result of the expression.
	 * 
	 * @return True if the expression was evaluated and the string value is valid. False otherwise.
	 */
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) { return false; }

	/** Assign the expression container that owns this expression. */
	void SetContainer(FActorIOExpressionContainer* InContainer) { ParentContainer = InContainer; }

	/** @return The expression container that owns this expression. */
	FActorIOExpressionContainer* GetContainer() const { return ParentContainer; }

	/**
	 * Adds a metadata to this expression.
	 * If the given metadata already exists, its value is overridden.
	 */
	void SetMetadata(const FString& InName, const FString& InValue);

	/** Removes the given metadata from the expression. */
	bool RemoveMetadata(const FString& InName);

	/** Removes all metadata from the expression. */
	void ClearMetadata();

	/** @return Whether the given metadata exists on this expression. */
	bool HasMetadata(const FString& InName) const;

	/** Gets the value of the given metadata. */
	bool GetMetadata(const FString& InName, FString& OutValue) const;

	/** @return The entire metadata string of the expression. */
	const FString& GetMetadataString() const { return Metadata; }

	/**
	 * Validates the expression and returns an error code if any issues are found.
	 * Called automatically by the editor UI, and during Map Checks.
	 */
	virtual FString CheckForErrors() const { return TEXT(""); }

private:

	/**
	 * The expression container that owns this expression.
	 * Using a raw pointer on purpose as reflection doesn't allow struct pointers.
	 * Since the lifetime of an expression is tied to its owning container, this reference should always be valid.
	 * @see FActorIOExpressionContainer::FixupContainerReferences
	 */
	FActorIOExpressionContainer* ParentContainer;

	/** List of Key=Value pairs for storing contextual information about this expression. */
	UPROPERTY()
	FString Metadata;
};

/**
 * Expression that evaluates to a literal value.
 */
USTRUCT()
struct ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Literal; }
	virtual FName GetTypeName() const override { return FName("Literal"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;
	//~ End FActorIOExpressionBase Interface

	/** Set the literal value of the expression. */
	void SetLiteralValue(const FString& InValue) { LiteralValue = InValue; }

	/** @return The literal value of the expression. */
	const FString& GetLiteralValue() const { return LiteralValue; }

protected:

	/** String value of the expression. */
	UPROPERTY()
	FString LiteralValue;
};

/**
 * Base class for an expression that executes a function.
 * These expressions can be negated to invert the result / functionality of the expression.
 * The actual negation behaviour is defined per expression class.
 */
USTRUCT()
struct ACTORIO_API FActorIOFunctionExpressionBase: public FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	FActorIOFunctionExpressionBase();

	//~ Begin FActorIOExpressionBase Interface
	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Function; }
	virtual bool CanContainChildExpressions() const override { return true; }
	virtual bool InitChildExpressionsFromString(const FString& Str) override;
	virtual FString ChildExpressionToString() const override;
	//~ End FActorIOExpressionBase Interface

	/** Set whether the expression is negated. */
	void SetNegated(bool bEnabled) { bNegated = bEnabled; }

	/** @return Whether the expression is negated. */
	bool IsNegated() const { return bNegated; }

protected:

	/** Whether the expression is negated. */
	UPROPERTY()
	bool bNegated;
};

/**
 * Expression that executes a UFunction from a BlueprintFunctionLibrary and returns its result.
 * Child expressions map to the parameters of the selected UFunction.
 * If negated, UFunction with bool return value will be inverted.
 */
USTRUCT()
struct ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual FName GetTypeName() const override { return FName("KismetFunction"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;
	virtual FString CheckForErrors() const override;
	//~ End FActorIOExpressionBase Interface

	/** Set the class that contains the function. */
	void SetFunctionClass(UClass* InClassPtr, bool bResetFunctionName = false);

	/** Set the name of the function to execute. */
	void SetFunctionName(FName InFunctionName);

	/** @return Class that contains the function. */
	const UClass* GetFunctionClass() const { return FunctionClass.Get(); }

	/** @return Name of the function to execute. */
	FName GetFunctionName() const { return FunctionName; }

	/** @return UFunction that will be executed by the expression. */
	UFunction* ResolveUFunction() const;

	/** @return Whether child expressions map to the parameters of the selected UFunction. */
	bool CheckArgumentsNeedUpdate() const;

	/**
	 * Updates child expressions to match the parameters of the selected UFunction.
	 * This will add/remove child expressions to match the parameter count.
	 * Also handles cases where the UFunction's parameter order/type has changed.
	 */
	void UpdateArguments();

protected:

	/** The class that contains the function. */
	UPROPERTY()
	TSubclassOf<UBlueprintFunctionLibrary> FunctionClass;

	/** Name of the function to execute. */
	UPROPERTY()
	FName FunctionName;
};

/** Valid comparison types for FActorIOCompareExpression. */
UENUM()
enum class EActorIOComparisonOp : uint8
{
	Less,
	LessOrEqual,
	Equal,
	NotEqual,
	GreaterOrEqual,
	Greater,
};

/**
 * Expression that compares two values.
 * If negated, inverts the result of the comparison.
 */
USTRUCT()
struct ACTORIO_API FActorIOCompareExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	FActorIOCompareExpression();

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual FName GetTypeName() const override { return FName("Compare"); }
	virtual bool Init() override;
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;
	virtual FString CheckForErrors() const override;
	//~ End FActorIOExpressionBase Interface

	/** Set the operator to use when comparing the values. */
	void SetComparisonOp(const EActorIOComparisonOp& Op) { ComparisonOp = Op; }

	/** @return The operator to use when comparing the values. */
	const EActorIOComparisonOp& GetComparisonOp() const { return ComparisonOp; }

	/** Convert an operator to string. */
	static FString OpToKeyword(const EActorIOComparisonOp& Op);

	/** Convert a string into an operator. */
	static TOptional<EActorIOComparisonOp> OpFromKeyword(const FString& Keyword);

	/** @return Whether child expressions are exactly two. */
	bool CheckArgumentCountValid() const;

protected:

	/** The operator to use when comparing the values. */
	UPROPERTY()
	EActorIOComparisonOp ComparisonOp;
};

/**
 * Expression that evaluates a list of child expressions sequentially.
 * If the owning container is storing conditions, childs are evaluated in an AND/OR relation based on negation.
 */
USTRUCT()
struct ACTORIO_API FActorIOGroupExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	/** Makes a new instance of this expression. */
	static TInstancedStruct<FActorIOExpressionBase> MakeInstance(const FString& Keyword);

	//~ Begin FActorIOExpressionBase Interface
	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Group; }
	virtual FName GetTypeName() const override { return FName("Group"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool CanEditChildExpressions() const override { return true; }
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;
	//~ End FActorIOExpressionBase Interface
};

/**
 * Container for storing expressions in a custom flat tree structure.
 * The tree consists of two arrays, storing expression instances and their parent relations separately.
 * Expressions can have any number of children, but some expressions may apply limitations.
 * There is only ever one root expression in the tree with its parent index being -1.
 * Elements of the tree are ordered, ensuring child expressions are always below their parents and the root is always at the top.
 * 
 * This complicated tree structure was necessary due to Unreal's property system not supporting nested properties with the same names.
 * @see PropertyHandleImp::LogAmbiguousArrayIndexWarning
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
	int32 GetNumChildExpressions(int32 ExprIdx, bool bRecursive = false) const; // #todo: check bRecursive

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
	 * The EditAnywhere tag is needed because the editor UI needs pointers to this property.
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

/**
 * Helper functions for parsing actor I/O expressions.
 */
class ACTORIO_API FActorIOExpressionParser
{
public:

	/**
	 * Create a new expression instance from the given keyword.
	 * This is the short name of the expression when initializing from string (val, and, or, etc...)
	 * Does not call the initializer, that will be done by the expression container.
	 */
	static TInstancedStruct<FActorIOExpressionBase> NewExpressionFromKeyword(FString Keyword);

	/**
	 * Parse the string for the next bracket, breaking the string into prefix and data.
	 * Example: val("Hello World")
	 * Prefix:  val
	 * Data:    "Hello World"
	 */
	static bool ParseBracket(const FString& Str, FString& OutPrefix, FString& OutData);
};
