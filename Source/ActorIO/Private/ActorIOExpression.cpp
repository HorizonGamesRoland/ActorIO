// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpression.h"
#include "ActorIOSubsystemBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceNull.h"

//=======================================================
//~ Begin FActorIOLiteralExpression
//=======================================================

bool FActorIOLiteralExpression::Evaluate(FString& OutResult)
{
	OutResult = LiteralValue;
	return true;
}

//=======================================================
//~ Begin FActorIOFunctionExpressionBase
//=======================================================

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
	if (InExpr && Args.IsValidIndex(Index))
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

	if (!ClassPtr)
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
	return IOSubsystem->ExecuteCommand(ClassPtr->GetDefaultObject(), *Cmd, Ar, IOSubsystem, &OutResult);
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
	if (ClassPtr && FunctionId != NAME_None)
	{
		return ClassPtr->FindFunctionByName(FunctionId);
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
