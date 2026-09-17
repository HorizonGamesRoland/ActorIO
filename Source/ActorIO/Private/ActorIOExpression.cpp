// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpression.h"
#include "ActorIOSubsystemBase.h"
#include "ActorIOVersions.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceNull.h"

// #todo: rename to ActorIOExpressions.h


//=======================================================
//~ Begin FActorIOExpressionBase
//=======================================================

FActorIOExpressionBase::FActorIOExpressionBase()
{
	ParentContainer = nullptr;
	//bIsCondition = false;
}

//FActorIOExpressionBase* FActorIOExpressionBase::GetRootExpression()
//{
//	FActorIOExpressionBase* RootExpr = this;
//	while (true)
//	{
//		FActorIOExpressionBase* Parent = RootExpr->GetParent();
//		if (!Parent)
//		{
//			break;
//		}
//
//		RootExpr = Parent;
//	}
//
//	return RootExpr;
//}

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

	//ResetArguments();
	//for (const FString& ArgData : ArgDatas)
	//{
	//	FActorIOExpressionBase* NewExpr = FActorIOExpressionParser::NewExpressionFromString(ArgData, Version);
	//	if (NewExpr)
	//	{
	//		AddArgument(NewExpr);
	//	}
	//}

	return true;
}

bool FActorIOFunctionExpressionBase::ExportArgumentsText(FString& Str) const
{
	//for (FActorIOExpressionBase* Arg : Args)
	//{
	//	if (!Arg) continue;
	//	if (!Str.IsEmpty()) Str += TEXT(',');
	//
	//	FString ArgData;
	//	if (!Arg->ExportText(ArgData))
	//	{
	//		return false;
	//	}
	//
	//	Str += ArgData;
	//}

	return true;
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

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<FActorIOExpressionBase*> ChildExpressions = ExprContainer->GetChildExpressions(SelfIdx);

	FString Cmd = FunctionId.ToString();
	for (FActorIOExpressionBase* Expr : ChildExpressions)
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
	//ResetArguments();

	UFunction* FunctionPtr = ResolveUFunction();
	if (FunctionPtr)
	{
		TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(FunctionPtr);
		for (int32 ArgIdx = 0; ArgIdx != FunctionParams.Num(); ++ArgIdx)
		{
			//FActorIOLiteralExpression* NewArg = new FActorIOLiteralExpression();
			//AddArgument(NewArg);
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

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<FActorIOExpressionBase*> ChildExpressions = ExprContainer->GetChildExpressions(SelfIdx);

	// Empty groups are treated as if they are not even there.
	if (ChildExpressions.Num() == 0)
	{
		OutResult = TEXT("True");
		return true;
	}

	bool bResult = true;
	for (FActorIOExpressionBase* Expr : ChildExpressions)
	{
		FString Result;
		if (!Expr->Evaluate(Executor, Result))
		{
			return false;
		}

		if (ExprContainer->IsConditionContainer())
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
//~ Begin FActorIOExpressionContainer
//=======================================================

int32 FActorIOExpressionContainer::AddExpression(const TInstancedStruct<FActorIOExpressionBase>& Expr, int32 ParentIdx)
{
	//TInstancedStruct<FActorIOExpressionBase> InstancedStruct;
	//InstancedStruct.InitializeAsScriptStruct(TBaseStructure<FActorIOGroupExpression>::Get());
	//Expressions.Add(InstancedStruct);

	//TInstancedStruct<FActorIOExpressionBase> InstancedStruct = TInstancedStruct<FActorIOExpressionBase>::Make<FActorIOGroupExpression>();
	//Expressions.Add(InstancedStruct);

	//FActorIOGroupExpression GroupExpr;
	//TInstancedStruct<FActorIOExpressionBase> InstancedStruct;
	//InstancedStruct.InitializeAs(GroupExpr);

	//FActorIOGroupExpression GroupExpr;
	//TInstancedStruct<FActorIOExpressionBase> InstancedStruct;
	//InstancedStruct.InitializeAs(FActorIOGroupExpression::StaticStruct(), GroupExpr);

	if (ParentIdx == INDEX_NONE && Expressions.Num() > 0)
	{
		// #todo: error
		return INDEX_NONE;
	}

	if (ParentIdx != INDEX_NONE && !Expressions.IsValidIndex(ParentIdx))
	{
		// #todo: error
		return INDEX_NONE;
	}

	// #todo: ensure expression can have children

	int32 ExprIdx = Expressions.Add(Expr);
	ParentIndexMapping.Add(ParentIdx);

	Expressions[ExprIdx]->SetContainer(this);

	return ExprIdx;
}

void FActorIOExpressionContainer::RemoveExpression(int32 ExprIdx)
{
	if (Expressions.IsValidIndex(ExprIdx))
	{
		Expressions.RemoveAt(ExprIdx);
		ParentIndexMapping.RemoveAt(ExprIdx);
	}
}

void FActorIOExpressionContainer::Empty()
{
	Expressions.Empty();
	ParentIndexMapping.Empty();
}

int32 FActorIOExpressionContainer::GetExpressionIdx(const FActorIOExpressionBase* InExpr) const
{
	if (InExpr->GetContainer() != this)
	{
		return INDEX_NONE;
	}

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		// #todo: does this work?
		if (Expressions[Idx].GetPtr<FActorIOExpressionBase>() == InExpr)
		{
			return Idx;
		}
	}

	return INDEX_NONE;
}

int32 FActorIOExpressionContainer::GetExpressionParentIdx(const FActorIOExpressionBase* InExpr) const
{
	int32 ExprIdx = GetExpressionIdx(InExpr);
	return ParentIndexMapping[ExprIdx];
}

TArray<FActorIOExpressionBase*> FActorIOExpressionContainer::GetChildExpressions(int32 ExprIdx)
{
	TArray<FActorIOExpressionBase*> OutExpressions;

	ensure(Expressions.Num() == ParentIndexMapping.Num());

	//const int32 RequestorIdx = Expressions.IndexOfByKey(Expr);
	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == ExprIdx)
		{
			if (Expressions[Idx].IsValid())
			{
				FActorIOExpressionBase* MutablePtr = Expressions[Idx].GetMutablePtr<FActorIOExpressionBase>();
				OutExpressions.Add(MutablePtr);
			}
		}
	}

	return OutExpressions;
}

FActorIOExpressionBase* FActorIOExpressionContainer::GetRootExpression()
{
	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == INDEX_NONE)
		{
			if (Expressions[Idx].IsValid())
			{
				return Expressions[Idx].GetMutablePtr<FActorIOExpressionBase>();
			}
		}
	}

	return nullptr;
}

bool FActorIOExpressionContainer::Evaluate(UObject* Executor)
{
	FActorIOExpressionBase* RootExpr = GetRootExpression();
	FString Result;
	if (RootExpr && RootExpr->Evaluate(Executor, Result))
	{
		if (bIsConditionContainer)
		{
			return FCString::ToBool(*Result);
		}
		else
		{
			return true;
		}
	}

	return false;
}

void FActorIOExpressionContainer::PostScriptConstruct()
{
	UE_LOG(LogTemp, Log, TEXT("%s"), ANSI_TO_TCHAR(__FUNCTION__));
	for (TInstancedStruct<FActorIOExpressionBase>& ExprInstance : Expressions)
	{
		ExprInstance->SetContainer(this);
	}
}

//bool FActorIOScriptCondition::operator==(const FActorIOScriptCondition& Other) const
//{
//	FActorIOGroupExpression* OtherExpr = Other.GetExpression();
//	if (Expr.IsValid() && OtherExpr)
//	{
//		FString Data;
//		FString OtherData;
//		if (Expr->ExportText(Data) && OtherExpr->ExportText(OtherData))
//		{
//			return Data == OtherData;
//		}
//	}
//
//	return false;
//}
//
//bool FActorIOScriptCondition::Serialize(FArchive& Ar)
//{
//	Ar.UsingCustomVersion(FActorIOExpressionVersion::GUID);
//
//	int32 Version = Ar.CustomVer(FActorIOExpressionVersion::GUID);
//	Ar << Version;
//
//	if (Ar.IsLoading())
//	{
//		Ar.SetCustomVersion(FActorIOExpressionVersion::GUID, Version, TEXT("ActorIOExpressionVer"));
//
//		FString SavedStr;
//		Ar << SavedStr;
//
//		if (Expr.IsValid())
//		{
//			return Expr->ImportText(SavedStr, Version);
//		}
//	}
//	else
//	{
//		FString ExprStr;
//		if (Expr.IsValid())
//		{
//			Expr->ExportText(ExprStr);
//		}
//
//		Ar << ExprStr;
//		return true;
//	}
//
//	return false;
//}
//
//bool FActorIOScriptCondition::ExportTextItem(FString& ValueStr, FActorIOScriptCondition const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
//{
//	return Expr.IsValid() && Expr->ExportText(ValueStr);
//}
//
//bool FActorIOScriptCondition::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
//{
//	return Expr.IsValid() && Expr->ImportText(Buffer, (int32)FActorIOExpressionVersion::LatestVersion);
//}

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
