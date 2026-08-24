// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Widgets/STimelineView.h"
#include "Widgets/STimelineSlider.h"
#include "Widgets/STimelineTableRow.h"
#include "Widgets/STimelineToolbar.h"

#include "ZzRewindRuntime.h"
#include "ZzRewindItem.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
namespace ZZ::Rewind
{

bool FTimelineRow::ShouldShow() const
{
	if (Item->bHidden || Item->bFiltered)
	{
		return false;
	}

	return true;
}

void FTimelineRow::RefreshFilteredChildren()
{
	FilteredChildren.Empty();

	for (auto Child : Children)
	{
		if (Child->ShouldShow())
		{
			FilteredChildren.Add(Child);
			Child->RefreshFilteredChildren();
		}
	}
}

FTimelineRow::RowPtr FTimelineRow::FindRow(const ItemPtr& InItem, bool bRecursive) const
{
	for (auto Row : Children)
	{
		if (Row->Item == InItem)
		{
			return Row;
		}

		if (bRecursive)
		{
			if (auto Find = Row->FindRow(InItem, bRecursive))
			{
				return Find;
			}
		}
	}

	return nullptr;
}

FTimelineRow::RowPtr FTimelineRow::FindOrCreateRow(const ItemPtr& InItem)
{
	auto Result = FindRow(InItem, true);
	if (!Result.IsValid())
	{
		Result = AddChildRow(InItem);
	}

	check(Result.IsValid());
	return Result;
}

FTimelineRow::RowPtr FTimelineRow::AddChildRow(const ItemPtr& InItem)
{
	auto NewRow = MakeShared<FTimelineRow>();
	NewRow->Item = InItem;
	NewRow->Parent = SharedThis(this);

	Children.Add(NewRow);
	if (NewRow->ShouldShow())
	{
		FilteredChildren.Add(NewRow);
	}

	return NewRow;
}

void FTimelineRow::ClearChildren()
{
	for (auto Row : Children)
	{
		Row->ClearChildren();
	}

	Children.Empty();
	FilteredChildren.Empty();
}

int32 FTimelineRow::GetDepth() const
{
	int32 Depth = 0;
	TWeakPtr<FTimelineRow> CurParent = Parent;
	while (CurParent.IsValid())
	{
		Depth++;
		CurParent = CurParent.Pin()->Parent;
	}
	
	return Depth - 1;
}

void STimelineView::Construct(const FArguments& InArgs)
{
	Debugger = FZzRewindRuntime::Get();
	Root = MakeShared<FTimelineRow>();
	
	auto MyScrollBar = SNew(SScrollBar).Thickness(FVector2D(6.0f, 6.0f));
	
	float ToolBarHeight = 20;
	ChildSlot
	[
		SNew(SOverlay)
		
		+ SOverlay::Slot()
		[
			SAssignNew(TreeView, STreeView<TreeItem>)
			.HeaderRow(ConstructTreeHeader())
			.TreeItemsSource(&Root.Get()->FilteredChildren)
			.SelectionMode(ESelectionMode::SingleToggle)
			.EnableAnimatedScrolling(true)
			.ExternalScrollbar(MyScrollBar)
			.OnGenerateRow(this, &STimelineView::TreeGenerateRow)
			.OnGetChildren(this, &STimelineView::OnGetChildren)
			.OnSelectionChanged(this, &STimelineView::TreeSelectionChanged)
			.OnContextMenuOpening(this, &STimelineView::OnGetContextMenuContent)
			.OnMouseButtonDoubleClick(this, &STimelineView::TreeMouseDoubleClick)
		]
		
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		.Padding(0, ToolBarHeight, 0, 0)
		[
			MyScrollBar
		]
	];

	for (auto Item : Debugger->PrimaryItems)
	{
		BuildItems(Item.ToSharedRef());
	}

	Debugger->OnCleanupDelegate.AddSP(this, &STimelineView::Cleanup);
	
	/*
	 * Use OnPreAddItemDelegate to ensure that the Parent is created first, 
	 * even if the Item has not been set up at this time
	 */
	Debugger->OnPreAddItemDelegate.AddSP(this, &STimelineView::OnPreAddItem);
}

void STimelineView::RequestRefresh()
{
	Root->RefreshFilteredChildren();
	TreeView->RequestTreeRefresh();
}

void STimelineView::BuildItems(TSharedRef<FRewindItem> InItem)
{
	CreateRow(InItem);
	for (auto Child : InItem->GetChildren())
	{
		CreateRow(Child.ToSharedRef());
		BuildItems(Child.ToSharedRef());
	}
}

void STimelineView::Cleanup()
{
	Root->ClearChildren();
	TreeView->RequestTreeRefresh();
}

void STimelineView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	Debugger->UpdateSmoothViewRange(InDeltaTime);

	if (Debugger->bRequestViewRefresh)
	{
		Debugger->bRequestViewRefresh = false;
		RequestRefresh();
	}
}


void STimelineView::OnPreAddItem(TSharedRef<FRewindItem> InItem, UObject*, bool bPrimary)
{
	check(InItem->GetParent().IsValid() != bPrimary);
	CreateRow(InItem);
}

void STimelineView::CreateRow(TSharedRef<FRewindItem> InItem)
{
	TSharedPtr<FTimelineRow> NewRow;
	
	if (auto ParentItem = InItem->GetParent())
	{
		auto ParentRow = Root->FindRow(ParentItem, true);
		check(ParentRow);
		
		if (ParentRow)
		{
			NewRow = ParentRow->AddChildRow(InItem);
		}
	}
	else
	{
		NewRow = Root->AddChildRow(InItem);
	}

	check(NewRow);

	TreeView->SetItemExpansion(NewRow, true);
	TreeView->RequestTreeRefresh();
}

TSharedRef<SHeaderRow> STimelineView::ConstructTreeHeader()
{
	auto HeaderRow = SNew(SHeaderRow)
		.ResizeMode(ESplitterResizeMode::Fill)
		.CanSelectGeneratedColumn(false);

	{
		SHeaderRow::FColumn::FArguments NameArgs;
		NameArgs.ColumnId("Track")
		.FillWidth(1.f)
		.ShouldGenerateWidget(true)
		[
			SNew(SBox).HeightOverride(20)
			[
				SNew(STimelineSlider)
			]
		];
		HeaderRow->AddColumn(NameArgs);
	}
	
	{
		SHeaderRow::FColumn::FArguments NameArgs;
		NameArgs.ColumnId("Head")
		.FillWidth(.3f)
		.ShouldGenerateWidget(true)
		[
			SNew(STimelineToolbar)
		];
		HeaderRow->AddColumn(NameArgs);
	}
	

	return HeaderRow;
}

TSharedRef<ITableRow> STimelineView::TreeGenerateRow(TreeItem InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STimelineTableRow, OwnerTable)
		.Item(InItem)
		.OwnerWidget(SharedThis(this)); 
}

void STimelineView::OnGetChildren(TreeItem InItem, TArray<TreeItem>& OutChildren)
{
	if(InItem)
	{
		OutChildren = InItem->FilteredChildren;
	}
}

void STimelineView::TreeSelectionChanged(TreeItem InItem, ESelectInfo::Type SelectInfo)
{
	Debugger->SetSelectedItem(InItem ? InItem->Item : nullptr);
}

void STimelineView::TreeMouseDoubleClick(TreeItem InItem)
{
}

TSharedPtr<SWidget> STimelineView::OnGetContextMenuContent()
{
	return SNullWidget::NullWidget;
}

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION