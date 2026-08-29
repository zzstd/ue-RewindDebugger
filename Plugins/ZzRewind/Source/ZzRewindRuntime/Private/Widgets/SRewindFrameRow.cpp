// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Widgets/SRewindFrameRow.h"
#include "ZzRewindRuntime.h"
#include "ZzRewindFrameItem.h"
#include "Utils/ZzRewindUtils.h"

#include "SlateOptMacros.h"

namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SRewindFrameRow::Construct(const FArguments& InArgs, TSharedRef<FRewindFrameItem> InItem)
{
	Item = InItem;
	Debugger = FZzRewindRuntime::Get();
	DefaultBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	ChildSlot
	[
		SNew(SBox)
	];
}

int32 SRewindFrameRow::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	if (auto StrongItem = Item.Pin())
	{
		TArray<FPaintItemInfo> PaintItemInfos;
		
		constexpr float Padding = 6;
		auto ContentGeo = AllottedGeometry.MakeChild(
			{AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y - Padding * 2}, 
			FSlateLayoutTransform({0, Padding}));
		
		if (StrongItem->bPaintItem)
		{
			PaintItems(ContentGeo, OutDrawElements, ++LayerId, PaintItemInfos);
		}
		
		if (StrongItem->bPaintFrame)
		{
			PaintFrame(ContentGeo, OutDrawElements, ++LayerId);
		}

		PaintBounds(PaintItemInfos, ContentGeo, OutDrawElements, ++LayerId);
		PaintTexts(PaintItemInfos, ContentGeo, OutDrawElements, ++LayerId);
	}
	
	return ++LayerId;
}

void SRewindFrameRow::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	SetHoveredFrame(MyGeometry, MouseEvent);
}

void SRewindFrameRow::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);
	HoveredFrame.Reset();
}

FReply SRewindFrameRow::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SetHoveredFrame(MyGeometry, MouseEvent);
	return SCompoundWidget::OnMouseMove(MyGeometry, MouseEvent);
}

void SRewindFrameRow::SetHoveredFrame(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	HoveredFrame = FUtils::LocalToFrame(FZzRewindRuntime::Get(), MyGeometry, MouseEvent.GetScreenSpacePosition().X);
}

void SRewindFrameRow::PaintFrame(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements,
									int32 LayerId) const
{
	auto StrongItem = Item.Pin();
	if (!StrongItem)
	{
		return;
	}
	
	const float SingleFrameSize = FUtils::SingleFrameSizeX(Debugger, AllottedGeometry);

	int32 ViewBegin, ViewEnd;
	FUtils::ExpandViewRangeFrame(Debugger, ViewBegin, ViewEnd);
	for (int32 CurFrame = ViewBegin; CurFrame <= ViewEnd; ++CurFrame)
	{
		FRewindFrameItem::FPaintFrameData PaintData;
		StrongItem->OnPaintFrame(CurFrame, PaintData);
		
		if (!PaintData.Valid && !PaintData.bDrawCurve)
		{
			continue;
		}
		
		const float GeoSizeY = AllottedGeometry.GetLocalSize().Y;
		const float OffsetX = FUtils::FrameToLocal(Debugger, AllottedGeometry, CurFrame);

		if (PaintData.Valid)
		{
			const float SizeY = GeoSizeY  * FMath::Abs(PaintData.Widget);
			float OffsetY = 0.f;
			if (PaintData.Widget >= 0.f)
			{
				OffsetY = GeoSizeY - SizeY;
			}
		
			FSlateDrawElement::MakeBox(
				OutDrawElements, 
				LayerId, 
				AllottedGeometry.ToPaintGeometry(
					FVector2f(SingleFrameSize + 0.1f, SizeY),
					FSlateLayoutTransform({OffsetX - 0.05f, OffsetY})),
				DefaultBrush, 
				ESlateDrawEffect::None,
				PaintData.Color);
		}
		
		if (PaintData.bDrawCurve)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements, 
				LayerId, 
				AllottedGeometry.ToPaintGeometry(
					FVector2f(SingleFrameSize + 0.4f, GeoSizeY),
					FSlateLayoutTransform({OffsetX - 0.2f, 0})),
				DefaultBrush, 
				ESlateDrawEffect::None,
				PaintData.Color);
		}

		if (HoveredFrame == CurFrame)
		{
			DrawHoveredEffect(
				AllottedGeometry.ToPaintGeometry(
				FVector2f(SingleFrameSize, GeoSizeY),
					FSlateLayoutTransform({OffsetX, 0})),
				OutDrawElements, 
				LayerId + 1);
		}
	}
}

void SRewindFrameRow::PaintItems(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, TArray<FPaintItemInfo>& OutInfos) const
{
	auto StrongItem = Item.Pin();
	if (!StrongItem)
	{
		return;
	}
	
	int32 ViewBegin, ViewEnd;
	FUtils::ExpandViewRangeFrame(Debugger, ViewBegin, ViewEnd);
	for (int64 CurFrame64 = ViewBegin; CurFrame64 <= ViewEnd; ++CurFrame64)
	{
		const int32 CurFrame = static_cast<int32>(CurFrame64);
		FRewindFrameItem::FPaintItemData PaintData;
		StrongItem->OnPaintItem(CurFrame, PaintData);
			
		if (!PaintData.Valid)
		{
			continue;
		}

		if (!ensure(PaintData.StartFrame <= CurFrame && CurFrame <= PaintData.EndFrame))
		{
			continue;
		}

		const int32 ItemStartFrame = FMath::Max(PaintData.StartFrame, ViewBegin);
		const int32 ItemEndFrame = FMath::Min(PaintData.EndFrame, ViewEnd);
		const int64 ItemFrameCount = static_cast<int64>(ItemEndFrame) - ItemStartFrame + 1;

		const float OffsetX = FUtils::FrameToLocal(Debugger, AllottedGeometry, ItemStartFrame);
		const float SizeX = FUtils::SingleFrameSizeX(Debugger, AllottedGeometry) * ItemFrameCount;

		auto ItemGeo = AllottedGeometry.ToPaintGeometry({SizeX, AllottedGeometry.GetLocalSize().Y}, FSlateLayoutTransform({OffsetX, 0}));
		
		FSlateDrawElement::MakeBox(
			OutDrawElements, 
			LayerId, 
			ItemGeo,
			DefaultBrush, 
			ESlateDrawEffect::None,
			PaintData.Color);

		if (!PaintData.Text.IsEmpty())
		{
			auto& TextRef = OutInfos.AddDefaulted_GetRef();

			TextRef.SizeX = SizeX;
			TextRef.OffsetX = OffsetX;
			TextRef.Text = PaintData.Text;
			TextRef.Color = PaintData.TextColor;
		}
		
		if (HoveredFrame.Get(-1) >= ItemStartFrame && HoveredFrame.Get(-1) <= ItemEndFrame)
		{
			DrawHoveredEffect(ItemGeo,OutDrawElements, LayerId + 1);
		}

		CurFrame64 = ItemEndFrame;
	}
}

void SRewindFrameRow::PaintTexts(const TArray<FPaintItemInfo>& Infos, const FGeometry& AllottedGeometry,
								   FSlateWindowElementList& OutDrawElements, int32 LayerId)
{
	FSlateFontInfo SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 8);
	
	for (int32 i = 0; i < Infos.Num(); ++i)
	{
		if (Infos[i].Text.IsEmpty())
			continue;
		
		float CullSizeX = Infos[i].SizeX;
		if (i < Infos.Num() - 1)
		{
			const float CurEndPos = Infos[i].OffsetX + Infos[i].SizeX;
			const float NextStartPos = Infos[i + 1].OffsetX;
			const float NewCurEndPos = FMath::Min(CurEndPos, NextStartPos);
			CullSizeX = NewCurEndPos - Infos[i].OffsetX;
		}

		auto& Info = Infos[i];
		
		// todo: vertically centered text
		auto TextGeo = AllottedGeometry.MakeChild(
			FVector2D(CullSizeX - 8, AllottedGeometry.GetLocalSize().Y), 
			FSlateLayoutTransform({Info.OffsetX + 4, 0}));
		
		FSlateRect TextClippingRect = TextGeo.GetLayoutBoundingRect();
		
		OutDrawElements.PushClip(FSlateClippingZone(TextClippingRect));
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			TextGeo.ToPaintGeometry(),
			Info.Text,
			SmallLayoutFont,
			ESlateDrawEffect::None,
			Info.Color
		);
		OutDrawElements.PopClip();
	}
}

void SRewindFrameRow::PaintBounds(const TArray<FPaintItemInfo>& Infos, const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	for (auto & Info : Infos)
	{
		// start effect
		FSlateDrawElement::MakeBox(
			OutDrawElements, 
			LayerId, 
			AllottedGeometry.ToPaintGeometry(
				FVector2f(2.f, AllottedGeometry.GetLocalSize().Y),
				FSlateLayoutTransform({Info.OffsetX, 0})),
			DefaultBrush, 
			ESlateDrawEffect::None,
			FLinearColor::Green);

		// end effect
		FSlateDrawElement::MakeBox(
			OutDrawElements, 
			LayerId, 
			AllottedGeometry.ToPaintGeometry(
				FVector2f(2.f, AllottedGeometry.GetLocalSize().Y),
				FSlateLayoutTransform({Info.OffsetX + Info.SizeX - 2, 0})),
			DefaultBrush, 
			ESlateDrawEffect::None,
			FLinearColor::Red);
	}
}

void SRewindFrameRow::DrawHoveredEffect(const FPaintGeometry& PaintGeo, FSlateWindowElementList& OutDrawElements,
										  int32 LayerId) const
{
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeo,
		DefaultBrush, 
		ESlateDrawEffect::None,
		FLinearColor::White.CopyWithNewOpacity(0.5));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}
