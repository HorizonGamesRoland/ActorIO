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
		UE_LOG(LogActorIO, Error, TEXT("FActorIOExpressionContainer: Attempted to add an expression (%s) as root, but root already exists."), *Expr->GetTypeName().ToString());
		return INDEX_NONE;
	}

	if (ParentIdx != INDEX_NONE && !Expressions.IsValidIndex(ParentIdx))
	{
		UE_LOG(LogActorIO, Error, TEXT("FActorIOExpressionContainer: Attempted to add an expression (%s) to an invalid parent (%d) index."), *Expr->GetTypeName().ToString(), ParentIdx);
		return INDEX_NONE;
	}

	if (ParentIdx != INDEX_NONE && !Expressions[ParentIdx]->CanContainChildExpressions())
	{
		UE_LOG(LogActorIO, Error, TEXT("FActorIOExpressionContainer: Attempted to add an expression (%s) to a parent (%d) that has CanContainChildExpressions() == false."), *Expr->GetTypeName().ToString(), ParentIdx);
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
		UE_LOG(LogActorIO, Error, TEXT("FActorIOExpressionContainer: Expression (%s) Init() failed during add."), *Expr->GetTypeName().ToString());
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
		UE_LOG(LogActorIO, Error, TEXT("FActorIOExpressionContainer: Failed to parse expression to add: %s"), *Str);
		return INDEX_NONE;
	}

	TInstancedStruct<FActorIOExpressionBase> Expr = FActorIOExpressionParser::NewExpressionFromKeyword(Prefix);
	if (!Expr.IsValid())
	{
		UE_LOG(LogActorIO, Error, TEXT("FActorIOExpressionContainer: Failed to add expression from keyword: %s"), *Prefix);
		return INDEX_NONE;
	}

	int32 ExprIdx = AddExpression(Expr, ParentIdx, SlotIdx);
	if (ExprIdx == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	if (!Expressions[ExprIdx]->InitFromString(Str))
	{
		UE_LOG(LogActorIO, Error, TEXT("FActorIOExpressionContainer: Expression (%s) InitFromString() failed during add. String was: %s."), *Expr->GetTypeName().ToString(), *Str);
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
	if (!ensureMsgf(Num == ParentIndexMapping.Num(), TEXT("Expression count and parent index count is mismatching! Container is corrupt.")))
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
	const int32 Num = Expressions.Num();
	if (!ensureMsgf(Num == ParentIndexMapping.Num(), TEXT("Expression count and parent index count is mismatching! Container is corrupt.")))
	{
		return INDEX_NONE;
	}

	return ParentIndexMapping[Idx];
}

int32 FActorIOExpressionContainer::GetExpressionSlotIdx(int32 Idx) const
{
	const int32 ParentIdx = ParentIndexMapping[Idx];
	const TArray<int32> ChildIdxs = GetChildExpressionIdxs(ParentIdx);
	return ChildIdxs.IndexOfByKey(Idx);
}

bool FActorIOExpressionContainer::IsEmpty() const
{
	return Expressions.IsEmpty();
}

bool FActorIOExpressionContainer::IsValidIndex(int32 Idx) const
{
	return Expressions.IsValidIndex(Idx);
}

int32 FActorIOExpressionContainer::GetNumChildExpressions(int32 ExprIdx, bool bRecursive) const
{
	const int32 Num = Expressions.Num();
	if (!ensureMsgf(Num == ParentIndexMapping.Num(), TEXT("Expression count and parent index count is mismatching! Container is corrupt.")))
	{
		return 0;
	}

	int32 OutNumExpressions = 0;
	if (Expressions.IsValidIndex(ExprIdx))
	{
		for (int32 Idx = ExprIdx + 1; Idx != Num; ++Idx)
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
	const int32 Num = Expressions.Num();
	if (!ensureMsgf(Num == ParentIndexMapping.Num(), TEXT("Expression count and parent index count is mismatching! Container is corrupt.")))
	{
		return TArray<int32>();
	}

	TArray<int32> OutExpressionIdxs;
	if (Expressions.IsValidIndex(ExprIdx))
	{
		for (int32 Idx = ExprIdx + 1; Idx != Num; ++Idx)
		{
			if (ParentIndexMapping[Idx] == ExprIdx)
			{
				OutExpressionIdxs.Add(Idx);
			}
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
		if (ExprInstance.IsValid())
		{
			ExprInstance->SetContainer(this);
		}
	}

	FixupContainerReferencesEvent.Broadcast();
}

bool FActorIOExpressionContainer::Evaluate(UObject* Executor)
{
	const int32 Num = Expressions.Num();
	if (!ensureMsgf(Num == ParentIndexMapping.Num(), TEXT("Expression count and parent index count is mismatching! Container is corrupt.")))
	{
		return false;
	}

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
	
	// Doesn't actually serialize, just write the custom version for PostSerialize (for use in the future).
	return false;
}

void FActorIOExpressionContainer::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading() || Ar.IsTransacting())
	{
		// Automatically fixup container references during load and after any transaction.
		// This ensures that the reference is initialized after load, and stays valid after any change happens.
		FixupContainerReferences();
	}
}

bool FActorIOExpressionContainer::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	bool bValue = false;
	if (FParse::Bool(Buffer, TEXT("bIsConditionContainer="), bValue))
	{
		// Do not allow importing from a different container type.
		if (bValue != bIsConditionContainer)
		{
			return true;
		}
	}

	// Falls back to default tagged property import.
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

	const int32 Num = Expressions.Num();
	if (!ensureMsgf(Num == ParentIndexMapping.Num(), TEXT("Expression count and parent index count is mismatching! Container is corrupt.")))
	{
		UE_LOG(LogActorIO, Log, TEXT("  Corrupt. Expression count and parent index count is mismatching!"));
		return;
	}

	if (Num == 0)
	{
		UE_LOG(LogActorIO, Log, TEXT("  Empty"));
		return;
	}

	TArray<FString> ExprTypes;
	TArray<FString> ExprValues;
	int32 LongestTypeLength = 0;
	for (int32 Idx = 0; Idx != Num; ++Idx)
	{
		const FActorIOExpressionBase* Expr = GetExpressionPtr(Idx);
		const FString& TypeStr = ExprTypes.Add_GetRef(Expr->GetTypeName().ToString());
		const FString& ValueStr = ExprValues.Add_GetRef(Expr->ToString());
		LongestTypeLength = FMath::Max(LongestTypeLength, TypeStr.Len());
	}

	for (int32 Idx = 0; Idx != Num; ++Idx)
	{
		FString TypeStr = ExprTypes[Idx];
		const FString& ValueStr = ExprValues[Idx];

		int32 LengthDiff = LongestTypeLength - TypeStr.Len();
		while (LengthDiff > 0)
		{
			TypeStr.AppendChar(TEXT(' '));
			LengthDiff--;
		}

		// %-3d prints the index left aligned, and space padded up to minimum of 3 characters.
		const int32 ParentIdx = GetExpressionParentIdx(Idx);
		UE_LOG(LogActorIO, Log, TEXT("  %-3d %s\t%-3d %s"), Idx, *TypeStr, ParentIdx, *ValueStr);
	}
}
