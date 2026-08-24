// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Widgets/STimelineTableRow.h"
#include "Widgets/STimelineView.h"
#include "ZzRewindRuntime.h"
#include "ZzRewindItem.h"
#include "Utils/ZzRewindUtils.h"

#include "SlateOptMacros.h"
#include "Widgets/Images/SLayeredImage.h"


namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

class SJKRewindTimelineExpanderArrow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SJKRewindTimelineExpanderArrow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STimelineTableRow>& InRow)
	{
		Row = InRow;

		ChildSlot
		[
			SNew(SBox)
			.Visibility(this, &SJKRewindTimelineExpanderArrow::GetExpanderVisibility)
			[
				SAssignNew(ExpanderArrow, SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.ClickMethod(EButtonClickMethod::MouseDown)
				.OnClicked(this, &SJKRewindTimelineExpanderArrow::OnExpanderClicked)
				.ContentPadding(0.0f)
				.IsFocusable(false)
				[
					SNew(SImage)
					.Image(this, &SJKRewindTimelineExpanderArrow::GetExpanderImage)
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]
		];
	}
private:
	TWeakPtr<STimelineTableRow> Row;
	TSharedPtr<SButton> ExpanderArrow;
	
	EVisibility GetExpanderVisibility() const
	{
		auto RowPtr = Row.Pin();
		if (!RowPtr.IsValid())
		{
			return EVisibility::Collapsed;
		}

		return RowPtr->DoesItemHaveChildren() ? EVisibility::Visible : EVisibility::Hidden;
		
	}
	
	const FSlateBrush* GetExpanderImage() const
	{
		auto RowPtr = Row.Pin();
		if (!RowPtr.IsValid())
		{
			return FAppStyle::Get().GetBrush("NoBrush");
		}

		static const FName ExpandedName = "TreeArrow_Expanded";
		static const FName CollapsedName = "TreeArrow_Collapsed";
		static const FName ExpandedHoveredName = "TreeArrow_Expanded_Hovered";
		static const FName CollapsedHoveredName = "TreeArrow_Collapsed_Hovered";
		
		FName ResourceName;
		if (RowPtr->IsItemExpanded())
		{
			ResourceName = ExpanderArrow->IsHovered() ? ExpandedHoveredName : ExpandedName;
		}
		else
		{
			ResourceName = ExpanderArrow->IsHovered() ? CollapsedHoveredName : CollapsedName;
		}

		return FAppStyle::Get().GetBrush(ResourceName);
	}
	
	FReply OnExpanderClicked()
	{
		auto RowPtr = Row.Pin();
		if (!RowPtr.IsValid())
		{
			return FReply::Unhandled();
		}

		const FModifierKeysState ModKeyState = FSlateApplication::Get().GetModifierKeys();
		if (ModKeyState.IsShiftDown())
		{
			RowPtr->Private_OnExpanderArrowShiftClicked();
		}
		else
		{
			RowPtr->ToggleExpansion();
		}

		return FReply::Handled();
	}
};

void STimelineTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	Row = InArgs._Item;
	OwnerWidgetPtr = InArgs._OwnerWidget;
	
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> STimelineTableRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == "Head")
	{
		const FSlateIcon ItemIcon = Row->Item->GetIcon();

		const TSharedRef<SLayeredImage> LayeredIcons = SNew(SLayeredImage)
			.DesiredSizeOverride(FVector2D(12, 12))
			.Image(ItemIcon.GetIcon());

		if (ItemIcon.GetOverlayIcon())
		{
			LayeredIcons->AddLayer(ItemIcon.GetOverlayIcon());
		}
		
		return
			SNew(SBorder)
			.Padding(2)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor_Lambda([this]()
			{
				if (IsSelected())
				{
					return FAppStyle::GetSlateColor("SelectionColor").GetSpecifiedColor().CopyWithNewOpacity(0.5f);
				}
				return FLinearColor::White;
			})
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Row->GetDepth() * 8.f, 0,0,0)
				[
					SNew(SJKRewindTimelineExpanderArrow, SharedThis(this))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0)
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						LayeredIcons
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(2, 0)
				[
					Row->Item->GenerateHeadWidget()
				]
			];
	}

	constexpr float HeaderRowHorPadding = 4.f; // this is tree HeaderRow default horizontal padding.
	return SNew(SBox).Padding(FMargin(HeaderRowHorPadding, 0))
		[
			SNew(SZzRewindTimelineRowWrapping)
			[
				Row->Item->GenerateTrackWidget()
			]
		];
}

void SZzRewindTimelineRowWrapping::Construct(const FArguments& InArgs)
{
	SetClipping(EWidgetClipping::ClipToBounds);
	ChildSlot[InArgs._Content.Widget];
	Debugger = FZzRewindRuntime::Get();
}

FReply SZzRewindTimelineRowWrapping::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D ViewRange = Debugger->DesiredViewRange;

	if (MouseEvent.IsControlDown())
	{
		const float MouseFractionX = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X / MyGeometry.GetLocalSize().X;
		const float ZoomDelta = -0.2f * MouseEvent.GetWheelDelta();

	
		const float OutputChange = (ViewRange.Y - ViewRange.X) * ZoomDelta;
	
		const float NewViewMin = ViewRange.X - (OutputChange * MouseFractionX);
		const float NewViewMax = ViewRange.Y + (OutputChange * (1.f - MouseFractionX));
	
		if (NewViewMin < NewViewMax)
		{
			Debugger->SetDesiredViewRange(NewViewMin, NewViewMax);
		}
		return FReply::Handled();
	}
	
	if (MouseEvent.IsShiftDown())
	{
		const float MoveDelta = -0.05f * MouseEvent.GetWheelDelta();
		const float OutputChange = (ViewRange.Y - ViewRange.X) * MoveDelta;
	
		Debugger->SetDesiredViewRange(
			ViewRange.X + OutputChange,
			ViewRange.Y + OutputChange);
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

int32 SZzRewindTimelineRowWrapping::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                            const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                            const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	float SingleFrameSize = FUtils::SingleFrameSizeX(Debugger, AllottedGeometry);
	SingleFrameSize = FMath::Max(SingleFrameSize, 1.f);
	
	const float LocalPx = FUtils::FrameToLocal(Debugger, AllottedGeometry, Debugger->ScrubFrame);
	const FVector2D Size(SingleFrameSize, AllottedGeometry.GetLocalSize().Y);
			
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId - 1, 
		AllottedGeometry.ToPaintGeometry(Size, FSlateLayoutTransform({LocalPx, 0})),
		FCoreStyle::Get().GetBrush("WhiteBrush"), 
		ESlateDrawEffect::None,
		FLinearColor::White * 0.3f);
		
	return LayerId;
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}
