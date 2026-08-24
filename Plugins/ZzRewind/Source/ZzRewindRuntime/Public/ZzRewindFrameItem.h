// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "ZzRewindItem.h"


namespace ZZ::Rewind
{

class ZZREWINDRUNTIME_API FRewindFrameItem : public FRewindItem
{
public:
	FRewindFrameItem();

	virtual TSharedRef<SWidget> GenerateHeadWidget() override;
	virtual TSharedRef<SWidget> GenerateTrackWidget() override;
	virtual TSharedRef<SWidget> GenerateInspector(int32 Frame) override;

	virtual FText GetTooltipText(int32 Frame) const;

	bool bPaintFrame = false;
	bool bPaintItem = false;
	
	struct FPaintFrameData
	{
		bool Valid = false;
		FLinearColor Color;
		float Widget = 1.f;
		
		bool bDrawCurve = false;
		float CurveValue = 0.f;
		FLinearColor CurveColor;
	};
	
	virtual void OnPaintFrame(int32 Frame, FPaintFrameData& OutPaint) const {}
	
	struct FPaintItemData
	{
		bool Valid = false;
		int32 StartFrame = -1;
		int32 EndFrame = -1;

		FLinearColor Color;
		
		FString Text;
		FLinearColor TextColor = FLinearColor::White;
	};
	
	virtual void OnPaintItem(int32 Frame, FPaintItemData& OutItem) const {}
protected:
	TWeakPtr<SWidget> TrackBox;
	TWeakPtr<SWidget> HeadBox;

	TWeakPtr<class SRewindFrameRow> FrameRowWidget;
	
	virtual bool IsHovered() const;
	FText PrivateGetTooltipText() const;
	virtual FOptionalSize GetRowHeightOptSize() const;
};

}