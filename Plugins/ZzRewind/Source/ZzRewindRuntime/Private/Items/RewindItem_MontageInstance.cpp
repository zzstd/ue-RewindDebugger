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
		// Ensure that smaller values are visible in the timeline
		const float ShowWidget = Mont->Weight > UE_KINDA_SMALL_NUMBER 
			? FMath::Lerp(0.1f, 1.f, Mont->Weight) : 0.f;
		
		OutPaint.Valid = true;
		OutPaint.Widget = Mont->DesiredWeight > 0.f ? ShowWidget : -ShowWidget;
		OutPaint.Color = FLinearColor(0.153f, 0.153f, 0.361f);
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
		->AddValue("Weight", Mont->Weight)
		->AddValue("Desired Weight", Mont->DesiredWeight);
	
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
		TEXT("Pos: %.2f\nPlayRate: %.2f\nWeight: %.2f\nDesired Weight: %.2f")
			, Mont->Pos, Mont->PlayRate, Mont->Weight, Mont->DesiredWeight));
	}
	
	return FText();
}

}
