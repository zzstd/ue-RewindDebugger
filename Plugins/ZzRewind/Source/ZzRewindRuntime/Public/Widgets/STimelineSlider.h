// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"



namespace ZZ::Rewind
{

class FZzRewindRuntime;

class ZZREWINDRUNTIME_API STimelineSlider : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STimelineSlider)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
private:
	void DrawTicks(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	void DrawHandle(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;

	struct FDragOperation
	{
		bool bActive = false;
		bool bStart = false;
	
		void Start(const FPointerEvent& MouseEvent);
		void OnDragged(const TSharedPtr<FZzRewindRuntime>& InDebugger, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, bool IsMoving);
		void End();
	} DragOpt;
	
	const FSlateBrush* ScrubFillBrush			= nullptr;
	const FSlateBrush* ScrubHandleDownBrush	= nullptr;
	
	TSharedPtr<FZzRewindRuntime> Debugger;
};


}