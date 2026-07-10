// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOExpression.h"
#include "ActorIOSubsystemBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceNull.h"

bool FActorIOLiteralExpression::Evaluate(FString& OutResult)
{
	OutResult = LiteralValue;
	return true;
}

bool FActorIOFunctionExpression::Evaluate(FString& OutResult)
{
	OutResult.Empty();

	UClass* ClassPtr = ClassRef.Get();
	if (!ClassPtr)
	{
		// #TODO: log error
		return false;
	}

	FString Cmd = FunctionId.ToString();
	for (TInstancedStruct<FActorIOExpressionBase>& Expr : Args)
	{
		if (!Expr.IsValid())
		{
			UE_LOG(LogActorIO, Error, TEXT("Encountered an invalid expression!"));
			return false;
		}

		FActorIOExpressionBase& ExprRef = Expr.GetMutable();

		FString Result;
		if (!ExprRef.Evaluate(Result))
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
