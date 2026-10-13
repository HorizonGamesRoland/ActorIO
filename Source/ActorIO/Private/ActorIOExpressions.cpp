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

void FActorIOExpressionBase::SetMetadata(const FString& InName, const FString& InValue)
{
	if (InName.IsEmpty())
	{
		return;
	}

	TArray<FString> MetadataLines;

	const TCHAR* Stream = *Metadata;
	while (*Stream)
	{
		FString NewMetadata;
		if (FParse::Token(Stream, NewMetadata, true, TEXT(',')))
		{
			MetadataLines.Emplace(NewMetadata);
		}
	}

	bool bFound = false;
	for (FString& Line : MetadataLines)
	{
		FString MetadataName;
		FString MetadataValue;
		if (Line.Split(TEXT("="), &MetadataName, &MetadataValue))
		{
			if (MetadataName == InName)
			{
				Line = FString::Printf(TEXT("%s=%s"), *MetadataName, *InValue);
				bFound = true;
				break;
			}
		}
	}

	if (!bFound)
	{
		if (!Metadata.IsEmpty()) Metadata += TEXT(',');
		Metadata += FString::Printf(TEXT("%s=%s"), *InName, *InValue);
	}
	else
	{
		Metadata.Empty();
		for (const FString& Line : MetadataLines)
		{
			if (!Metadata.IsEmpty()) Metadata += TEXT(',');
			Metadata += Line;
		}
	}
}

bool FActorIOExpressionBase::RemoveMetadata(const FString& InName)
{
	if (InName.IsEmpty())
	{
		return false;
	}

	TArray<FString> MetadataLines;

	const TCHAR* Stream = *Metadata;
	while (*Stream)
	{
		FString NewMetadata;
		if (FParse::Token(Stream, NewMetadata, true, TEXT(',')))
		{
			MetadataLines.Emplace(NewMetadata);
		}
	}

	bool bFound = false;
	for (int32 Idx = MetadataLines.Num() - 1; Idx >= 0; --Idx)
	{
		FString MetadataName;
		FString MetadataValue;
		if (MetadataLines[Idx].Split(TEXT("="), &MetadataName, &MetadataValue))
		{
			if (MetadataName == InName)
			{
				MetadataLines.RemoveAt(Idx);
				bFound = true;
				break;
			}
		}
	}

	if (bFound)
	{
		Metadata.Empty();
		for (const FString& Line : MetadataLines)
		{
			if (!Metadata.IsEmpty()) Metadata += TEXT(',');
			Metadata += Line;
		}
	}

	return bFound;
}

void FActorIOExpressionBase::ClearMetadata()
{
	Metadata.Empty();
}

bool FActorIOExpressionBase::HasMetadata(const FString& InName) const
{
	FString Match = InName + TEXT('=');
	FString Value; // unused
	return FParse::Value(*Metadata, *Match, Value);
}

bool FActorIOExpressionBase::GetMetadata(const FString& InName, FString& OutValue) const
{
	FString Match = InName + TEXT('=');
	return FParse::Value(*Metadata, *Match, OutValue);
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

UFunction* FActorIOKismetFunctionExpression::ResolveUFunction() const
{
	if (FunctionClass && FunctionName != NAME_None)
	{
		return FunctionClass->FindFunctionByName(FunctionName);
	}

	return nullptr;
}

bool FActorIOKismetFunctionExpression::CheckArgumentsNeedUpdate() const
{
	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);

	UFunction* FunctionPtr = ResolveUFunction();
	if (!FunctionPtr)
	{
		return ExprContainer->GetNumChildExpressions(SelfIdx) > 0;
	}

	TArray<int32> ChildExprIdxs = ExprContainer->GetChildExpressionIdxs(SelfIdx);
	TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(FunctionPtr);

	int32 Diff = ChildExprIdxs.Num() - FunctionParams.Num();
	if (Diff != 0)
	{
		return true;
	}

	for (int32 ParamIdx = 0; ParamIdx != FunctionParams.Num(); ++ParamIdx)
	{
		FString ExtendedTypeText;
		FString PropType = FunctionParams[ParamIdx]->GetCPPType(&ExtendedTypeText) + ExtendedTypeText;
		FString PropName = FunctionParams[ParamIdx]->GetAuthoredName();

		const FActorIOExpressionBase* ChildExpr = ExprContainer->GetExpressionPtr(ChildExprIdxs[ParamIdx]);

		FString ChildExprPropName;
		FString ChildExprPropType;
		ChildExpr->GetMetadata(TEXT("PropName"), ChildExprPropName);
		ChildExpr->GetMetadata(TEXT("PropType"), ChildExprPropType);

		if (ChildExprPropName != PropName || ChildExprPropType != PropType)
		{
			return true;
		}
	}

	return false;
}

void FActorIOKismetFunctionExpression::UpdateArguments()
{
	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);

	UFunction* FunctionPtr = ResolveUFunction();
	if (!FunctionPtr)
	{
		ExprContainer->RemoveChildExpressions(SelfIdx);
		return;
	}

	TArray<int32> ChildExprIdxs;
	bool bChildExprIdxsDirty = true;

	TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(FunctionPtr);
	for (int32 ParamIdx = 0; ParamIdx != FunctionParams.Num(); ++ParamIdx)
	{
		if (bChildExprIdxsDirty)
		{
			ChildExprIdxs = ExprContainer->GetChildExpressionIdxs(SelfIdx);
			bChildExprIdxsDirty = false;
		}

		FString ExtendedTypeText;
		FString PropType = FunctionParams[ParamIdx]->GetCPPType(&ExtendedTypeText) + ExtendedTypeText;
		FString PropName = FunctionParams[ParamIdx]->GetAuthoredName();

		int32 MatchingChildIdx = INDEX_NONE;
		for (int32 ChildIdx = 0; ChildIdx != ChildExprIdxs.Num(); ++ChildIdx)
		{
			const FActorIOExpressionBase* ChildExpr = ExprContainer->GetExpressionPtr(ChildExprIdxs[ChildIdx]);

			FString ChildExprPropName;
			FString ChildExprPropType;
			ChildExpr->GetMetadata(TEXT("PropName"), ChildExprPropName);
			ChildExpr->GetMetadata(TEXT("PropType"), ChildExprPropType);

			if (ChildExprPropName == PropName && ChildExprPropType == PropType)
			{
				MatchingChildIdx = ChildIdx;
				break;
			}
		}

		if (MatchingChildIdx == INDEX_NONE)
		{
			FActorIOLiteralExpression NewExpr;
			NewExpr.SetMetadata(TEXT("PropName"), PropName);
			NewExpr.SetMetadata(TEXT("PropType"), PropType);
			TInstancedStruct<FActorIOExpressionBase> NewExprInstance;
			NewExprInstance.InitializeAs<FActorIOLiteralExpression>(NewExpr);

			ExprContainer->AddExpression(NewExprInstance, SelfIdx, ParamIdx);
			bChildExprIdxsDirty = true;
			continue;
		}
		else
		{
			if (MatchingChildIdx != ParamIdx)
			{
				FActorIOExpressionBase* ChildExpr = ExprContainer->GetExpressionPtr(ChildExprIdxs[MatchingChildIdx]);
				ChildExpr->SetMetadata(TEXT("PropName"), PropName);
				ChildExpr->SetMetadata(TEXT("PropType"), PropType);

				ExprContainer->MoveExpressionTo(ChildExprIdxs[MatchingChildIdx], SelfIdx, ParamIdx);
				bChildExprIdxsDirty = true;
				continue;
			}
		}
	}

	if (bChildExprIdxsDirty)
	{
		ChildExprIdxs = ExprContainer->GetChildExpressionIdxs(SelfIdx);
		bChildExprIdxsDirty = false;
	}

	int32 Diff = ChildExprIdxs.Num() - FunctionParams.Num();
	while (Diff > 0)
	{
		ExprContainer->RemoveExpression(ChildExprIdxs.Last());
		ChildExprIdxs.RemoveAt(ChildExprIdxs.Num() - 1);
		Diff--;
	}
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
	// - or(func(...),func(...),func(...))     #todo: add eq, neq examples

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

int32 FActorIOExpressionContainer::AddExpression(const TInstancedStruct<FActorIOExpressionBase>& Expr, int32 ParentIdx, int32 SlotIdx)
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
	MoveExpressionTo(ExprIdx, ParentIdx, SlotIdx);

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

void FActorIOExpressionContainer::MoveExpressionTo(int32 ExprIdx, int32 ParentIdx, int32 SlotIdx)
{
	const int32 Num = Expressions.Num();
	if (!ensure(Num == ParentIndexMapping.Num()))
	{
		return;
	}

	if (!Expressions.IsValidIndex(ExprIdx) || !Expressions.IsValidIndex(ParentIdx))
	{
		return;
	}

	// The root cannot be moved.
	if (ParentIndexMapping[ExprIdx] == INDEX_NONE)
	{
		return;
	}

	// Prevent cycles: the new parent must not be the expression itself or one of its descendants.
	if (IsExpressionDescendantOf(ParentIdx, ExprIdx))
	{
		return;
	}

	// Build ordered child lists (array order == sibling order).
	TArray<TArray<int32>> Children;
	Children.SetNum(Num);

	int32 RootIdx = INDEX_NONE;
	for (int32 Idx = 0; Idx != Num; ++Idx)
	{
		const int32 Parent = ParentIndexMapping[Idx];
		if (Parent == INDEX_NONE)
		{
			if (RootIdx == INDEX_NONE) RootIdx = Idx;
		}
		else
		{
			Children[Parent].Add(Idx);
		}
	}

	if (RootIdx == INDEX_NONE)
	{
		return;
	}

	// Detach from old parent, attach to new parent at the requested slot.
	Children[ParentIndexMapping[ExprIdx]].RemoveSingle(ExprIdx);

	TArray<int32>& NewSiblings = Children[ParentIdx];
	const int32 InsertAt = (SlotIdx < 0) ? NewSiblings.Num() : FMath::Min(SlotIdx, NewSiblings.Num());
	NewSiblings.Insert(ExprIdx, InsertAt);

	// Pre-order traversal gives the new array layout (as old indices).
	TArray<int32> NewOrder;
	NewOrder.Reserve(Num);

	TArray<int32> Stack;
	Stack.Add(RootIdx);
	while (Stack.Num() > 0)
	{
		const int32 Idx = Stack.Pop(EAllowShrinking::No);
		NewOrder.Add(Idx);

		const TArray<int32>& Kids = Children[Idx];
		for (int32 K = Kids.Num() - 1; K >= 0; --K)
		{
			Stack.Add(Kids[K]);
		}
	}

	// If something was unreachable the tree was already corrupt. Bail before modifying anything.
	if (!ensure(NewOrder.Num() == Num))
	{
		return;
	}

	TArray<int32> OldToNew;
	OldToNew.SetNumUninitialized(Num);
	for (int32 NewIdx = 0; NewIdx != Num; ++NewIdx)
	{
		OldToNew[NewOrder[NewIdx]] = NewIdx;
	}

	// Rebuild both arrays.
	TArray<TInstancedStruct<FActorIOExpressionBase>> NewExpressions;
	TArray<int32> NewParentMapping;
	NewExpressions.Reserve(Num);
	NewParentMapping.Reserve(Num);

	for (const int32 OldIdx : NewOrder)
	{
		NewExpressions.Add(MoveTemp(Expressions[OldIdx]));

		const int32 OldParent = (OldIdx == ExprIdx) ? ParentIdx : ParentIndexMapping[OldIdx];
		NewParentMapping.Add(OldParent == INDEX_NONE ? INDEX_NONE : OldToNew[OldParent]);
	}

	Expressions = MoveTemp(NewExpressions);
	ParentIndexMapping = MoveTemp(NewParentMapping);
}

void FActorIOExpressionContainer::MoveExpressionTo_OLD(int32 ExprIdx, int32 ParentIdx, int32 SlotIdx)
{
	if (!Expressions.IsValidIndex(ExprIdx) || !Expressions.IsValidIndex(ParentIdx))
	{
		return;
	}
	
	TFunction<void(int32, int32)> fnMoveExpression = [this, ParentIdx](int32 OriginalIndex, int32 NewIndex)
	{
		// Uses same move implementation as properties in the editor.
		// @see FPropertyValueImpl::MoveElementTo
	
		if (NewIndex > OriginalIndex)
		{
			Expressions.InsertDefaulted(NewIndex + 1);
			Expressions.Swap(OriginalIndex, NewIndex + 1);
			Expressions.RemoveAt(OriginalIndex);
	
			ParentIndexMapping.InsertDefaulted(NewIndex + 1);
			ParentIndexMapping.Swap(OriginalIndex, NewIndex + 1);
			ParentIndexMapping.RemoveAt(OriginalIndex);
	
			for (int32 Idx = NewIndex; Idx >= 0; --Idx)
			{
				if (ParentIndexMapping[Idx] > ParentIdx)
				{
					ParentIndexMapping[Idx]--;
				}
			}
		}
		else if (NewIndex < OriginalIndex)
		{
			Expressions.InsertDefaulted(NewIndex);
			Expressions.Swap(OriginalIndex + 1, NewIndex);
			Expressions.RemoveAt(OriginalIndex + 1);
	
			ParentIndexMapping.InsertDefaulted(NewIndex);
			ParentIndexMapping.Swap(OriginalIndex + 1, NewIndex);
			ParentIndexMapping.RemoveAt(OriginalIndex + 1);
	
			for (int32 Idx = NewIndex; Idx < ParentIndexMapping.Num() - 1; ++Idx)
			{
				if (ParentIndexMapping[Idx] > ParentIdx)
				{
					ParentIndexMapping[Idx]++;
				}
			}
		}
	};
	
	int32 OriginalIdx = ExprIdx;
	int32 NewIdx = (ParentIdx + 1) + FMath::Max(0, FMath::Min(SlotIdx, GetNumChildExpressions(ExprIdx) - 1));
	
	TArray<FActorIOExpressionBase*> Childs = GetChildExpressions(OriginalIdx);
	//TArray<FActorIOExpressionBase*> NewChilds = GetChildExpressions(NewIdx);
	fnMoveExpression(OriginalIdx, NewIdx);
	
	int32 NumChildsProcessed = 0;
	for (FActorIOExpressionBase* ChildExpr : Childs)
	{
		int32 ChildIdx = GetExpressionIdx(ChildExpr);
		int32 ChildTargetIdx = NewIdx + 1 + NumChildsProcessed;
		fnMoveExpression(ChildIdx, ChildTargetIdx);
	
		ParentIndexMapping[ChildTargetIdx] = NewIdx;
		NumChildsProcessed++;
	}
	
	//for (int32& ChildIdx : ChildExprIdxs)
	//{
	//	if (ChildIdx > NewIdx)
	//	{
	//		ChildIdx++;
	//	}
	//	else if (ChildIdx < NewIdx)
	//	{
	//		ChildIdx--;
	//	}
	//}
	
	//for (int32& OldParentIdx : ParentIndexMapping)
	//{
	//	if (OldParentIdx == OriginalIdx)
	//	{
	//		OldParentIdx = NewIdx;
	//	}
	//	else if (OldParentIdx >= NewIdx)
	//	{
	//		OldParentIdx++;
	//	}
	//}
	
	//TArray<int32> ChildExprIdxs = GetChildExpressionIdxs(NewIdx);
	//int32 ChildsProcessed = 0;
	//for (const int32 ChildIdx : ChildExprIdxs)
	//{
	//	MoveExpressionTo(ChildIdx, NewIdx, ChildsProcessed);
	//	ChildsProcessed++;
	//}
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

int32 FActorIOExpressionContainer::GetExpressionParentIdx(int32 Idx) const
{
	return ParentIndexMapping[Idx]; // #todo: return bool to support error checking?
}

int32 FActorIOExpressionContainer::GetExpressionSlotIdx(int32 Idx) const
{
	int32 ParentIdx = ParentIndexMapping[Idx];
	TArray<int32> ChildIdxs = GetChildExpressionIdxs(ParentIdx);
	return ChildIdxs.IndexOfByKey(Idx);
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

int32 FActorIOExpressionContainer::GetNumChildExpressions(int32 ExprIdx) const
{
	int32 OutNumExpressions = 0;

	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx)
	{
		if (ParentIndexMapping[Idx] == ExprIdx && Expressions[Idx].IsValid())
		{
			OutNumExpressions++;
		}
	}

	return OutNumExpressions;
}

bool FActorIOExpressionContainer::IsExpressionDescendantOf(int32 ExprIdx, int32 AncestorIdx) const
{
	int32 Steps = 0;
	for (int32 Idx = ExprIdx; Idx != INDEX_NONE; Idx = ParentIndexMapping[Idx])
	{
		if (Idx == AncestorIdx)
		{
			return true;
		}

		if (!ensureMsgf(++Steps <= Expressions.Num(), TEXT("Cycle detected in expression parent mapping.")))
		{
			return false;
		}
	}

	return false;
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

	FixupContainerReferencesEvent.Broadcast();
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

bool FActorIOExpressionContainer::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	bool bValue = false;
	if (FParse::Bool(Buffer, TEXT("bIsConditionContainer="), bValue))
	{
		// do not allow importing from a different container type
		if (bValue != bIsConditionContainer)
		{
			return true;
		}
	}

	// falls back to default tagged property import
	return false;
}

#if WITH_EDITOR
bool FActorIOExpressionContainer::HasAnyErrors() const
{
	for (const TInstancedStruct<FActorIOExpressionBase>& Expr : Expressions)
	{
		if (Expr->GetTypeName() == TEXT("KismetFunction"))
		{
			const FActorIOKismetFunctionExpression* FunctionExpr = Expr.GetPtr<FActorIOKismetFunctionExpression>();
			if (FunctionExpr->CheckArgumentsNeedUpdate())
			{
				return true;
			}
		}
	}

	return false;
}
#endif

void FActorIOExpressionContainer::DebugDumpContainerData() const
{
	UE_LOG(LogActorIO, Log, TEXT("Dumping expression container data:"));

	int32 NumExpression = GetNumExpressions();
	if (NumExpression == 0)
	{
		UE_LOG(LogActorIO, Log, TEXT("  Empty"));
		return;
	}

	TArray<FString> ExprLines;
	int32 LongestLineLength = 0;
	for (int32 Idx = 0; Idx != NumExpression; ++Idx)
	{
		const FActorIOExpressionBase* Expr = GetExpressionPtr(Idx);
		const FString NewLine = Expr->GetTypeName().ToString();
		LongestLineLength = FMath::Max(LongestLineLength, NewLine.Len());
		ExprLines.Add(NewLine);
	}

	for (int32 Idx = 0; Idx != NumExpression; ++Idx)
	{
		FString Line = ExprLines[Idx];

		int32 LineCharDiff = LongestLineLength - Line.Len();
		while (LineCharDiff > 0)
		{
			Line.AppendChar(TEXT(' '));
			LineCharDiff--;
		}

		// %-3d prints the index left aligned, and space padded up to minimum of 3 characters.
		const int32 ParentIdx = GetExpressionParentIdx(Idx);
		UE_LOG(LogActorIO, Log, TEXT("  %-3d %s\t%d"), Idx, *Line, ParentIdx);
	}
}

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
