// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

namespace ZZ::Rewind
{
	
class FRewindItem;

class STimelineItemEditorFilter : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STimelineItemEditorFilter)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedRef<SWidget> MakeSelectActorMenu();
	FReply OnSelectActorClicked();

	// only PIE
	TSharedPtr<FRewindItem> FindItemByActor(const AActor* Actor) const;

	// PIE or editor
	TSharedPtr<FRewindItem> FindItemByPlayer(int32 InPlayerIndex) const;

	void SetShowItem(TSharedPtr<FRewindItem> InItem);
private:
	TSharedPtr<FRewindItem> GetFilteredItem() const;
};
}