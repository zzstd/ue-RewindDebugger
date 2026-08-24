// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZzRewindItemRegister.h"
#include "ZzRewindFrameItem.h"
#include "ZzRewindBlueprintRegister.generated.h"

class UZzRewindFrameItemBlueprintBase;

UCLASS()
class ZZREWINDBLUEPRINT_API UZzRewindBlueprintRegister : public UZzRewindItemRegister
{
	GENERATED_BODY()
public:
	virtual void OnActorSpawned(AActor* NewActor) override;
	virtual void OnPostComponentCreated(TSharedRef<ZZ::Rewind::FRewindItem> Item, UActorComponent* Component) override;
	
	UPROPERTY(Transient)
	TArray<UClass*> BlueprintClasses;
};

namespace ZZ::Rewind
{
	class FRewindItem_BlueprintBase : public FRewindFrameItem
	{
	public:
		virtual void OnSetup(UObject* Owner) override;
		virtual void OnRecord(int32 Frame) override;
		virtual FText GetDisplayName() const override;
		virtual TSharedRef<SWidget> GenerateInspector(int32 Frame) override;
		
		virtual FText GetTooltipText(int32 Frame) const override;
		virtual void OnPaintFrame(int32 Frame, FPaintFrameData& OutPaint) const override;
		virtual void OnPaintItem(int32 Frame, FPaintItemData& OutItem) const override;
	private:
		TSharedPtr<class SVariantValueView> InspectorWidget;
		TStrongObjectPtr<UZzRewindFrameItemBlueprintBase> ItemBP;
	};
}
