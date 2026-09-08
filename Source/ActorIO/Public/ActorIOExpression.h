// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "UObject/WeakObjectPtr.h"
#include "ActorIOExpression.generated.h"

UENUM()
enum class EActorIOExpressionType : uint8
{
	Invalid,
	Literal,
	Function
};

class ACTORIO_API FActorIOExpressionBase
{
public:

	FActorIOExpressionBase();
	virtual ~FActorIOExpressionBase() = default;

	virtual EActorIOExpressionType GetType() const { return EActorIOExpressionType::Invalid; }
	virtual FName GetTypeName() const { return NAME_None; }
	virtual bool Evaluate(FString& OutResult) { return false; }

	virtual bool ExportText(FString& Str) const { return false; }
	virtual bool ImportText(const FString& Str, int32 Version) { return false; }

	void SetParent(FActorIOExpressionBase* InExpr) { ParentExpr = InExpr; }
	FActorIOExpressionBase* GetParent() const { return ParentExpr; }

	FActorIOExpressionBase* GetRootExpression();
	bool IsRootExpression() const { return ParentExpr != nullptr; }

private:

	FActorIOExpressionBase* ParentExpr;
};

class ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
public:

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Literal; }
	virtual FName GetTypeName() const override { return FName("Literal"); }
	virtual bool Evaluate(FString& OutResult) override;
	virtual bool ExportText(FString& Str) const override;
	virtual bool ImportText(const FString& Str, int32 Version) override;

	void SetLiteralValue(const FString& InValue) { LiteralValue = InValue; }
	const FString& GetLiteralValue() const { return LiteralValue; }

protected:

	FString LiteralValue;
};

class ACTORIO_API FActorIOFunctionExpressionBase: public FActorIOExpressionBase
{
public:

	FActorIOFunctionExpressionBase();
	virtual ~FActorIOFunctionExpressionBase();

	virtual EActorIOExpressionType GetType() const override { return EActorIOExpressionType::Function; }

	void AddArgument(FActorIOExpressionBase* InExpr);
	void RemoveArgument(FActorIOExpressionBase* InExpr);
	void SetArgumentAt(int32 Index, FActorIOExpressionBase* InExpr);
	void ResetArguments();

	const TArray<FActorIOExpressionBase*>& GetArguments() const { return Args; }
	FActorIOExpressionBase* GetArgumentAt(int32 Index) const;
	int32 GetNumArguments(bool bRecursive = false) const;

	void SetNegated(bool bEnabled) { bNegated = bEnabled; }
	bool IsNegated() const { return bNegated; }

protected:

	virtual bool ImportArgumentsText(const FString& Str, int32 Version);
	virtual bool ExportArgumentsText(FString& Str) const;

	TArray<FActorIOExpressionBase*> Args;

	bool bNegated;
};

class ACTORIO_API FActorIOKismetFunctionExpression : public FActorIOFunctionExpressionBase
{
public:

	virtual FName GetTypeName() const override { return FName("KismetFunction"); }
	virtual bool Evaluate(FString& OutResult) override;
	virtual bool ExportText(FString& Str) const override;
	virtual bool ImportText(const FString& Str, int32 Version) override;

	void SetFunctionClass(UClass* InClassPtr);
	void SetFunctionId(FName InFunctionId);

	const TWeakObjectPtr<UClass>& GetClass() const { return ClassPtr; }
	FName GetFunctionId() const { return FunctionId; }
	UFunction* GetUFunction();
	TArray<FProperty*> GetUFunctionParams();

protected:

	void UpdateArguments();

	TWeakObjectPtr<UClass> ClassPtr;

	FName FunctionId;
};

class ACTORIO_API FActorIOGroupExpression : public FActorIOFunctionExpressionBase
{
public:

	virtual FName GetTypeName() const override { return FName("Group"); }
	virtual bool Evaluate(FString& OutResult) override;
	virtual bool ExportText(FString& Str) const override;
	virtual bool ImportText(const FString& Str, int32 Version) override;
};

USTRUCT()
struct ACTORIO_API FActorIOScriptCondition
{
	GENERATED_BODY()

public:
	
	FActorIOScriptCondition();
	~FActorIOScriptCondition();

	bool operator==(const FActorIOScriptCondition& Other) const;
	bool operator!=(const FActorIOScriptCondition& Other) const { return !(*this == Other); }

	bool Serialize(FArchive& Ar);
	bool ExportTextItem(FString& ValueStr, FActorIOScriptCondition const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;
	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);

	FActorIOGroupExpression* GetExpression() const { return Expr; }

protected:

	FActorIOGroupExpression* Expr;
};

template<>
struct TStructOpsTypeTraits<FActorIOScriptCondition> : public TStructOpsTypeTraitsBase2<FActorIOScriptCondition>
{
	enum
	{
		WithSerializer = true,
		WithExportTextItem = true,
		WithImportTextItem = true,
		//WithIdenticalViaEquality = true - disabled to avoid users bricking their maps
	};
};

class ACTORIO_API FActorIOExpresionParser
{
public:

	static FActorIOExpressionBase* NewExpressionFromString(const FString& Str, int32 Version = INDEX_NONE);

	static FActorIOExpressionBase* NewExpressionOfType(FName TypeName);

	static bool ParseBracket(const FString& Str, FString& OutPrefix, FString& OutData);
};
