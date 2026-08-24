// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


namespace ZZ::Rewind
{


class FRewindItem;


class FTimelineRow : public TSharedFromThis<FTimelineRow>
{
	using RowPtr = TSharedPtr<FTimelineRow>;
	using ItemPtr = TSharedPtr<FRewindItem>;
public:
	ItemPtr Item;

	TWeakPtr<FTimelineRow> Parent;
	
	TArray<RowPtr> Children;
	TArray<RowPtr> FilteredChildren;

	bool ShouldShow() const;
	void RefreshFilteredChildren();
	
	RowPtr FindRow(const ItemPtr& InItem, bool bRecursive) const;
	RowPtr FindOrCreateRow(const ItemPtr& InItem);

	RowPtr AddChildRow(const ItemPtr& InItem);
	void ClearChildren();

	int32 GetDepth() const;
};

class ZZREWINDRUNTIME_API STimelineView : public SCompoundWidget
{
	using TreeItem = TSharedPtr<FTimelineRow>;
public:
	SLATE_BEGIN_ARGS(STimelineView)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void RequestRefresh();
private:
	void BuildItems(TSharedRef<FRewindItem> InItem);
	void Cleanup();
	void OnPreAddItem(TSharedRef<FRewindItem> InItem, UObject*, bool bPrimary);
	void CreateRow(TSharedRef<FRewindItem> InItem);
	
	TreeItem Root;

	// track + head
	TSharedPtr<STreeView<TreeItem>> TreeView;
	
	TSharedRef<SHeaderRow> ConstructTreeHeader();
	TSharedRef<ITableRow> TreeGenerateRow(TreeItem InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TreeItem InItem, TArray<TreeItem>& OutChildren);
	void TreeSelectionChanged(TreeItem InItem, ESelectInfo::Type SelectInfo);
	void TreeMouseDoubleClick(TreeItem InItem);
	TSharedPtr<SWidget> OnGetContextMenuContent();
	
	TSharedPtr<class FZzRewindRuntime> Debugger;
};

}