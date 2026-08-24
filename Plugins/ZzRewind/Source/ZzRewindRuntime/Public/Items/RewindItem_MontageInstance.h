// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindFrameItem.h"
#include "Utils/FrameData.h"


namespace ZZ::Rewind
{

class ZZREWINDRUNTIME_API FRewindItem_MontageInstance: public FRewindFrameItem
{
public:
	FRewindItem_MontageInstance();
	
	virtual FText GetDisplayName() const override;
	virtual void OnPaintFrame(int32 Frame, FPaintFrameData& OutPaint) const override;
	virtual TSharedRef<SWidget> GenerateInspector(int32 Frame) override;
	virtual FText GetTooltipText(int32 Frame) const override;
	
	TWeakObjectPtr<UAnimMontage> Montage;
	FString MontageName;
	
	struct FMontageInst
	{
		float Pos;
		float PlayRate;
		float Widget;
		float DesiredWidget;
		bool operator==(const FMontageInst& Other) const
		{
			return Pos == Other.Pos && PlayRate == Other.PlayRate && DesiredWidget == Other.DesiredWidget;
		}
	};
	
	TFrameData<FMontageInst> MontageInstData;
private:
	TSharedPtr<class SVariantValueView> InspectorWidget;
};

}