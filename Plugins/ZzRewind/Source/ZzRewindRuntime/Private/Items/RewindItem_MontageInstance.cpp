// Copyright 2024-2026 zz studio. All Rights Reserved.

#include "Items/RewindItem_MontageInstance.h"

#include "Widgets/SVariantValueView.h"
#include "Styling/SlateIconFinder.h"

namespace ZZ::Rewind
{

FRewindItem_MontageInstance::FRewindItem_MontageInstance()
{
	bPaintFrame = true;

	Icon = FSlateIconFinder::FindIconForClass(UAnimMontage::StaticClass());
}

FText FRewindItem_MontageInstance::GetDisplayName() const
{
	return FText::FromString(MontageName);
}

void FRewindItem_MontageInstance::OnPaintFrame(int32 Frame, FPaintFrameData& OutPaint) const
{
	if (auto Mont = MontageInstData.Find(Frame))
	{
		OutPaint.Valid = true;
		OutPaint.Widget = Mont->DesiredWidget > 0.f ? Mont->Widget : -Mont->Widget;
		OutPaint.Color = FLinearColor(0.153, 0.153, 0.361);
	}
}

TSharedRef<SWidget> FRewindItem_MontageInstance::GenerateInspector(int32 Frame)
{
	if (!InspectorWidget)
	{
		SAssignNew(InspectorWidget, SVariantValueView);
	}
	
	auto InspectorNode = InspectorWidget->GetRoot()->Begin("Montage", Montage.Get());
	if (auto Mont = MontageInstData.Find(Frame))
	{
		const float MontageLength = Montage.IsValid() ? Montage.Get()->GetPlayLength() : 1.f;
		
		InspectorNode->Show()
		->AddValue("Position", FString::Printf(TEXT("%.3f / %.3f (%.3f%%)"), Mont->Pos, MontageLength, (Mont->Pos / MontageLength) * 100.f))
		->AddValue("Play Rate", Mont->PlayRate)
		->AddValue("Widget", Mont->Widget)
		->AddValue("Desired Widget", Mont->DesiredWidget);
	
		InspectorNode->Expand();
		InspectorNode->ShowChildren();
	}
	else
	{
		InspectorNode->HiddenChildren();
	}
	
	return InspectorWidget.ToSharedRef();
}

FText FRewindItem_MontageInstance::GetTooltipText(int32 Frame) const
{
	if (auto Mont = MontageInstData.Find(Frame))
	{
		return FText::FromString(FString::Printf(
		TEXT("Pos: %.2f\nPlayRate: %.2f\nWidget: %.2f\nWidget: %.2f")
			, Mont->Pos, Mont->PlayRate, Mont->Widget, Mont->DesiredWidget));
	}
	
	return FText();
}

}