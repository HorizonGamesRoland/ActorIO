// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpression.h"
#include "ActorIOSubsystemBase.h"
#include "ActorIOVersions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Serialization/NameAsStringProxyArchive.h"
#include "Serialization/Formatters/BinaryArchiveFormatter.h"
#include "Misc/OutputDeviceNull.h"

//=======================================================
//~ Begin FActorIOExpressionBase
//=======================================================

FActorIOExpressionBase::FActorIOExpressionBase()
{
	ParentExpr = nullptr;
}

void FActorIOExpressionBase::Serialize(FArchive& Ar)
{
	EActorIOExpressionType Type = GetType();
	FString SubType = GetSubType().ToString(); // can't serialize FNames directly
	Ar << Type;
	Ar << SubType;
}

//=======================================================
//~ Begin FActorIOLiteralExpression
//=======================================================

bool FActorIOLiteralExpression::Evaluate(FString& OutResult)
{
	OutResult = LiteralValue;
	return true;
}

void FActorIOLiteralExpression::Serialize(FArchive& Ar)
{
	FActorIOExpressionBase::Serialize(Ar);
	Ar << LiteralValue;
}

//=======================================================
//~ Begin FActorIOFunctionExpressionBase
//=======================================================

FActorIOFunctionExpressionBase::~FActorIOFunctionExpressionBase()
{
	Args.Empty();
}

void FActorIOFunctionExpressionBase::Serialize(FArchive& Ar)
{
	FActorIOExpressionBase::Serialize(Ar);

	if (Ar.IsLoading() && !Args.IsEmpty())
	{
		Args.Empty();
	}

	int32 NumArgs = Args.Num();
	Ar << NumArgs;

	for (int32 ArgIdx = 0; ArgIdx != NumArgs; ++ArgIdx)
	{
		const int64 DataSizePosition = Ar.Tell();
		int64 DataSize = 0;

		// Pre-serialize the data size. We'll rewrite this after serializing the expression.
		Ar << DataSize;

		const int64 BeginDataPosition = Ar.Tell();

		if (Ar.IsLoading())
		{
			// Skip if no data was serialized for this expression.
			if (DataSize <= 0) continue;

			EActorIOExpressionType Type = EActorIOExpressionType::Invalid;
			FString SubType;
			Ar << Type;
			Ar << SubType;

			FActorIOExpressionBase* NewArg = FActorIOExpresionHelper::CreateExpressionFromType(Type, FName(SubType));
			UE_CLOG(!NewArg, LogActorIO, Warning, TEXT("Could not create expression from type '%s:%s'."), *UEnum::GetValueAsString(Type), *SubType);

			// Go back to the start before attempting to serialize the expression.
			Ar.Seek(BeginDataPosition);

			if (NewArg)
			{
				AddArgument(NewArg);
				NewArg->Serialize(Ar);
			}

			// Not sure how to recover from this.
			checkf(Ar.Tell() <= BeginDataPosition + DataSize, TEXT("Serialized more data then expected when loading an expression!"));

			// Seek to the end of the data block in case we serialized less.
			Ar.Seek(BeginDataPosition + DataSize);
		}
		else if (Ar.IsSaving())
		{
			FActorIOExpressionBase* Arg = Args[ArgIdx];
			if (Arg)
			{
				Arg->Serialize(Ar);
			}

			// Seek back and re-write the data size with the actual size.
			const int64 EndDataPosition = Ar.Tell();
			DataSize = EndDataPosition - BeginDataPosition;

			Ar.Seek(DataSizePosition);
			Ar << DataSize;
			Ar.Seek(EndDataPosition);
		}
	}
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

void FActorIOKismetFunctionExpression::Serialize(FArchive& Ar)
{
	FActorIOFunctionExpressionBase::Serialize(Ar);

	FSoftObjectPath ClassPath;
	if (Ar.IsSaving() && ClassPtr.IsValid())
	{
		ClassPath = ClassPtr.Pin().Get();
	}

	Ar << ClassPath;
	Ar << FunctionId;

	if (Ar.IsLoading())
	{
		ClassPath.FixupCoreRedirects();
		if (ClassPath.IsValid())
		{
			UClass* ResolvedClass = Cast<UClass>(ClassPath.ResolveObject());
			if (ResolvedClass)
			{
				ClassPtr = ResolvedClass;
			}
		}
	}
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

//=======================================================
//~ Begin FActorIOScriptCondition
//=======================================================

bool FActorIOScriptCondition::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FActorIOExpressionVersion::GUID);

	int32 Version = Ar.CustomVer(FActorIOExpressionVersion::GUID);
	Ar << Version;

	if (Ar.IsLoading())
	{
		Ar.SetCustomVersion(FActorIOExpressionVersion::GUID, Version, TEXT("ActorIOExpressionVer"));
	}

	Expr.Serialize(Ar);
	return true;
}

FActorIOExpressionBase* FActorIOExpresionHelper::CreateExpressionFromType(EActorIOExpressionType InType, FName InSubType)
{
	if (InType == EActorIOExpressionType::Literal)
	{
		return new FActorIOLiteralExpression();
	}
	else if (InType == EActorIOExpressionType::Function)
	{
		if (InSubType == FName("KismetFunction"))
		{
			return new FActorIOKismetFunctionExpression();
		}
		else if (InSubType == FName("Group"))
		{
			return new FActorIOGroupExpression();
		}
	}

	return nullptr;
}
