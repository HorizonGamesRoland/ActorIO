// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpression.h"
#include "ActorIOSubsystemBase.h"
#include "ActorIOVersions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceNull.h"

//=======================================================
//~ Begin FActorIOExpressionBase
//=======================================================

FActorIOExpressionBase::FActorIOExpressionBase()
{
	ParentExpr = nullptr;
}

//=======================================================
//~ Begin FActorIOLiteralExpression
//=======================================================

bool FActorIOLiteralExpression::Evaluate(FString& OutResult)
{
	OutResult = LiteralValue;
	return true;
}

bool FActorIOLiteralExpression::ExportText(FString& Str) const
{
	Str = FString::Printf(TEXT("val(\"%s\")"), *LiteralValue);
	return true;
}

bool FActorIOLiteralExpression::ImportText(const FString& Str)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpresionParser::ParseNextBracket(Str, Prefix, Data))
	{
		return false;
	}

	if (Prefix != TEXT("val"))
	{
		return false;
	}

	if (Data.Len() >= 2)
	{
		if (Data[0] == TEXT('"') && Data[Data.Len() - 1] == TEXT('"'))
		{
			Data.MidInline(1, Data.Len() - 2);
		}
	}

	LiteralValue = Data;
	return true;
}

//=======================================================
//~ Begin FActorIOFunctionExpressionBase
//=======================================================

FActorIOFunctionExpressionBase::~FActorIOFunctionExpressionBase()
{
	Args.Empty();
}

void FActorIOFunctionExpressionBase::AddArgument(FActorIOExpressionBase* InExpr)
{
	if (InExpr)
	{
		InExpr->SetParent(this);
		Args.Add(InExpr);
	}
}

void FActorIOFunctionExpressionBase::RemoveArgument(FActorIOExpressionBase* InExpr)
{
	if (InExpr)
	{
		int32 Idx = Args.Find(InExpr);
		if (Idx != INDEX_NONE)
		{
			Args.RemoveAt(Idx);
			delete InExpr;
		}
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
		Args[Index] = InExpr;
	}
}

void FActorIOFunctionExpressionBase::ResetArguments()
{
	// #todo: check if element destructors are called properly
	Args.Empty();
}

bool FActorIOFunctionExpressionBase::ImportArgumentsText(const FString& Str)
{
	const TCHAR* Stream = *Str;

	TArray<FString> ArgDatas;

	FString NewData;
	bool bInQuotes = false;
	int32 BracketDepth = 0;
	while (*Stream)
	{
		// #todo: add escape char for cases where bracket is present in a literal

		if (*Stream == TEXT(',') && BracketDepth == 0)
		{
			ArgDatas.Add(NewData);
			NewData.Empty();
		}
		else
		{
			NewData.AppendChar(*Stream);
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

	if (BracketDepth != 0)
	{
		return false;
	}

	ResetArguments();
	for (const FString& ArgData : ArgDatas)
	{
		FString Prefix;
		FString Data;
		if (!FActorIOExpresionParser::ParseNextBracket(ArgData, Prefix, Data))
		{
			return false;
		}

		FActorIOExpressionBase* NewExpr = FActorIOExpresionParser::CreateExpressionFromTypeString(Prefix);
		if (!NewExpr)
		{
			return false;
		}

		if (!NewExpr->ImportText(Data))
		{
			return false;
		}

		AddArgument(NewExpr);
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

//=======================================================
//~ Begin FActorIOKismetFunctionExpression
//=======================================================

bool FActorIOKismetFunctionExpression::Evaluate(FString& OutResult)
{
	OutResult.Empty();

	if (!ClassPtr.IsValid())
	{
		// #TODO: log error
		return false;
	}

	FString Cmd = FunctionId.ToString();
	for (FActorIOExpressionBase* Expr : Args)
	{
		FString Result;
		if (!Expr || !Expr->Evaluate(Result))
		{
			return false;
		}

		Cmd += TEXT(" ");
		Cmd += Result;
	}

	UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(GEngine->GetCurrentPlayWorld());
	check(IOSubsystem);

	FOutputDeviceNull Ar;
	return IOSubsystem->ExecuteCommand(ClassPtr.Pin().Get()->GetDefaultObject(), *Cmd, Ar, IOSubsystem, &OutResult);
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
		// #todo: negated

		FSoftObjectPath ClassPath = ClassPtr.Pin().Get()->GetPathName();
		FuncData += ClassPath.ToString();
		FuncData += TEXT(':');
		FuncData += FunctionId.ToString();

		if (!ArgsData.IsEmpty())
		{
			FuncData += FString::Printf(TEXT("(%s)"), *ArgsData);
		}
	}

	Str = FString::Printf(TEXT("func(%s))"), *FuncData);
	return true;
}

bool FActorIOKismetFunctionExpression::ImportText(const FString& Str)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpresionParser::ParseNextBracket(Str, Prefix, Data))
	{
		return false;
	}

	if (Prefix != TEXT("func"))
	{
		return false;
	}

	FString FuncData = Data;
	FString ArgsData;
	if (FActorIOExpresionParser::ParseNextBracket(FuncData, Prefix, Data))
	{
		if (Prefix == TEXT("not"))
		{
			// #todo: set negated

			FuncData = Data;
			if (FActorIOExpresionParser::ParseNextBracket(FuncData, Prefix, Data))
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
	}

	if (!ResolvedClass)
	{
		return false;
	}

	ClassPtr = ResolvedClass;
	FunctionId = FName(*FunctionIdStr, FNAME_Find);

	if (!ImportArgumentsText(ArgsData))
	{
		return false;
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

	TArray<FProperty*> FunctionParams = GetUFunctionParams();
	for (int32 ArgIdx = 0; ArgIdx != FunctionParams.Num(); ++ArgIdx)
	{
		FActorIOLiteralExpression* NewArg = new FActorIOLiteralExpression();
		AddArgument(NewArg);
	}
}

UFunction* FActorIOKismetFunctionExpression::GetUFunction()
{
	if (ClassPtr.IsValid() && FunctionId != NAME_None)
	{
		return ClassPtr.Pin().Get()->FindFunctionByName(FunctionId);
	}

	return nullptr;
}

TArray<FProperty*> FActorIOKismetFunctionExpression::GetUFunctionParams()
{
	TArray<FProperty*> OutParams;

	UFunction* ReferencedFunction = GetUFunction();
	if (ReferencedFunction)
	{
		for (TFieldIterator<FProperty> It(ReferencedFunction); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			FProperty* FunctionProp = *It;
			checkSlow(FunctionProp);

			// Do not create widget for return property.
			if (FunctionProp->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				continue;
			}

			// Do not create widget for output params, but only if they are not passed by ref
			// since in that case the value is also an input param.
			if (FunctionProp->HasAnyPropertyFlags(CPF_OutParm) && !FunctionProp->HasAnyPropertyFlags(CPF_ReferenceParm))
			{
				continue;
			}

			// Skip blueprint generated '__WorldContext' property.
			// The value for this will be auto initialized for us by the 'ActorIO::ExecuteCommand' function.
			if (FunctionProp->GetName() == TEXT("__WorldContext"))
			{
				continue;
			}

			OutParams.Add(FunctionProp);
		}
	}

	return OutParams;
}

//=======================================================
//~ Begin FActorIOGroupExpression
//=======================================================

bool FActorIOGroupExpression::Evaluate(FString& OutResult)
{
	for (FActorIOExpressionBase* Expr : Args)
	{
		FString Result;
		if (!Expr || !Expr->Evaluate(Result))
		{
			return false;
		}

		if (Result != TEXT("1") && Result != TEXT("True"))
		{
			return false;
		}
	}

	return true;
}

bool FActorIOGroupExpression::ExportText(FString& Str) const
{
	FString ArgsData;
	if (!ExportArgumentsText(ArgsData))
	{
		return false;
	}

	Str = FString::Printf(TEXT("and(%s)"), *ArgsData); // #todo: negated
	return true;
}

bool FActorIOGroupExpression::ImportText(const FString& Str)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpresionParser::ParseNextBracket(Str, Prefix, Data))
	{
		return false;
	}

	if (Prefix != TEXT("and") && Prefix != TEXT("or"))
	{
		return false;
	}

	// #todo: negated

	if (!ImportArgumentsText(Data))
	{
		return false;
	}

	return true;
}

//=======================================================
//~ Begin FActorIOScriptCondition
//=======================================================

bool FActorIOScriptCondition::operator==(const FActorIOScriptCondition& Other) const
{
	FString Data;
	FString OtherData;
	if (Expr.ExportText(Data) && Other.Expr.ExportText(OtherData))
	{
		return Data == OtherData;
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

		FString ExprStr;
		if (Expr.ExportText(ExprStr))
		{
			Ar << ExprStr;
			return true;
		}
	}
	else
	{
		FString SavedStr;
		Ar << SavedStr;

		if (Expr.ImportText(SavedStr))
		{
			return true;
		}
	}

	return false;
}

bool FActorIOScriptCondition::ExportTextItem(FString& ValueStr, FActorIOScriptCondition const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
	return Expr.ExportText(ValueStr);
}

bool FActorIOScriptCondition::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	return Expr.ImportText(Buffer);
}

//=======================================================
//~ Begin FActorIOExpresionParser
//=======================================================

FActorIOExpressionBase* FActorIOExpresionParser::CreateExpressionFromTypeString(const FString& Str)
{
	if (Str == TEXT("val"))
	{
		return new FActorIOLiteralExpression();
	}
	else if (Str == TEXT("func"))
	{
		return new FActorIOKismetFunctionExpression();
	}
	else if (Str == TEXT("and"))
	{
		return new FActorIOGroupExpression();
	}
	else if (Str == TEXT("or"))
	{
		return new FActorIOGroupExpression();
	}

	return nullptr;
}

bool FActorIOExpresionParser::ParseNextBracket(const FString& Str, FString& OutPrefix, FString& OutData)
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

	OutPrefix = Str.Left(OpenBracketIdx - 1);
	OutData = Str.Mid(OpenBracketIdx, Str.Len() - OpenBracketIdx - 1);
	return true;
}
