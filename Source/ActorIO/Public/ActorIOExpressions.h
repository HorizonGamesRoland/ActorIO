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
	virtual bool Evaluate(UObject* Executor, FString& OutResult) { return false; }

	virtual bool ExportText(FString& Str) const { return false; }
	virtual bool ImportText(const FString& Str, int32 Version) { return false; }

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
	virtual bool Evaluate(UObject* Executor, FString& OutResult) override;
	virtual bool ExportText(FString& Str) const override;
	virtual bool ImportText(const FString& Str, int32 Version) override;

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

	virtual bool ImportArgumentsText(const FString& Str, int32 Version);
	virtual bool ExportArgumentsText(FString& Str) const;

	UPROPERTY()
	bool bNegated;
};

USTRUCT()
struct ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
	GENERATED_BODY()

public:

	virtual FName GetTypeName() const override { return FName("KismetFunction"); }
	virtual bool Evaluate(UObject* Executor, FString& OutResult) override;
	virtual bool ExportText(FString& Str) const override;
	virtual bool ImportText(const FString& Str, int32 Version) override;

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
	virtual bool Evaluate(UObject* Executor, FString& OutResult) override;
	virtual bool ExportText(FString& Str) const override;
	virtual bool ImportText(const FString& Str, int32 Version) override;
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionContainer
{
	GENERATED_BODY()

public:

	FActorIOExpressionContainer();

	int32 AddExpression(const TInstancedStruct<FActorIOExpressionBase>& Expr, int32 ParentIdx);

	void RemoveExpression(int32 ExprIdx, bool bRemoveChilds = true);

	void RemoveChildExpressions(int32 ExprIdx);

	void Empty();

	TArray<TInstancedStruct<FActorIOExpressionBase>>& GetExpressions() { return Expressions; }
	int32 GetNumExpressions() const { return Expressions.Num(); }

	int32 GetExpressionIdx(const FActorIOExpressionBase* InExpr) const;
	int32 GetExpressionParentIdx(const FActorIOExpressionBase* InExpr) const;

	TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) { return Expressions[Idx]; }
	const TInstancedStruct<FActorIOExpressionBase>& GetExpression(int32 Idx) const { return Expressions[Idx]; }

	TArray<FActorIOExpressionBase*> GetChildExpressions(int32 ExprIdx);
	TArray<int32> GetChildExpressionIdxs(int32 ExprIdx);

	FActorIOExpressionBase* GetRootExpression();

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

	static FActorIOExpressionBase* NewExpressionFromString(const FString& Str, int32 Version = INDEX_NONE);

	static FActorIOExpressionBase* NewExpressionOfType(FName TypeName);

	static bool ParseBracket(const FString& Str, FString& OutPrefix, FString& OutData);
};
