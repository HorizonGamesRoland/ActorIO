// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "StructUtils/InstancedStruct.h"
#include "ActorIOExpression.generated.h"

USTRUCT()
struct ACTORIO_API FActorIOExpressionBase
{
	GENERATED_BODY()

	virtual ~FActorIOExpressionBase() = default;

	virtual bool Evaluate(FString& OutResult) PURE_VIRTUAL(FActorIOExpressionBase::Evaluate, return false;)
};

USTRUCT()
struct ACTORIO_API FActorIOLiteralExpression : public FActorIOExpressionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString LiteralValue;

	virtual bool Evaluate(FString& OutResult) override;
};

USTRUCT()
struct ACTORIO_API FActorIOFunctionExpression: public FActorIOExpressionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<UBlueprintFunctionLibrary> ClassRef;

	UPROPERTY(EditAnywhere)
	FName FunctionId;

	UPROPERTY(EditAnywhere)
	TArray<TInstancedStruct<FActorIOExpressionBase>> Args;

	virtual bool Evaluate(FString& OutResult) override;
};

USTRUCT()
struct ACTORIO_API FActorIOExpressionConditionProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FActorIOFunctionExpression Expression;
};
