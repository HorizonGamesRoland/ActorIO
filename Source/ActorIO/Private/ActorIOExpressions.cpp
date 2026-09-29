// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpressions.h"
#include "ActorIOSubsystemBase.h"
#include "ActorIOVersions.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceNull.h"

//=======================================================
//~ Begin FActorIOExpressionBase
//=======================================================

FActorIOExpressionBase::FActorIOExpressionBase()
{
	ParentContainer = nullptr;
}

//=======================================================
//~ Begin FActorIOLiteralExpression
//=======================================================

bool FActorIOLiteralExpression::Evaluate(UObject* Executor, FString& OutReturnValue)
{
	OutReturnValue = LiteralValue;
	return true;
}

bool FActorIOLiteralExpression::InitFromString(const FString& Str)
{
	// Example valid strings:
	// - val()
	// - val(1.0)
	// - val(Hello World)

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

FString FActorIOLiteralExpression::ToString() const
{
	return FString::Printf(TEXT("val(%s)"), *LiteralValue);
}

//=======================================================
//~ Begin FActorIOFunctionExpressionBase
//=======================================================

FActorIOFunctionExpressionBase::FActorIOFunctionExpressionBase()
{
	bNegated = false;
}

bool FActorIOFunctionExpressionBase::InitChildExpressionsFromString(const FString& Str)
{
	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	TArray<FString> ExprDatas;
	if (!Str.IsEmpty())
	{
		const TCHAR* Stream = *Str;

		FString NewData;
		int32 BracketDepth = 0;
		while (*Stream)
		{
			if (BracketDepth == 0 && *Stream == TEXT(','))
			{
				ExprDatas.Add(NewData);
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
			ExprDatas.Add(NewData);
		}
	}

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	ExprContainer->RemoveChildExpressions(SelfIdx);

	for (const FString& ExprData : ExprDatas)
	{
		ExprContainer->NewExpressionFromString(ExprData, SelfIdx);
	}

	return true;
}

FString FActorIOFunctionExpressionBase::ChildExpressionToString() const
{
	FString OutStr;

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return OutStr;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<FActorIOExpressionBase*> ChildExpressions = ExprContainer->GetChildExpressions(SelfIdx);

	for (FActorIOExpressionBase* Expr : ChildExpressions)
	{
		if (!OutStr.IsEmpty()) OutStr += TEXT(',');
		OutStr += Expr->ToString();
	}

	return OutStr;
}

//=======================================================
//~ Begin FActorIOKismetFunctionExpression
//=======================================================

bool FActorIOKismetFunctionExpression::Evaluate(UObject* Executor, FString& OutReturnValue)
{
	OutReturnValue.Empty();

	UClass* ClassPtr = FunctionClass.Get();
	UFunction* FunctionPtr = ResolveUFunction();
	if (!FunctionPtr)
	{
		// #TODO: log error
		return false;
	}

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<FActorIOExpressionBase*> ChildExpressions = ExprContainer->GetChildExpressions(SelfIdx);

	FString Cmd = FunctionName.ToString();
	for (FActorIOExpressionBase* Expr : ChildExpressions)
	{
		FString ReturnValue;
		if (!Expr || !Expr->Evaluate(Executor, ReturnValue))
		{
			return false;
		}

		Cmd += TEXT(" ");
		Cmd += ReturnValue;
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

				OutReturnValue = bValue ? TEXT("True") : TEXT("False");
			}
			else
			{
				ReturnProperty->ExportTextItem_InContainer(OutReturnValue, Container, nullptr, nullptr, PPF_None);
			}
		}
	};

	FOutputDeviceNull Ar;
	return IOSubsystem->ExecuteCommand(ClassPtr->GetDefaultObject(), *Cmd, Ar, IOSubsystem, ReturnPropertyAccessor);
}

bool FActorIOKismetFunctionExpression::InitFromString(const FString& Str)
{
	// Example valid strings:
	// - func()
	// - func(/Script/BP_ExpressionLib_C:TestFunction()
	// - func(not(/Script/BP_ExpressionLib_C:TestFunction(func(/Script/BP_ExpressionLib_C:ConstTrue),val(1.0))))

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
		FString ChildsData;
		if (FActorIOExpressionParser::ParseBracket(FuncData, Prefix, Data))
		{
			bNegated = Prefix == TEXT("not");
			if (bNegated)
			{
				FuncData = Data;
				if (FActorIOExpressionParser::ParseBracket(FuncData, Prefix, Data))
				{
					FuncData = Prefix;
					ChildsData = Data;
				}
			}
			else
			{
				FuncData = Prefix;
				ChildsData = Data;
			}
		}

		FString ClassPathStr;
		FString FunctionNameStr;
		if (!FuncData.Split(TEXT(":"), &ClassPathStr, &FunctionNameStr))
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

		UE_CLOG(ResolvedClass == nullptr, LogActorIO, Error, TEXT("%s - Could not resolve class: %s"), ANSI_TO_TCHAR(__FUNCTION__), *ClassPath.ToString());
		FunctionClass = ResolvedClass;
		FunctionName = FName(*FunctionNameStr, FNAME_Find);

		UFunction* FunctionPtr = ResolveUFunction();
		if (FunctionPtr)
		{
			InitChildExpressionsFromString(ChildsData);
		}
	}

	return true;
}

FString FActorIOKismetFunctionExpression::ToString() const
{
	FString FuncData;
	if (FunctionClass)
	{
		FuncData += FunctionClass->GetPathName();
		FuncData += TEXT(':');
		FuncData += FunctionName.ToString();

		FString ChildsData = ChildExpressionToString();
		if (!ChildsData.IsEmpty())
		{
			FuncData += FString::Printf(TEXT("(%s)"), *ChildsData);
		}

		if (bNegated)
		{
			FString DataToWrap = FuncData;
			FuncData = FString::Printf(TEXT("not(%s)"), *DataToWrap);
		}
	}

	return FString::Printf(TEXT("func(%s)"), *FuncData);
}

void FActorIOKismetFunctionExpression::SetFunctionClass(UClass* InClassPtr)
{
	if (FunctionClass != InClassPtr)
	{
		FunctionClass = InClassPtr;
		UpdateArguments();
	}
}

void FActorIOKismetFunctionExpression::SetFunctionName(FName InFunctionName)
{
	if (FunctionName != InFunctionName)
	{
		FunctionName = InFunctionName;
		UpdateArguments();
	}
}

void FActorIOKismetFunctionExpression::UpdateArguments()
{
	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	ExprContainer->RemoveChildExpressions(SelfIdx);

	UFunction* FunctionPtr = ResolveUFunction();
	if (FunctionPtr)
	{
		TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(FunctionPtr);
		for (int32 ArgIdx = 0; ArgIdx != FunctionParams.Num(); ++ArgIdx)
		{
			FActorIOLiteralExpression NewExpr;
			TInstancedStruct<FActorIOExpressionBase> NewExprInstance;
			NewExprInstance.InitializeAs<FActorIOLiteralExpression>(NewExpr);

			ExprContainer->AddExpression(NewExprInstance, SelfIdx);
		}
	}
}

UFunction* FActorIOKismetFunctionExpression::ResolveUFunction() const
{
	if (FunctionClass && FunctionName != NAME_None)
	{
		return FunctionClass->FindFunctionByName(FunctionName);
	}

	return nullptr;
}

//=======================================================
//~ Begin FActorIOGroupExpression
//=======================================================

bool FActorIOGroupExpression::Evaluate(UObject* Executor, FString& OutReturnValue)
{
	OutReturnValue.Empty();

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<FActorIOExpressionBase*> ChildExpressions = ExprContainer->GetChildExpressions(SelfIdx);

	// Empty groups are treated as if they are not even there.
	if (ChildExpressions.Num() == 0)
	{
		OutReturnValue = TEXT("True");
		return true;
	}

	bool bReturnValue = true;
	for (FActorIOExpressionBase* Expr : ChildExpressions)
	{
		FString ReturnValue;
		if (!Expr->Evaluate(Executor, ReturnValue))
		{
			return false;
		}

		if (ExprContainer->IsConditionContainer())
		{
			// This handles early exit cases when evaluating conditions:
			// - Result is true and we want 'ANY is true' (bNegated true), so its a pass.
			// - Result is false and we want 'ALL is true' (bNegated false), so its a fail.

			const bool bResultAsBool = FCString::ToBool(*ReturnValue);
			if (bResultAsBool == bNegated)
			{
				bReturnValue = bResultAsBool;
				break;
			}
		}
	}

	OutReturnValue = bReturnValue ? TEXT("True") : TEXT("False");
	return true;
}

bool FActorIOGroupExpression::InitFromString(const FString& Str)
{
	// Example valid strings:
	// - and()
	// - or
	// - and(func(...),func(...))
	// - or(func(...),func(...),func(...))     # todo: add eq, neq examples

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
	InitChildExpressionsFromString(Data);
	return true;
}

FString FActorIOGroupExpression::ToString() const
{
	FString ChildsData = ChildExpressionToString();
	if (bNegated)
	{
		return FString::Printf(TEXT("or(%s)"), *ChildsData);
	}
	else
	{
		return FString::Printf(TEXT("and(%s)"), *ChildsData);
	}
}

//=======================================================
//~ Begin FActorIOExpressionContainer
//=======================================================

FActorIOExpressionContainer::FActorIOExpressionContainer()
{
	bIsConditionContainer = false;
}

int32 FActorIOExpressionContainer::AddExpression(const TInstancedStruct<FActorIOExpressionBase>& Expr, int32 ParentIdx)
{
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

bool FActorIOExpressionContainer::InitFromString(const FString& Str)
{
	Empty();
	return NewExpressionFromString(Str, INDEX_NONE);
}

FString FActorIOExpressionContainer::ToString() const
{
	const FActorIOExpressionBase* RootExpr = GetRootExpression();
	if (RootExpr)
	{
		return RootExpr->ToString();
	}

	return TEXT("");
}

bool FActorIOExpressionContainer::NewExpressionFromString(const FString& Str, int32 ParentIdx)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpressionParser::ParseBracket(Str, Prefix, Data))
	{
		return false;
	}

	TInstancedStruct<FActorIOExpressionBase> NewExprInstance = FActorIOExpressionParser::NewExpressionOfType(Prefix);
	if (!NewExprInstance.IsValid())
	{
		return false;
	}

	int32 NewExprIdx = AddExpression(NewExprInstance, ParentIdx);
	if (NewExprIdx == INDEX_NONE)
	{
		return false;
	}

	return Expressions[NewExprIdx]->InitFromString(Str);
}

void FActorIOExpressionContainer::RemoveExpression(int32 ExprIdx)
{
	if (Expressions.IsValidIndex(ExprIdx))
	{
		RemoveChildExpressions(ExprIdx);

		Expressions.RemoveAt(ExprIdx);
		ParentIndexMapping.RemoveAt(ExprIdx);

		// Shift parent indices down to account for the removed element.
		for (int32& ParentIdx : ParentIndexMapping)
		{
			if (ParentIdx > ExprIdx)
			{
				ParentIdx--;
			}
		}
	}
}

void FActorIOExpressionContainer::RemoveChildExpressions(int32 ExprIdx)
{
	if (Expressions.IsValidIndex(ExprIdx))
	{
		TArray<int32> ChildExprIdxs = GetChildExpressionIdxs(ExprIdx);
		ChildExprIdxs.Sort([](const int32& A, const int32& B) { return A > B; });
		for (const int32& ChildIdx : ChildExprIdxs)
		{
			RemoveExpression(ChildIdx);
		}
	}
}

void FActorIOExpressionContainer::Empty()
{
	Expressions.Empty();
	ParentIndexMapping.Empty();
}

int32 FActorIOExpressionContainer::GetExpressionIdx(const FActorIOExpressionBase* InExpr) const
{
	if (InExpr->GetContainer() == this)
	{
		for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
		{
			if (Expressions[Idx].GetPtr<FActorIOExpressionBase>() == InExpr)
			{
				return Idx;
			}
		}
	}

	return INDEX_NONE;
}

int32 FActorIOExpressionContainer::GetExpressionParentIdx(const FActorIOExpressionBase* InExpr) const
{
	const int32 ExprIdx = GetExpressionIdx(InExpr);
	return ParentIndexMapping[ExprIdx];
}

FActorIOExpressionBase* FActorIOExpressionContainer::GetExpressionPtr(int32 Idx)
{
	if (Expressions.IsValidIndex(Idx) && Expressions[Idx].IsValid())
	{
		return Expressions[Idx].GetMutablePtr<FActorIOExpressionBase>();
	}

	return nullptr;
}

const FActorIOExpressionBase* FActorIOExpressionContainer::GetExpressionPtr(int32 Idx) const
{
	if (Expressions.IsValidIndex(Idx) && Expressions[Idx].IsValid())
	{
		return Expressions[Idx].GetPtr<FActorIOExpressionBase>();
	}

	return nullptr;
}

TArray<FActorIOExpressionBase*> FActorIOExpressionContainer::GetChildExpressions(int32 ExprIdx)
{
	TArray<FActorIOExpressionBase*> OutExpressions;

	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == ExprIdx && Expressions[Idx].IsValid())
		{
			FActorIOExpressionBase* ExpressionPtr = Expressions[Idx].GetMutablePtr<FActorIOExpressionBase>();
			OutExpressions.Add(ExpressionPtr);
		}
	}

	return OutExpressions;
}

TArray<const FActorIOExpressionBase*> FActorIOExpressionContainer::GetChildExpressions(int32 ExprIdx) const
{
	TArray<const FActorIOExpressionBase*> OutExpressions;

	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == ExprIdx && Expressions[Idx].IsValid())
		{
			const FActorIOExpressionBase* ExpressionPtr = Expressions[Idx].GetPtr<FActorIOExpressionBase>();
			OutExpressions.Add(ExpressionPtr);
		}
	}

	return OutExpressions;
}

TArray<int32> FActorIOExpressionContainer::GetChildExpressionIdxs(int32 ExprIdx) const
{
	TArray<int32> OutExpressionIdxs;

	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == ExprIdx && Expressions[Idx].IsValid())
		{
			OutExpressionIdxs.Add(Idx);
		}
	}

	return OutExpressionIdxs;
}

FActorIOExpressionBase* FActorIOExpressionContainer::GetRootExpression()
{
	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == INDEX_NONE && Expressions[Idx].IsValid())
		{
			return Expressions[Idx].GetMutablePtr<FActorIOExpressionBase>();
		}
	}

	return nullptr;
}

const FActorIOExpressionBase* FActorIOExpressionContainer::GetRootExpression() const
{
	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == INDEX_NONE && Expressions[Idx].IsValid())
		{
			return Expressions[Idx].GetPtr<FActorIOExpressionBase>();
		}
	}

	return nullptr;
}

int32 FActorIOExpressionContainer::GetRootExpressionIdx() const
{
	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == INDEX_NONE && Expressions[Idx].IsValid())
		{
			return Idx;
		}
	}

	return INDEX_NONE;
}

void FActorIOExpressionContainer::FixupContainerReferences()
{
	for (TInstancedStruct<FActorIOExpressionBase>& ExprInstance : Expressions)
	{
		ExprInstance->SetContainer(this);
	}
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

bool FActorIOExpressionContainer::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FActorIOExpressionVersion::GUID);
	
	// doesn't actually serialize, just write the custom version for PostSerialize (for use in the future)
	return false;
}

void FActorIOExpressionContainer::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading() || Ar.IsTransacting())
	{
		FixupContainerReferences();
	}
}

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

TInstancedStruct<FActorIOExpressionBase> FActorIOExpressionParser::NewExpressionOfType(FString TypeStr)
{
	TInstancedStruct<FActorIOExpressionBase> NewExprInstance;

	if (TypeStr == TEXT("val"))
	{
		FActorIOLiteralExpression LiteralExpr;
		NewExprInstance.InitializeAs<FActorIOLiteralExpression>(LiteralExpr);
	}
	else if (TypeStr == TEXT("func"))
	{
		FActorIOKismetFunctionExpression FunctionExpr;
		NewExprInstance.InitializeAs<FActorIOKismetFunctionExpression>(FunctionExpr);
	}
	else if (TypeStr == TEXT("and") || TypeStr == TEXT("or"))
	{
		FActorIOGroupExpression GroupExpr;
		GroupExpr.SetNegated(TypeStr == TEXT("or"));
		NewExprInstance.InitializeAs<FActorIOGroupExpression>(GroupExpr);
	}

	return NewExprInstance;
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
