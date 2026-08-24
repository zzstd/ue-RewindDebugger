// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"



namespace ZZ::Rewind
{
	

class FRewindFrameItem;

class ZZREWINDRUNTIME_API SRewindFrameRow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRewindFrameRow)
		{
		}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FRewindFrameItem> InItem);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	TOptional<int32> HoveredFrame;
private:
	const FSlateBrush* DefaultBrush = nullptr;
	
	TWeakPtr<FRewindFrameItem> Item;
	TSharedPtr<class FZzRewindRuntime> Debugger;

	void SetHoveredFrame(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	void PaintFrame(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	
	struct FPaintItemInfo
	{
		float SizeX;
		float OffsetX;
		FString Text;
		FLinearColor Color;
	};
	void PaintItems(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, TArray<FPaintItemInfo>& OutInfos) const;

	static void PaintTexts(const TArray<FPaintItemInfo>& Infos, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId);
	void PaintBounds(const TArray<FPaintItemInfo>& Infos, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	void DrawHoveredEffect(const FPaintGeometry& PaintGeo, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
};



}