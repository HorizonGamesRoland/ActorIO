// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Expressions/ActorIOExpressionContainer.h"
#include "Expressions/ActorIOExpressionBase.h"
#include "ActorIOVersions.h"

FActorIOExpressionContainer::FActorIOExpressionContainer()
{
	bIsConditionContainer = false;
}

bool FActorIOExpressionContainer::InitFromString(const FString& Str)
{
	if (!IsEmpty())
	{
		Empty();
	}

	int32 ExprIdx = AddExpressionFromString(Str, INDEX_NONE);
	return ExprIdx != INDEX_NONE;
}

FString FActorIOExpressionContainer::ToString() const
{
	const FActorIOExpressionBase* RootExpr = GetExpressionPtr(0);
	if (RootExpr)
	{
		return RootExpr->ToString();
	}

	return TEXT("");
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

	if (ParentIdx != INDEX_NONE && !Expressions[ParentIdx]->CanContainChildExpressions())
	{
		// #todo: error
		return INDEX_NONE;
	}

	int32 ExprIdx = Expressions.Add(Expr);
	ParentIndexMapping.Add(ParentIdx);

	Expressions[ExprIdx]->SetContainer(this);
	if (ExprIdx != 0)
	{
		// Move the expression to its slot before initialization so that
		// child expressions can be created during init with a fix parent idx.
		ExprIdx = MoveExpressionTo(ExprIdx, ParentIdx, SlotIdx);
	}

	if (!Expressions[ExprIdx]->Init())
	{
		RemoveExpression(ExprIdx);
		return INDEX_NONE;
	}

	return ExprIdx;
}

int32 FActorIOExpressionContainer::AddExpressionFromString(const FString& Str, int32 ParentIdx, int32 SlotIdx)
{
	FString Prefix;
	FString Data;
	if (!FActorIOExpressionParser::ParseBracket(Str, Prefix, Data))
	{
		return INDEX_NONE;
	}

	TInstancedStruct<FActorIOExpressionBase> NewExprInstance = FActorIOExpressionParser::NewExpressionFromKeyword(Prefix);
	if (!NewExprInstance.IsValid())
	{
		return INDEX_NONE;
	}

	int32 ExprIdx = AddExpression(NewExprInstance, ParentIdx, SlotIdx);
	if (ExprIdx == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	if (!Expressions[ExprIdx]->InitFromString(Str)) // #todo: is it an error if Data is empty?
	{
		RemoveExpression(ExprIdx);
		return INDEX_NONE;
	}

	return ExprIdx;
}

void FActorIOExpressionContainer::RemoveExpression(int32 ExprIdx)
{
	if (Expressions.IsValidIndex(ExprIdx))
	{
		RemoveChildExpressions(ExprIdx);

		if (Expressions[ExprIdx].IsValid())
		{
			Expressions[ExprIdx]->SetContainer(nullptr);
		}

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
	for (TInstancedStruct<FActorIOExpressionBase>& Expr : Expressions)
	{
		if (Expr.IsValid())
		{
			Expr->SetContainer(nullptr);
		}
	}

	Expressions.Empty();
	ParentIndexMapping.Empty();
}

int32 FActorIOExpressionContainer::MoveExpressionTo(int32 ExprIdx, int32 ParentIdx, int32 SlotIdx)
{
	const int32 Num = Expressions.Num();
	if (!ensure(Num == ParentIndexMapping.Num())) // #todo: make the same check everywhere
	{
		return INDEX_NONE;
	}

	if (!Expressions.IsValidIndex(ExprIdx) || !Expressions.IsValidIndex(ParentIdx))
	{
		return INDEX_NONE;
	}

	// Validate the whole parent mapping first (everything below follows these values).
	for (int32 Idx = 0; Idx != Num; ++Idx)
	{
		const int32 Parent = ParentIndexMapping[Idx];
		if (Parent != INDEX_NONE && !ensureMsgf(Expressions.IsValidIndex(Parent), TEXT("Expression %d has out of range parent index %d."), Idx, Parent))
		{
			return INDEX_NONE;
		}
	}

	// The root cannot be moved.
	if (ParentIndexMapping[ExprIdx] == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	// Prevent cycles: the new parent must not be the expression itself or one of its descendants.
	if (IsExpressionDescendantOf(ParentIdx, ExprIdx))
	{
		return INDEX_NONE;
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
		return INDEX_NONE;
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
		return INDEX_NONE;
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

	return OldToNew[ExprIdx];
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
	const int32 ParentIdx = ParentIndexMapping[Idx];
	const TArray<int32> ChildIdxs = GetChildExpressionIdxs(ParentIdx);
	return ChildIdxs.IndexOfByKey(Idx);
}

bool FActorIOExpressionContainer::IsEmpty() const
{
	return Expressions.IsEmpty(); // #todo: keep checking ParentIndexMapping.IsEmpty?
}

bool FActorIOExpressionContainer::IsValidIndex(int32 Idx) const
{
	return Expressions.IsValidIndex(Idx); // #todo: keep checking for ParentIndexMapping.IsValidIndex?
}

int32 FActorIOExpressionContainer::GetNumChildExpressions(int32 ExprIdx, bool bRecursive) const
{
	int32 OutNumExpressions = 0;

	ensure(Expressions.Num() == ParentIndexMapping.Num()); // #todo: convert to ensureMsgf

	if (Expressions.IsValidIndex(ExprIdx))
	{
		for (int32 Idx = ExprIdx + 1; Idx != Expressions.Num(); ++Idx)
		{
			if (ParentIndexMapping[Idx] == ExprIdx)
			{
				OutNumExpressions++;
				if (bRecursive)
				{
					OutNumExpressions += GetNumChildExpressions(Idx, bRecursive);
				}
			}
		}
	}

	return OutNumExpressions;
}

TArray<int32> FActorIOExpressionContainer::GetChildExpressionIdxs(int32 ExprIdx) const
{
	TArray<int32> OutExpressionIdxs;

	ensure(Expressions.Num() == ParentIndexMapping.Num());

	for (int32 Idx = 0; Idx != Expressions.Num(); ++Idx) // #todo: this is now ordered, so loop from ExprIdx?
	{
		if (ParentIndexMapping[Idx] == ExprIdx && Expressions[Idx].IsValid())
		{
			OutExpressionIdxs.Add(Idx);
		}
	}

	return OutExpressionIdxs;
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
	FActorIOExpressionBase* RootExpr = GetExpressionPtr(0);
	if (RootExpr && Executor)
	{
		FExpressionEvaluationContext Context;
		Context.Executor = Executor;
		Context.SetNamedArgument(TEXT("$Executor"), Executor->GetPathName());

		FString Result;
		RootExpr->Evaluate(Context, Result);

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

bool FActorIOExpressionContainer::HasAnyErrors(FString& OutErrorCode) const
{
	for (const TInstancedStruct<FActorIOExpressionBase>& Expr : Expressions)
	{
		if (!Expr.IsValid())
		{
			OutErrorCode = TEXT("InvalidExpression");
			return true;
		}

		OutErrorCode = Expr->CheckForErrors();
		if (!OutErrorCode.IsEmpty())
		{
			return true;
		}
	}

	return false;
}

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
