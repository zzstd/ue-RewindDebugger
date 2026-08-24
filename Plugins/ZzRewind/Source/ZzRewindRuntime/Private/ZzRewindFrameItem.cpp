// Copyright 2024-2026 zz studio. All Rights Reserved.

#include "ZzRewindFrameItem.h"

#include "Widgets/SRewindFrameRow.h"
#include <Styling/SlateIconFinder.h>


namespace ZZ::Rewind
{

FRewindFrameItem::FRewindFrameItem()
{
	Icon = FSlateIconFinder::FindIconForClass(UObject::StaticClass());
}

TSharedRef<SWidget> FRewindFrameItem::GenerateHeadWidget()
{
	return
		SAssignNew(HeadBox, SBox)
		.HeightOverride(this, &FRewindFrameItem::GetRowHeightOptSize)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &FRewindFrameItem::GetDisplayName)
			.Font_Lambda([this]()
			{
				return IsHovered()
					? FCoreStyle::GetDefaultFontStyle("Bold", 10)
					: FCoreStyle::GetDefaultFontStyle("Regular", 10);
			})
		];
}

TSharedRef<SWidget> FRewindFrameItem::GenerateTrackWidget()
{
	return
		SAssignNew(TrackBox, SBox)
		.HeightOverride(this, &FRewindFrameItem::GetRowHeightOptSize)
		.VAlign(VAlign_Fill)
		[
			/*SNew(SBox)
			.HeightOverride(12)
			[*/
				SAssignNew(FrameRowWidget, SRewindFrameRow, SharedThis(this))
				.ToolTipText(this, &FRewindFrameItem::PrivateGetTooltipText)
			//]
		];
}

TSharedRef<SWidget> FRewindFrameItem::GenerateInspector(int32 Frame)
{
	return SNullWidget::NullWidget;
}

FText FRewindFrameItem::GetTooltipText(int32 Frame) const
{
	return FText();
}

FText FRewindFrameItem::PrivateGetTooltipText() const
{
	if (FrameRowWidget.IsValid())
	{
		if (FrameRowWidget.Pin()->HoveredFrame.IsSet())
		{
			return GetTooltipText(FrameRowWidget.Pin()->HoveredFrame.GetValue());
		}
	}

	return FText();
}

FOptionalSize FRewindFrameItem::GetRowHeightOptSize() const
{
	return 20.f;
}

bool FRewindFrameItem::IsHovered() const
{
	if (HeadBox.IsValid() && TrackBox.IsValid())
	{
		return HeadBox.Pin()->IsHovered() || TrackBox.Pin()->IsHovered();
	}

	return false;
}

}