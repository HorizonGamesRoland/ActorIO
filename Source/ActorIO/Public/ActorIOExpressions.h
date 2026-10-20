// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "StructUtils/InstancedStruct.h"
#include "Templates/SubclassOf.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActorIOExpressions.generated.h"

UENUM()
enum class EActorIOExpressionType : uint8
{
	Invalid,
	Literal,
	Function,
	Group
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	FActorIOExpressionBase();
	virtual ~FActorIOExpressionBase() = default;

	virtual EActorIOExpressionType GetType() const { return EActorIOExpressionType::Invalid; }
	virtual FName GetTypeName() const { return NAME_None; }

	virtual bool Init() { return true; }
	virtual bool InitFromString(const FString& Str) { return false; }
	virtual FString ToString() const { return TEXT(""); }

	virtual bool CanContainChildExpressions() const { return false; }
	virtual bool CanEditChildExpressions() const { return false; }
	virtual bool InitChildExpressionsFromString(const FString& Str) { return false; }
	virtual FString ChildExpressionToString() const { return TEXT(""); }

	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) { return false; }

	void SetContainer(FActorIOExpressionContainer* InContainer) { ParentContainer = InContainer; }
	FActorIOExpressionContainer* GetContainer() const { return ParentContainer; }

	void SetMetadata(const FString& InName, const FString& InValue);
	bool RemoveMetadata(const FString& InName);
	void ClearMetadata();
	bool HasMetadata(const FString& InName) const;
	bool GetMetadata(const FString& InName, FString& OutValue) const;
	const FString& GetMetadataString() const { return Metadata; }

	virtual FString CheckForErrors() const { return TEXT(""); }

private:

	FActorIOExpressionContainer* ParentContainer;

	UPROPERTY()
	FString Metadata;
};

USTRUCT()
struct ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Literal; }
	virtual FName GetTypeName() const override { return FName("Literal"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;

	void SetLiteralValue(const FString& InValue) { LiteralValue = InValue; }
	const FString& GetLiteralValue() const { return LiteralValue; }

protected:

	UPROPERTY()
	FString LiteralValue;
};

USTRUCT()
struct ACTORIO_API FActorIOFunctionExpressionBase: public FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	FActorIOFunctionExpressionBase();

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Function; }
	virtual bool CanContainChildExpressions() const override { return true; }
	virtual bool InitChildExpressionsFromString(const FString& Str) override;
	virtual FString ChildExpressionToString() const override;

	void SetNegated(bool bEnabled) { bNegated = bEnabled; }
	bool IsNegated() const { return bNegated; }

protected:

	UPROPERTY()
	bool bNegated;
};

USTRUCT()
struct ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	virtual FName GetTypeName() const override { return FName("KismetFunction"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;
	virtual FString CheckForErrors() const override;

	void SetFunctionClass(UClass* InClassPtr, bool bResetFunctionName = false);
	void SetFunctionName(FName InFunctionName);

	const UClass* GetFunctionClass() const { return FunctionClass.Get(); }
	FName GetFunctionName() const { return FunctionName; }
	UFunction* ResolveUFunction() const;

	bool CheckArgumentsNeedUpdate() const;

	void UpdateArguments();

protected:

	UPROPERTY()
	TSubclassOf<UBlueprintFunctionLibrary> FunctionClass;

	UPROPERTY()
	FName FunctionName;
};

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

USTRUCT()
struct ACTORIO_API FActorIOCompareExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	FActorIOCompareExpression();

	virtual FName GetTypeName() const override { return FName("Compare"); }
	virtual bool Init() override;
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;
	virtual FString CheckForErrors() const override;

	void SetComparisonOp(const EActorIOComparisonOp& Op) { ComparisonOp = Op; }
	const EActorIOComparisonOp& GetComparisonOp() const { return ComparisonOp; }

	static FString OpToTypeString(const EActorIOComparisonOp& Op);
	static TOptional<EActorIOComparisonOp> OpFromTypeString(const FString& Str);

	bool CheckArgumentCountValid() const;

protected:

	UPROPERTY()
	EActorIOComparisonOp ComparisonOp;
};

USTRUCT()
struct ACTORIO_API FActorIOGroupExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Group; }
	virtual FName GetTypeName() const override { return FName("Group"); }
	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
	virtual bool CanEditChildExpressions() const override { return true; }
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionContainer
{
	GENERATED_BODY()

public:

	FActorIOExpressionContainer();

	bool InitFromString(const FString& Str);

	FString ToString() const;

	int32 AddExpression(const TInstancedStruct<FActorIOExpressionBase>& Expr, int32 ParentIdx, int32 SlotIdx = INDEX_NONE);

	int32 AddExpressionFromString(const FString& Str, int32 ParentIdx, int32 SlotIdx = INDEX_NONE);

	void RemoveExpression(int32 ExprIdx);

	void RemoveChildExpressions(int32 ExprIdx);

	void Empty();

	int32 MoveExpressionTo(int32 ExprIdx, int32 ParentIdx, int32 SlotIdx);

	void MoveExpressionTo_OLD(int32 ExprIdx, int32 ParentIdx, int32 SlotIdx); // #todo: debug, remove?

	TArray<TInstancedStruct<FActorIOExpressionBase>>& GetExpressions() { return Expressions; }
	const TArray<TInstancedStruct<FActorIOExpressionBase>>& GetExpressions() const { return Expressions; }
	int32 GetNumExpressions() const { return Expressions.Num(); }

	int32 GetExpressionIdx(const FActorIOExpressionBase* InExpr) const;
	int32 GetExpressionParentIdx(int32 Idx) const;
	int32 GetExpressionSlotIdx(int32 Idx) const;

	bool IsValidIndex(int32 Idx) const;

	TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) { return Expressions[Idx]; }
	const TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) const { return Expressions[Idx]; }

	FActorIOExpressionBase* GetExpressionPtr(int32 Idx);
	const FActorIOExpressionBase* GetExpressionPtr(int32 Idx) const;

	TArray<FActorIOExpressionBase*> GetChildExpressions(int32 ExprIdx);
	TArray<const FActorIOExpressionBase*> GetChildExpressions(int32 ExprIdx) const;
	TArray<int32> GetChildExpressionIdxs(int32 ExprIdx) const;
	int32 GetNumChildExpressions(int32 ExprIdx) const; // #todo: add recursive mode

	bool IsExpressionDescendantOf(int32 ExprIdx, int32 AncestorIdx) const;

	FActorIOExpressionBase* GetRootExpression(); // #todo: make it just return first element and ensure
	const FActorIOExpressionBase* GetRootExpression() const;
	int32 GetRootExpressionIdx() const;

	void FixupContainerReferences();
	FSimpleMulticastDelegate& OnFixupContainerReferences() { return FixupContainerReferencesEvent; }

	void SetIsConditionContainer(bool bEnabled) { bIsConditionContainer = bEnabled; }
	bool IsConditionContainer() const { return bIsConditionContainer; }

	bool Evaluate(UObject* Executor);

	bool Serialize(FArchive& Ar);
	void PostSerialize(const FArchive& Ar);
	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);

	bool HasAnyErrors() const;

	void DebugDumpContainerData() const;

protected:

	UPROPERTY(EditAnywhere)
	TArray<TInstancedStruct<FActorIOExpressionBase>> Expressions;

	UPROPERTY()
	TArray<int32> ParentIndexMapping;

	UPROPERTY()
	bool bIsConditionContainer;

	// #todo: container change event?

	FSimpleMulticastDelegate FixupContainerReferencesEvent;
};

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

class ACTORIO_API FActorIOExpressionParser
{
public:

	static TInstancedStruct<FActorIOExpressionBase> NewExpressionOfType(FString TypeStr);

	static bool ParseBracket(const FString& Str, FString& OutPrefix, FString& OutData);
};
