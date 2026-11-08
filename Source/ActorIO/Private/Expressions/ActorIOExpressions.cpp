// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Expressions/ActorIOExpressions.h"
#include "Expressions/ActorIOExpressionContainer.h"
#include "ActorIOSubsystemBase.h"
#include "Misc/OutputDeviceNull.h"

//=======================================================
//~ Begin FActorIOLiteralExpression
//=======================================================

TInstancedStruct<FActorIOExpressionBase> FActorIOLiteralExpression::MakeInstance(const FString& Keyword)
{
	TInstancedStruct<FActorIOExpressionBase> OutInstance;
	OutInstance.InitializeAs<FActorIOLiteralExpression>();
	return OutInstance;
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

bool FActorIOLiteralExpression::Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue)
{
	// Replace named argument with its actual value.
	if (LiteralValue.StartsWith(NAMEDARGUMENT_PREFIX) && InContext.NamedArguments.Contains(LiteralValue))
	{
		const FString NamedArgValue = InContext.NamedArguments[LiteralValue];
		OutReturnValue = NamedArgValue;
	}
	else
	{
		OutReturnValue = LiteralValue;
	}
	
	return true;
}

//=======================================================
//~ Begin FActorIOKismetFunctionExpression
//=======================================================

TInstancedStruct<FActorIOExpressionBase> FActorIOKismetFunctionExpression::MakeInstance(const FString& Keyword)
{
	TInstancedStruct<FActorIOExpressionBase> OutInstance;
	OutInstance.InitializeAs<FActorIOKismetFunctionExpression>();
	return OutInstance;
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

		if (!ResolvedClass)
		{
			UE_LOG(LogActorIO, Error, TEXT("%s - Could not resolve class: %s"), ANSI_TO_TCHAR(__FUNCTION__), *ClassPath.ToString());
			return false;
		}

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

		FString ChildsData = ChildExpressionsToString();
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

bool FActorIOKismetFunctionExpression::Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue)
{
	OutReturnValue.Empty();

	UClass* ClassPtr = FunctionClass.Get();
	UFunction* FunctionPtr = ResolveUFunction();
	if (!FunctionPtr)
	{
		return false;
	}

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<int32> ChildExprIdxs = ExprContainer->GetChildExpressionIdxs(SelfIdx);

	FString Cmd = FunctionName.ToString();
	for (int32 Idx = 0; Idx != ChildExprIdxs.Num(); ++Idx)
	{
		FActorIOExpressionBase* Expr = ExprContainer->GetExpressionPtr(ChildExprIdxs[Idx]);
		FString ReturnValue;
		if (!Expr || !Expr->Evaluate(InContext, ReturnValue))
		{
			return false;
		}

		Cmd += TEXT(" ");
		Cmd += ReturnValue;
	}

	UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(InContext.Executor);
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

FString FActorIOKismetFunctionExpression::CheckForErrors() const
{
	if (FunctionName != NAME_None)
	{
		UClass* ClassPtr = FunctionClass.Get();
		UFunction* FunctionPtr = ResolveUFunction();
		if (ClassPtr && !FunctionPtr)
		{
			return TEXT("FunctionNotFound");
		}
	}

	if (CheckArgumentsNeedUpdate())
	{
		return TEXT("ArgumentsNeedUpdate");
	}

	return TEXT("");
}

void FActorIOKismetFunctionExpression::SetFunctionClass(UClass* InClassPtr, bool bResetFunctionName)
{
	if (FunctionClass != InClassPtr)
	{
		FunctionClass = InClassPtr;
		if (bResetFunctionName)
		{
			FunctionName = NAME_None;
		}

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

	const int32 SelfIdx = ExprContainer->GetExpressionIdx(this);

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

	const int32 SelfIdx = ExprContainer->GetExpressionIdx(this);

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
		// For each function param, try to find a child expression that matches that param.
		// If it exists, make sure it is at the same index as the param (in case param order changed).
		// Otherwise add a new child expression for that param.

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
			TInstancedStruct<FActorIOExpressionBase> NewExprInstance;
			FActorIOLiteralExpression& NewExpr = NewExprInstance.InitializeAs<FActorIOLiteralExpression>();
			NewExpr.SetMetadata(TEXT("PropName"), PropName);
			NewExpr.SetMetadata(TEXT("PropType"), PropType);

			// Auto assign value for world context param.
			// Not initializing blueprint generated "__WorldContext" because the I/O system takes care of that.
			// It is also skipped by IActorIO::GetUFunctionInputParams, so it never shows up here.
			if (PropName == TEXT("WorldContextObject") || PropName == TEXT("WorldContext") || PropName == TEXT("WorldContextObj"))
			{
				NewExpr.SetLiteralValue(TEXT("$Executor"));
			}

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

	// At this point all params have a corresponding child expression.
	// We now just have to remove the excess childs.

	int32 Diff = ChildExprIdxs.Num() - FunctionParams.Num();
	while (Diff > 0)
	{
		ExprContainer->RemoveExpression(ChildExprIdxs.Last());
		ChildExprIdxs.RemoveAt(ChildExprIdxs.Num() - 1);
		Diff--;
	}
}

//=======================================================
//~ Begin FActorIOCompareExpression
//=======================================================

FActorIOCompareExpression::FActorIOCompareExpression()
{
	ComparisonOp = EActorIOComparisonOp::Equal;
}

TInstancedStruct<FActorIOExpressionBase> FActorIOCompareExpression::MakeInstance(const FString& Keyword)
{
	TInstancedStruct<FActorIOExpressionBase> OutInstance;
	FActorIOCompareExpression& CompareExpr = OutInstance.InitializeAs<FActorIOCompareExpression>();
	TOptional<EActorIOComparisonOp> Op = FActorIOCompareExpression::OpFromKeyword(Keyword);
	if (Op.IsSet()) CompareExpr.SetComparisonOp(Op.GetValue());
	return OutInstance;
}

bool FActorIOCompareExpression::Init()
{
	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	const int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	if (!ExprContainer->IsValidIndex(SelfIdx))
	{
		return false;
	}

	ExprContainer->AddExpressionFromString(TEXT("val()"), SelfIdx);
	ExprContainer->AddExpressionFromString(TEXT("val()"), SelfIdx);
	return true;
}

bool FActorIOCompareExpression::InitFromString(const FString& Str)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpressionParser::ParseBracket(Str, Prefix, Data))
	{
		return false;
	}

	TOptional<EActorIOComparisonOp> Op = OpFromKeyword(Prefix);
	if (!Op.IsSet())
	{
		return false;
	}

	ComparisonOp = Op.GetValue();

	if (!Data.IsEmpty())
	{
		InitChildExpressionsFromString(Data);
	}

	return true;
}

FString FActorIOCompareExpression::ToString() const
{
	FString OpStr = OpToKeyword(ComparisonOp);
	FString ChildsData = ChildExpressionsToString();
	return FString::Printf(TEXT("%s(%s)"), *OpStr, *ChildsData);
}

bool FActorIOCompareExpression::Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue)
{
	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	const int32 SelfIdx = ExprContainer->GetExpressionIdx(this);

	TArray<int32> ChildIdxs = ExprContainer->GetChildExpressionIdxs(SelfIdx);
	if (ChildIdxs.Num() != 2)
	{
		return false;
	}

	FString ValueA;
	if (!ExprContainer->GetExpressionPtr(ChildIdxs[0])->Evaluate(InContext, ValueA))
	{
		return false;
	}

	FString ValueB;
	if (!ExprContainer->GetExpressionPtr(ChildIdxs[1])->Evaluate(InContext, ValueB))
	{
		return false;
	}

	bool bReturnValue = false;
	if (ValueA.IsNumeric() && ValueB.IsNumeric())
	{
		const float NumericValueA = FCString::Atof(*ValueA);
		const float NumericValueB = FCString::Atof(*ValueB);
		const float ErrorTolerance = 0.001f;

		switch (ComparisonOp)
		{
		case EActorIOComparisonOp::Less:
			bReturnValue = NumericValueA < NumericValueB;
			break;
		case EActorIOComparisonOp::LessOrEqual:
			bReturnValue = NumericValueA < NumericValueB || FMath::IsNearlyEqual(NumericValueA, NumericValueB, ErrorTolerance);
			break;
		case EActorIOComparisonOp::Equal:
			bReturnValue = FMath::IsNearlyEqual(NumericValueA, NumericValueB, ErrorTolerance);
			break;
		case EActorIOComparisonOp::NotEqual:
			bReturnValue = !FMath::IsNearlyEqual(NumericValueA, NumericValueB, ErrorTolerance);
			break;
		case EActorIOComparisonOp::GreaterOrEqual:
			bReturnValue = NumericValueA > NumericValueB || FMath::IsNearlyEqual(NumericValueA, NumericValueB, ErrorTolerance);
			break;
		case EActorIOComparisonOp::Greater:
			bReturnValue = NumericValueA > NumericValueB;
			break;
		default:
			return false;
		}
	}
	else
	{
		switch (ComparisonOp)
		{
		case EActorIOComparisonOp::Less:
			bReturnValue = ValueA < ValueB;
			break;
		case EActorIOComparisonOp::LessOrEqual:
			bReturnValue = ValueA <= ValueB;
			break;
		case EActorIOComparisonOp::Equal:
			bReturnValue = ValueA == ValueB;
			break;
		case EActorIOComparisonOp::NotEqual:
			bReturnValue = ValueA != ValueB;
			break;
		case EActorIOComparisonOp::GreaterOrEqual:
			bReturnValue = ValueA >= ValueB;
			break;
		case EActorIOComparisonOp::Greater:
			bReturnValue = ValueA > ValueB;
			break;
		default:
			return false;
		}
	}

	if (bNegated)
	{
		bReturnValue = !bReturnValue;
	}

	OutReturnValue = bReturnValue ? TEXT("True") : TEXT("False");
	return true;
}

FString FActorIOCompareExpression::CheckForErrors() const
{
	if (!CheckArgumentCountValid())
	{
		return TEXT("InvalidArgumentCount");
	}

	return TEXT("");
}

FString FActorIOCompareExpression::OpToKeyword(const EActorIOComparisonOp& Op)
{
	switch (Op)
	{
	case EActorIOComparisonOp::Less:
		return TEXT("lt");
	case EActorIOComparisonOp::LessOrEqual:
		return TEXT("le");
	case EActorIOComparisonOp::Equal:
		return TEXT("eq");
	case EActorIOComparisonOp::NotEqual:
		return TEXT("ne");
	case EActorIOComparisonOp::GreaterOrEqual:
		return TEXT("ge");
	case EActorIOComparisonOp::Greater:
		return TEXT("gt");
	default:
		checkNoEntry();
		return TEXT("");
	}
}

TOptional<EActorIOComparisonOp> FActorIOCompareExpression::OpFromKeyword(const FString& Keyword)
{
	if (Keyword == TEXT("lt")) return EActorIOComparisonOp::Less;
	else if (Keyword == TEXT("le")) return EActorIOComparisonOp::LessOrEqual;
	else if (Keyword == TEXT("eq")) return EActorIOComparisonOp::Equal;
	else if (Keyword == TEXT("ne")) return EActorIOComparisonOp::NotEqual;
	else if (Keyword == TEXT("ge")) return EActorIOComparisonOp::GreaterOrEqual;
	else if (Keyword == TEXT("gt")) return EActorIOComparisonOp::Greater;
	return TOptional<EActorIOComparisonOp>();
}

bool FActorIOCompareExpression::CheckArgumentCountValid() const
{
	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	const int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	return ExprContainer->GetNumChildExpressions(SelfIdx) == 2;
}

//=======================================================
//~ Begin FActorIOGroupExpression
//=======================================================

TInstancedStruct<FActorIOExpressionBase> FActorIOGroupExpression::MakeInstance(const FString& Keyword)
{
	TInstancedStruct<FActorIOExpressionBase> OutInstance;
	FActorIOGroupExpression& GroupExpr = OutInstance.InitializeAs<FActorIOGroupExpression>();
	GroupExpr.SetNegated(Keyword == TEXT("or"));
	return OutInstance;
}

bool FActorIOGroupExpression::InitFromString(const FString& Str)
{
	// Example valid strings:
	// - and()
	// - or
	// - and(func(...),func(...))
	// - or(func(...),func(...),eq(...))

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
	FString ChildsData = ChildExpressionsToString();
	if (bNegated)
	{
		return FString::Printf(TEXT("or(%s)"), *ChildsData);
	}
	else
	{
		return FString::Printf(TEXT("and(%s)"), *ChildsData);
	}
}

bool FActorIOGroupExpression::Evaluate(FExpressionEvaluationContext& InContext, FString& OutReturnValue)
{
	OutReturnValue.Empty();

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return false;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<int32> ChildExprIdxs = ExprContainer->GetChildExpressionIdxs(SelfIdx);

	// Empty groups are treated as if they are not even there.
	if (ChildExprIdxs.Num() == 0)
	{
		OutReturnValue = TEXT("True");
		return true;
	}

	bool bReturnValue = true;
	for (int32 Idx = 0; Idx != ChildExprIdxs.Num(); ++Idx)
	{
		FActorIOExpressionBase* Expr = ExprContainer->GetExpressionPtr(ChildExprIdxs[Idx]);
		FString ReturnValue;
		if (!Expr || !Expr->Evaluate(InContext, ReturnValue))
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
