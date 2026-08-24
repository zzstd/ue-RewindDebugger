// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindItem.h"

#include "ZzRewindRuntime.h"


namespace ZZ::Rewind
{

void FRewindItem::OnSetup(UObject* Owner)
{
}

void FRewindItem::OnRecord(int32 Frame)
{
	auto TempChildren = Children;
	for (auto Child : TempChildren)
	{
		Child->OnRecord(Frame);
	}
}

void FRewindItem::OnRewind(UWorld* World, int32 Frame)
{
	auto TempChildren = Children;
	for (auto Child : TempChildren)
	{
		Child->OnRewind(World, Frame);
	}
}

void FRewindItem::OnRewindCleanup(UWorld* World)
{
	auto TempChildren = Children;
	for (auto Child : TempChildren)
	{
		Child->OnRewindCleanup(World);
	}
}

bool FRewindItem::HasTag(FName InTag) const
{
	return Tags.Contains(InTag);
}

void FRewindItem::InternalAddChild(TSharedRef<FRewindItem> NewItem, UObject* Owner)
{
	NewItem->Parent = SharedThis(this);
	Children.Add(NewItem);
	FZzRewindRuntime::Get()->InternalAddItem(NewItem, Owner, false);
}

}