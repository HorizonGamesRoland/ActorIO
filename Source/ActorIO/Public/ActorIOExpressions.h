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
	Function
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
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) { return false; }

	virtual bool InitFromString(const FString& Str) { return false; }
	virtual FString ToString() const { return TEXT(""); }

	void SetContainer(FActorIOExpressionContainer* InContainer) { ParentContainer = InContainer; }
	FActorIOExpressionContainer* GetContainer() const { return ParentContainer; }

private:

	FActorIOExpressionContainer* ParentContainer;
};

USTRUCT()
struct ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
	GENERATED_BODY()

public:

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Literal; }
	virtual FName GetTypeName() const override { return FName("Literal"); }
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;

	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;

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

	void SetNegated(bool bEnabled) { bNegated = bEnabled; }
	bool IsNegated() const { return bNegated; }

protected:

	virtual bool InitChildExpressionsFromString(const FString& Str);

	virtual FString ChildExpressionToString() const;

	UPROPERTY()
	bool bNegated;
};

USTRUCT()
struct ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	virtual FName GetTypeName() const override { return FName("KismetFunction"); }
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;

	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;

	void SetFunctionClass(UClass* InClassPtr);
	void SetFunctionName(FName InFunctionName);

	const UClass* GetFunctionClass() const { return FunctionClass.Get(); }
	FName GetFunctionName() const { return FunctionName; }
	UFunction* ResolveUFunction() const;

protected:

	void UpdateArguments();

	UPROPERTY()
	TSubclassOf<UBlueprintFunctionLibrary> FunctionClass;

	UPROPERTY()
	FName FunctionName;
};

USTRUCT()
struct ACTORIO_API FActorIOGroupExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	virtual FName GetTypeName() const override { return FName("Group"); }
	virtual bool Evaluate(UObject* Executor, FString& OutReturnValue) override;

	virtual bool InitFromString(const FString& Str) override;
	virtual FString ToString() const override;
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionContainer
{
	GENERATED_BODY()

public:

	FActorIOExpressionContainer();

	int32 AddExpression(const TInstancedStruct<FActorIOExpressionBase>& Expr, int32 ParentIdx);

	bool InitFromString(const FString& Str);

	FString ToString() const;

	bool NewExpressionFromString(const FString& Str, int32 ParentIdx);

	void RemoveExpression(int32 ExprIdx);

	void RemoveChildExpressions(int32 ExprIdx);

	void Empty();

	TArray<TInstancedStruct<FActorIOExpressionBase>>& GetExpressions() { return Expressions; }
	int32 GetNumExpressions() const { return Expressions.Num(); }

	int32 GetExpressionIdx(const FActorIOExpressionBase* InExpr) const;
	int32 GetExpressionParentIdx(const FActorIOExpressionBase* InExpr) const;

	TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) { return Expressions[Idx]; }
	const TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) const { return Expressions[Idx]; }

	FActorIOExpressionBase* GetExpressionPtr(int32 Idx);
	const FActorIOExpressionBase* GetExpressionPtr(int32 Idx) const;

	TArray<FActorIOExpressionBase*> GetChildExpressions(int32 ExprIdx);
	TArray<const FActorIOExpressionBase*> GetChildExpressions(int32 ExprIdx) const;
	TArray<int32> GetChildExpressionIdxs(int32 ExprIdx) const;

	FActorIOExpressionBase* GetRootExpression();
	const FActorIOExpressionBase* GetRootExpression() const;
	int32 GetRootExpressionIdx() const;

	void FixupContainerReferences();
	FSimpleMulticastDelegate& OnFixupContainerReferences() { return FixupContainerReferencesEvent; }

	void SetIsConditionContainer(bool bEnabled) { bIsConditionContainer = bEnabled; }
	bool IsConditionContainer() const { return bIsConditionContainer; }

	bool Evaluate(UObject* Executor);

	//bool operator==(const FActorIOScriptCondition& Other) const;
	//bool operator!=(const FActorIOScriptCondition& Other) const { return !(*this == Other); }

	bool Serialize(FArchive& Ar);
	void PostSerialize(const FArchive& Ar);
	//bool ExportTextItem(FString& ValueStr, FActorIOScriptCondition const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;
	//bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);

protected:

	UPROPERTY(EditAnywhere)
	TArray<TInstancedStruct<FActorIOExpressionBase>> Expressions;

	UPROPERTY()
	TArray<int32> ParentIndexMapping;

	UPROPERTY()
	bool bIsConditionContainer;

	FSimpleMulticastDelegate FixupContainerReferencesEvent;
};

template<>
struct TStructOpsTypeTraits<FActorIOExpressionContainer> : public TStructOpsTypeTraitsBase2<FActorIOExpressionContainer>
{
	enum
	{
		WithSerializer = true,
		WithPostSerialize = true
		//WithExportTextItem = true,
		//WithImportTextItem = true,
		//WithIdenticalViaEquality = true - disabled to avoid users bricking their maps
	};
};

class ACTORIO_API FActorIOExpressionParser
{
public:

	static TInstancedStruct<FActorIOExpressionBase> NewExpressionOfType(FString TypeStr);

	static bool ParseBracket(const FString& Str, FString& OutPrefix, FString& OutData);
};
