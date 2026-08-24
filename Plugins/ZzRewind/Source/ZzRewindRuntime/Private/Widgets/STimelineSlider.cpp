// Copyright 2024-2026 zz studio. All Rights Reserved.

#include "Widgets/STimelineSlider.h"
#include "ZzRewindRuntime.h"
#include "Utils/ZzRewindUtils.h"

#include "SlateOptMacros.h"

namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STimelineSlider::Construct(const FArguments& InArgs)
{
	Debugger = FZzRewindRuntime::Get();
	ScrubFillBrush			= FAppStyle::GetBrush(TEXT("Sequencer.Timeline.ScrubFill"));
	ScrubHandleDownBrush	= FAppStyle::GetBrush(TEXT("Sequencer.Timeline.VanillaScrubHandleDown"));
}

FReply STimelineSlider::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		DragOpt.Start(MouseEvent);	// don't DragDetect, activate now
		DragOpt.OnDragged(Debugger, MyGeometry, MouseEvent, false);
		return FReply::Handled().CaptureMouse(SharedThis(this)).PreventThrottling();
	}

	return FReply::Unhandled();
}

FReply STimelineSlider::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (DragOpt.bActive || DragOpt.bStart)
	{
		DragOpt.OnDragged(Debugger, MyGeometry, MouseEvent, true);
		DragOpt.End();
		return FReply::Handled().ReleaseMouseCapture();
	}
	
	return FReply::Unhandled();
}

FReply STimelineSlider::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && DragOpt.bActive)
	{
		DragOpt.OnDragged(Debugger, MyGeometry, MouseEvent, true);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

int32 STimelineSlider::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                               const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                               const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	DrawTicks(AllottedGeometry, OutDrawElements, ++LayerId);
	DrawHandle(AllottedGeometry, OutDrawElements, ++LayerId, InWidgetStyle);
	return LayerId;
}

void STimelineSlider::FDragOperation::Start(const FPointerEvent& MouseEvent)
{
	bActive = true;
}

void STimelineSlider::FDragOperation::OnDragged(const TSharedPtr<FZzRewindRuntime>& InDebugger, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, bool IsMoving)
{
	const int32 ScrubFrame = FUtils::LocalToFrame(InDebugger, MyGeometry, MouseEvent.GetScreenSpacePosition().X);
	InDebugger->SetScrubFrame(ScrubFrame, IsMoving ? EScrubTimeInfo::OnMouseDrag : EScrubTimeInfo::OnNavigation);
}

void STimelineSlider::FDragOperation::End()
{
	bActive = false;
	bStart = false;
}

void STimelineSlider::DrawTicks(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements,
	int32 LayerId) const
{
	constexpr int32 MajorStep = 60;
	FSlateFontInfo SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 8);
	
	int32 ViewBegin, ViewEnd;
	FUtils::ExpandViewRangeFrame(Debugger, ViewBegin, ViewEnd);

	const int32 BeginOffset = (MajorStep - ViewBegin % MajorStep) % MajorStep;
	
	TArray<FVector2D> LinePoints;
	LinePoints.SetNumUninitialized(2);
	for (int32 CurFrame = ViewBegin + BeginOffset; CurFrame <= ViewEnd; CurFrame += MajorStep)
	{
		const float LinePx = FUtils::FrameToLocal(Debugger, AllottedGeometry, CurFrame);
			
		LinePoints[0] = FVector2D(LinePx, 0);
		LinePoints[1] = FVector2D( LinePx, AllottedGeometry.GetLocalSize().Y);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			FLinearColor::White,
			false
			);
			
		const FVector2D TextOffset(LinePx + 5.f, 0.f);
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId, 
			AllottedGeometry.ToPaintGeometry(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(TextOffset) ), 
			FString::Printf(TEXT("%.1f"), CurFrame / 60.0), 
			SmallLayoutFont,
			ESlateDrawEffect::None,
			FLinearColor::White * 0.65f
			);
	}
}

void STimelineSlider::DrawHandle(const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	constexpr float HandleSize = 13.f;
	
	const int32 ArrowLayer = LayerId + 2;
	FLinearColor ScrubColor = InWidgetStyle.GetColorAndOpacityTint();
	{
		ScrubColor.A = ScrubColor.A * 0.75f;
		ScrubColor.B *= 0.1f;
		ScrubColor.G *= 0.2f;
	}
	
	float HandleStart = FUtils::FrameToLocal(Debugger, AllottedGeometry, Debugger->ScrubFrame);
	const float FullHandleSize = FUtils::SingleFrameSizeX(Debugger, AllottedGeometry);
	if (FullHandleSize > 4)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			ArrowLayer,
			AllottedGeometry.ToPaintGeometry(FVector2f(FullHandleSize, AllottedGeometry.Size.Y),
				FSlateLayoutTransform(FVector2f( HandleStart, 0.f ))),
			ScrubFillBrush,
			ESlateDrawEffect::None,
			ScrubColor
		);
		return;
	}
	
	HandleStart -= HandleSize * 0.5f;
	const float HandleEnd = HandleStart + HandleSize;

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		ArrowLayer,
		AllottedGeometry.ToPaintGeometry(FVector2f(HandleEnd - HandleStart, AllottedGeometry.Size.Y),
			FSlateLayoutTransform(FVector2f( HandleStart, 0.f ))),
		ScrubHandleDownBrush,
		ESlateDrawEffect::None,
		ScrubColor
	);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}
