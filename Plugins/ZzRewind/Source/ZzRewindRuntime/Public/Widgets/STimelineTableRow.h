// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


namespace ZZ::Rewind
{

class STimelineView;
class FTimelineRow;

class STimelineTableRow : public SMultiColumnTableRow<TSharedPtr<STimelineTableRow>>
{
public:
	SLATE_BEGIN_ARGS(STimelineTableRow) {}
	SLATE_ARGUMENT(TSharedPtr<STimelineView>, OwnerWidget)
	SLATE_ARGUMENT(TSharedPtr<FTimelineRow>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);
	
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
private:
	TSharedPtr<FTimelineRow>	Row;
	TWeakPtr<STimelineView>		OwnerWidgetPtr;
};

// draw Scrub Frame, scale view range
class SZzRewindTimelineRowWrapping : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZzRewindTimelineRowWrapping) {}
	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
private:
	TSharedPtr<class FZzRewindRuntime> Debugger;
};

}