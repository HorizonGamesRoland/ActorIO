// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Expressions/ActorIOExpressionBase.h"
#include "Expressions/ActorIOExpressionContainer.h"
#include "Expressions/ActorIOExpressionRegistry.h"
#include "ActorIOSubsystemBase.h"

//=======================================================
//~ Begin FExpressionEvaluationContext
//=======================================================

void FExpressionEvaluationContext::SetNamedArgument(const FString& InName, const FString& InValue)
{
	if (InName.IsEmpty() || !InName.StartsWith(NAMEDARGUMENT_PREFIX))
	{
		UE_LOG(LogActorIO, Error, TEXT("FExpressionEvaluationContext: Attempted to set named argument without the required '%s' prefix. Name was: %s"), NAMEDARGUMENT_PREFIX, *InName);
		return;
	}

	int32 InvalidCharIndex = INDEX_NONE;
	if (InName.FindChar('"', InvalidCharIndex) || InName.FindChar(';', InvalidCharIndex))
	{
		UE_LOG(LogActorIO, Error, TEXT("FExpressionEvaluationContext: Attempted to set named argument with illegal character. Name was: %s"), *InName);
		return;
	}

	if (!InValue.IsEmpty())
	{
		FString& Arg = NamedArguments.FindOrAdd(InName);
		Arg = InValue;
	}
	else
	{
		NamedArguments.Remove(InName);
	}
}

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
		ExprContainer->AddExpressionFromString(ExprData, SelfIdx);
	}

	return true;
}

FString FActorIOFunctionExpressionBase::ChildExpressionsToString() const
{
	FString OutStr;

	FActorIOExpressionContainer* ExprContainer = GetContainer();
	if (!ExprContainer) return OutStr;

	int32 SelfIdx = ExprContainer->GetExpressionIdx(this);
	TArray<int32> ChildExprIdxs = ExprContainer->GetChildExpressionIdxs(SelfIdx);
	for (int32 Idx = 0; Idx != ChildExprIdxs.Num(); ++Idx)
	{
		FActorIOExpressionBase* Expr = ExprContainer->GetExpressionPtr(ChildExprIdxs[Idx]);
		if (Expr)
		{
			if (!OutStr.IsEmpty()) OutStr += TEXT(',');
			OutStr += Expr->ToString();
		}
	}

	return OutStr;
}

//=======================================================
//~ Begin FActorIOExpressionParser
//=======================================================

TInstancedStruct<FActorIOExpressionBase> FActorIOExpressionParser::NewExpressionFromKeyword(FString Keyword)
{
	FActorIOExpressionRegistry& Registry = FActorIOExpressionRegistry::Get();
	for (auto& Elem : Registry.GetExpressions())
	{
		FActorIOExpressionDesc Desc = Elem.Value;
		if (Desc.Keywords.Contains(Keyword))
		{
			return Desc.InstanceDelegate.Execute(Keyword);
		}
	}

	TInstancedStruct<FActorIOExpressionBase> InvalidExprInstance;
	return InvalidExprInstance;
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
