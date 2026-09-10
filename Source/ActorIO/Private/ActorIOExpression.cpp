// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpression.h"
#include "ActorIOSubsystemBase.h"
#include "ActorIOVersions.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceNull.h"

//=======================================================
//~ Begin FActorIOExpressionBase
//=======================================================

FActorIOExpressionBase::FActorIOExpressionBase()
{
	ParentExpr = nullptr;
	bIsCondition = false;
}

FActorIOExpressionBase* FActorIOExpressionBase::GetRootExpression()
{
	FActorIOExpressionBase* RootExpr = this;
	while (true)
	{
		FActorIOExpressionBase* Parent = RootExpr->GetParent();
		if (!Parent)
		{
			break;
		}

		RootExpr = Parent;
	}

	return RootExpr;
}

//=======================================================
//~ Begin FActorIOLiteralExpression
//=======================================================

bool FActorIOLiteralExpression::Evaluate(UObject* Executor, FString& OutResult)
{
	OutResult = LiteralValue;
	return true;
}

bool FActorIOLiteralExpression::ExportText(FString& Str) const
{
	Str = FString::Printf(TEXT("val(%s)"), *LiteralValue);
	return true;
}

bool FActorIOLiteralExpression::ImportText(const FString& Str, int32 Version)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpressionParser::ParseBracket(Str, Prefix, Data))
	{
		return false;
	}

	if (Prefix != TEXT("val"))
	{
		return false;
	}

	LiteralValue = Data;
	return true;
}

//=======================================================
//~ Begin FActorIOFunctionExpressionBase
//=======================================================

FActorIOFunctionExpressionBase::FActorIOFunctionExpressionBase()
{
	bNegated = false;
}

FActorIOFunctionExpressionBase::~FActorIOFunctionExpressionBase()
{
	Args.Empty();
}

void FActorIOFunctionExpressionBase::AddArgument(FActorIOExpressionBase* InExpr)
{
	if (InExpr)
	{
		InExpr->SetParent(this);
		InExpr->SetIsCondition(bIsCondition);
		Args.Add(InExpr);
	}
}

void FActorIOFunctionExpressionBase::RemoveArgument(FActorIOExpressionBase* InExpr)
{
	if (InExpr)
	{
		Args.Remove(InExpr);
		delete InExpr;
	}
}

void FActorIOFunctionExpressionBase::SetArgumentAt(int32 Index, FActorIOExpressionBase* InExpr)
{
	if (InExpr && Args.IsValidIndex(Index) && Args[Index] != InExpr)
	{
		if (Args[Index] != nullptr)
		{
			delete Args[Index];
			Args[Index] = nullptr;
		}

		InExpr->SetParent(this);
		InExpr->SetIsCondition(bIsCondition);
		Args[Index] = InExpr;
	}
}

void FActorIOFunctionExpressionBase::ResetArguments()
{
	// #todo: check if element destructors are called properly
	Args.Empty();
}

bool FActorIOFunctionExpressionBase::ImportArgumentsText(const FString& Str, int32 Version)
{
	TArray<FString> ArgDatas;
	if (!Str.IsEmpty())
	{
		const TCHAR* Stream = *Str;

		FString NewData;
		int32 BracketDepth = 0;
		while (*Stream)
		{
			if (BracketDepth == 0 && *Stream == TEXT(','))
			{
				ArgDatas.Add(NewData);
				NewData.Empty();
			}
			else
			{
				NewData += *Stream;
				if (*Stream == TEXT('('))
				{
					BracketDepth++;
				}
				else if (*Stream == TEXT(')'))
				{
					BracketDepth--;
				}
			}

			Stream++;
		}

		UE_CLOG(BracketDepth != 0, LogActorIO, Error, TEXT("Unbalanced bracket count when importing arguments: %s"), *Str);
		if (BracketDepth == 0 && !NewData.IsEmpty())
		{
			ArgDatas.Add(NewData);
		}
	}

	ResetArguments();
	for (const FString& ArgData : ArgDatas)
	{
		FActorIOExpressionBase* NewExpr = FActorIOExpressionParser::NewExpressionFromString(ArgData, Version);
		if (NewExpr)
		{
			AddArgument(NewExpr);
		}
	}

	return true;
}

bool FActorIOFunctionExpressionBase::ExportArgumentsText(FString& Str) const
{
	for (FActorIOExpressionBase* Arg : Args)
	{
		if (!Arg) continue;
		if (!Str.IsEmpty()) Str += TEXT(',');

		FString ArgData;
		if (!Arg->ExportText(ArgData))
		{
			return false;
		}

		Str += ArgData;
	}

	return true;
}

FActorIOExpressionBase* FActorIOFunctionExpressionBase::GetArgumentAt(int32 Index) const
{
	if (Args.IsValidIndex(Index))
	{
		return Args[Index];
	}

	return nullptr;
}

int32 FActorIOFunctionExpressionBase::GetNumArguments(bool bRecursive) const
{
	int32 NumArgs = 0;
	for (FActorIOExpressionBase* Arg : Args)
	{
		NumArgs++;
		if (bRecursive && Arg->GetType() == EActorIOExpressionType::Function)
		{
			FActorIOFunctionExpressionBase* FunctionArg = static_cast<FActorIOFunctionExpressionBase*>(Arg);
			NumArgs += FunctionArg->GetNumArguments();
		}
	}

	return NumArgs;
}

//=======================================================
//~ Begin FActorIOKismetFunctionExpression
//=======================================================

bool FActorIOKismetFunctionExpression::Evaluate(UObject* Executor, FString& OutResult)
{
	OutResult.Empty();

	UFunction* FunctionPtr = ResolveUFunction();
	if (!FunctionPtr)
	{
		// #TODO: log error
		return false;
	}

	FString Cmd = FunctionId.ToString();
	for (FActorIOExpressionBase* Expr : Args)
	{
		FString Result;
		if (!Expr || !Expr->Evaluate(Executor, Result))
		{
			return false;
		}

		Cmd += TEXT(" ");
		Cmd += Result;
	}

	UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(Executor);
	if (!IOSubsystem)
	{
		return false;
	}

	const TFunction<void(FProperty*, uint8*)> ReturnPropertyAccessor = [&](FProperty* ReturnProperty, uint8* Container)
	{
		if (ReturnProperty)
		{
			if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(ReturnProperty))
			{
				bool bValue = BoolProperty->GetPropertyValue_InContainer(Container);
				if (bNegated)
				{
					bValue = !bValue;
				}

				OutResult = bValue ? TEXT("True") : TEXT("False");
			}
			else
			{
				ReturnProperty->ExportTextItem_InContainer(OutResult, Container, nullptr, nullptr, PPF_None);
			}
		}
	};

	FOutputDeviceNull Ar;
	return IOSubsystem->ExecuteCommand(ClassPtr.Pin()->GetDefaultObject(), *Cmd, Ar, IOSubsystem, ReturnPropertyAccessor);
}

bool FActorIOKismetFunctionExpression::ExportText(FString& Str) const
{
	FString ArgsData;
	if (!ExportArgumentsText(ArgsData))
	{
		return false;
	}

	FString FuncData;
	if (ClassPtr.IsValid())
	{
		FSoftObjectPath ClassPath = ClassPtr.Pin()->GetPathName();
		FuncData += ClassPath.ToString();
		FuncData += TEXT(':');
		FuncData += FunctionId.ToString();

		if (!ArgsData.IsEmpty())
		{
			FuncData += FString::Printf(TEXT("(%s)"), *ArgsData);
		}

		if (bNegated)
		{
			FString DataToWrap = FuncData;
			FuncData = FString::Printf(TEXT("not(%s)"), *DataToWrap);
		}
	}

	Str = FString::Printf(TEXT("func(%s)"), *FuncData);
	return true;
}

bool FActorIOKismetFunctionExpression::ImportText(const FString& Str, int32 Version)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpressionParser::ParseBracket(Str, Prefix, Data))
	{
		return false;
	}

	if (Prefix != TEXT("func"))
	{
		return false;
	}

	if (!Data.IsEmpty())
	{
		FString FuncData = Data;
		FString ArgsData;
		if (FActorIOExpressionParser::ParseBracket(FuncData, Prefix, Data))
		{
			bNegated = Prefix == TEXT("not");
			if (bNegated)
			{
				FuncData = Data;
				if (FActorIOExpressionParser::ParseBracket(FuncData, Prefix, Data))
				{
					FuncData = Prefix;
					ArgsData = Data;
				}
			}
			else
			{
				FuncData = Prefix;
				ArgsData = Data;
			}
		}

		FString ClassPathStr;
		FString FunctionIdStr;
		if (!FuncData.Split(TEXT(":"), &ClassPathStr, &FunctionIdStr))
		{
			return false;
		}

		FSoftObjectPath ClassPath = ClassPathStr;
		ClassPath.FixupCoreRedirects();

		UClass* ResolvedClass = nullptr;
		if (ClassPath.IsValid())
		{
			ResolvedClass = Cast<UClass>(ClassPath.ResolveObject());
			if (!ResolvedClass)
			{
				ResolvedClass = Cast<UClass>(ClassPath.TryLoad());
			}
		}

		UE_CLOG(ResolvedClass == nullptr, LogActorIO, Error, TEXT("%s - Could not resolve class: %s"), ANSI_TO_TCHAR(__FUNCTION__), *ClassPath.ToString())
		ClassPtr = ResolvedClass;
		FunctionId = FName(*FunctionIdStr, FNAME_Find);

		if (ResolveUFunction())
		{
			ImportArgumentsText(ArgsData, Version);
		}
	}

	return true;
}

void FActorIOKismetFunctionExpression::SetFunctionClass(UClass* InClassPtr)
{
	if (ClassPtr != InClassPtr)
	{
		ClassPtr = InClassPtr;
		UpdateArguments();
	}
}

void FActorIOKismetFunctionExpression::SetFunctionId(FName InFunctionId)
{
	if (FunctionId != InFunctionId)
	{
		FunctionId = InFunctionId;
		UpdateArguments();
	}
}

void FActorIOKismetFunctionExpression::UpdateArguments()
{
	ResetArguments();

	UFunction* FunctionPtr = ResolveUFunction();
	if (FunctionPtr)
	{
		TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(FunctionPtr);
		for (int32 ArgIdx = 0; ArgIdx != FunctionParams.Num(); ++ArgIdx)
		{
			FActorIOLiteralExpression* NewArg = new FActorIOLiteralExpression();
			AddArgument(NewArg);
		}
	}
}

UFunction* FActorIOKismetFunctionExpression::ResolveUFunction() const
{
	if (ClassPtr.IsValid() && FunctionId != NAME_None)
	{
		return ClassPtr.Pin()->FindFunctionByName(FunctionId);
	}

	return nullptr;
}

//=======================================================
//~ Begin FActorIOGroupExpression
//=======================================================

bool FActorIOGroupExpression::Evaluate(UObject* Executor, FString& OutResult)
{
	OutResult.Empty();

	// Empty groups are treated as if they are not even there.
	if (Args.IsEmpty())
	{
		OutResult = TEXT("True");
		return true;
	}

	bool bResult = true;
	for (FActorIOExpressionBase* Expr : Args)
	{
		FString Result;
		if (!Expr || !Expr->Evaluate(Executor, Result))
		{
			return false;
		}

		if (bIsCondition)
		{
			// This handles early exit cases when evaluating conditions:
			// - Result is true and we want 'ANY is true' (bNegated true), so its a pass.
			// - Result is false and we want 'ALL is true' (bNegated false), so its a fail.

			const bool bResultAsBool = FCString::ToBool(*Result);
			if (bResultAsBool == bNegated)
			{
				bResult = bResultAsBool;
				break;
			}
		}
	}

	OutResult = bResult ? TEXT("True") : TEXT("False");
	return true;
}

bool FActorIOGroupExpression::ExportText(FString& Str) const
{
	FString ArgsData;
	if (!ExportArgumentsText(ArgsData))
	{
		return false;
	}

	FString Prefix = bNegated ? TEXT("or") : TEXT("and");
	Str = FString::Printf(TEXT("%s(%s)"), *Prefix, *ArgsData);
	return true;
}

bool FActorIOGroupExpression::ImportText(const FString& Str, int32 Version)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpressionParser::ParseBracket(Str, Prefix, Data))
	{
		return false;
	}

	if (Prefix != TEXT("and") && Prefix != TEXT("or"))
	{
		return false;
	}

	bNegated = Prefix == TEXT("or");
	ImportArgumentsText(Data, Version);
	return true;
}

//=======================================================
//~ Begin FActorIOScriptCondition
//=======================================================

FActorIOScriptCondition::FActorIOScriptCondition()
{
	Expr = nullptr;
}

void FActorIOScriptCondition::Initialize()
{
	if (Expr.IsValid())
	{
		Expr.Reset();
	}

	Expr = MakeShared<FActorIOGroupExpression>();
	Expr->SetIsCondition(true);
}

bool FActorIOScriptCondition::operator==(const FActorIOScriptCondition& Other) const
{
	FActorIOGroupExpression* OtherExpr = Other.GetExpression();
	if (Expr.IsValid() && OtherExpr)
	{
		FString Data;
		FString OtherData;
		if (Expr->ExportText(Data) && OtherExpr->ExportText(OtherData))
		{
			return Data == OtherData;
		}
	}

	return false;
}

bool FActorIOScriptCondition::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FActorIOExpressionVersion::GUID);

	int32 Version = Ar.CustomVer(FActorIOExpressionVersion::GUID);
	Ar << Version;

	if (Ar.IsLoading())
	{
		Ar.SetCustomVersion(FActorIOExpressionVersion::GUID, Version, TEXT("ActorIOExpressionVer"));

		FString SavedStr;
		Ar << SavedStr;

		if (Expr.IsValid())
		{
			return Expr->ImportText(SavedStr, Version);
		}
	}
	else
	{
		FString ExprStr;
		if (Expr.IsValid())
		{
			Expr->ExportText(ExprStr);
		}

		Ar << ExprStr;
		return true;
	}

	return false;
}

bool FActorIOScriptCondition::ExportTextItem(FString& ValueStr, FActorIOScriptCondition const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
	return Expr.IsValid() && Expr->ExportText(ValueStr);
}

bool FActorIOScriptCondition::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	return Expr.IsValid() && Expr->ImportText(Buffer, (int32)FActorIOExpressionVersion::LatestVersion);
}

bool FActorIOScriptCondition::Evaluate(UObject* Executor)
{
	FString Result;
	if (Expr.IsValid() && Expr->Evaluate(Executor, Result))
	{
		return FCString::ToBool(*Result);
	}

	return false;
}

//=======================================================
//~ Begin FActorIOExpressionParser
//=======================================================

FActorIOExpressionBase* FActorIOExpressionParser::NewExpressionFromString(const FString& Str, int32 Version)
{
	if (Version == INDEX_NONE)
	{
		Version = (int32)FActorIOExpressionVersion::LatestVersion;
	}

	FString Prefix;
	FString Data;
	if (!FActorIOExpressionParser::ParseBracket(Str, Prefix, Data))
	{
		return nullptr;
	}

	FActorIOExpressionBase* NewExpr = NewExpressionOfType(FName(Prefix));
	if (NewExpr)
	{
		if (!NewExpr->ImportText(Str, Version))
		{
			delete NewExpr;
			NewExpr = nullptr;
		}
	}

	return NewExpr;
}

FActorIOExpressionBase* FActorIOExpressionParser::NewExpressionOfType(FName TypeName)
{
	if (TypeName == FName("val"))
	{
		return new FActorIOLiteralExpression();
	}
	else if (TypeName == FName("func"))
	{
		return new FActorIOKismetFunctionExpression();
	}
	else if (TypeName == FName("and") || TypeName == FName("or"))
	{
		FActorIOGroupExpression* GroupExpr = new FActorIOGroupExpression();
		GroupExpr->SetNegated(TypeName == FName("or"));
		return GroupExpr;
	}

	return nullptr;
}

bool FActorIOExpressionParser::ParseBracket(const FString& Str, FString& OutPrefix, FString& OutData)
{
	int32 OpenBracketIdx = INDEX_NONE;
	if (!Str.FindChar(TEXT('('), OpenBracketIdx))
	{
		return false;
	}

	TCHAR LastChar = Str[Str.Len() - 1];
	if (LastChar != TEXT(')'))
	{
		return false;
	}

	OutPrefix = Str.Left(OpenBracketIdx);
	OutData = Str.Mid(OpenBracketIdx + 1, Str.Len() - (OpenBracketIdx + 2));
	return true;
}
