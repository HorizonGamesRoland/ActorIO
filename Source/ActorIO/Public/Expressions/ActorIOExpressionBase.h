// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "StructUtils/InstancedStruct.h"
#include "ActorIOExpressionBase.generated.h"

struct FActorIOExpressionContainer;

/**
 * Context for evaluating I/O expressions.
 * Stores the executing actor and a list of named arguments with their values.
 * Automatically created when evaluating an expression container.
 */
USTRUCT()
struct ACTORIO_API FExpressionEvaluationContext
{
	GENERATED_BODY()

	/** Reference to the requesting object. */
	UPROPERTY()
	TObjectPtr<UObject> Executor;

	/**
	 * List of named arguments and their corresponding values.
	 * Do not modify directly. Use SetNamedArgument() instead.
	 */
	UPROPERTY()
	TMap<FString, FString> NamedArguments;

	/** Default constructor. */
	FExpressionEvaluationContext() :
		Executor(nullptr),
		NamedArguments(TMap<FString, FString>())
	{}

	/**
	 * Add a named argument (parameter) to the current execution context.
	 * If it already exists then the value is simply updated.
	 */
	void SetNamedArgument(const FString& InName, const FString& InValue);
};

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
	 * Initializes child expressions so that they match the given string.
	 * Generates expressions from the ground up, previous childs are removed.
	 */
	virtual bool InitChildExpressionsFromString(const FString& Str) { return false; }

	/** @return String representation of child expressions, delimited by ',' */
	virtual FString ChildExpressionsToString() const { return TEXT(""); }

	/**
	 * Evaluates the expression to a string value.
	 * The out string value is the actual result of the expression.
	 * 
	 * @return True if the expression was evaluated and the string value is valid. False otherwise.
	 */
	virtual bool Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue) { return false; }

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

	/** List of space delimited Key=Value pairs for storing contextual information about this expression. */
	UPROPERTY()
	FString Metadata;
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
	virtual FString ChildExpressionsToString() const override;
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
