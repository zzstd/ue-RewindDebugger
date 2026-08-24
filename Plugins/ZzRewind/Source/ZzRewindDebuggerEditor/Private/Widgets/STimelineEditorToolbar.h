// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindType.h"
#include "Widgets/SCompoundWidget.h"

namespace ZZ::Rewind
{
	
	
class STimelineEditorToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STimelineEditorToolbar)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
private:
	TSharedPtr<class FZzRewindRuntime> RewindDebugger;

	void OnScrubFrameChanged(int32 NewFrame, EScrubTimeInfo::Type InType);

	TSharedRef<SWidget> GetSettingMenuContent();

	bool IsEnabledToggleRecord() const;
	FReply HandleToggleRecord();
	const FSlateBrush* GetToggleRecordIcon() const;

	bool IsEnabledTogglePlay() const;
	FReply HandleTogglePlay();
	const FSlateBrush* GetTogglePlayIcon() const;

	bool bRewindPlaying = false;
	float CurrentRewindTime = 0.f;
	void RewindPlay();
	void RewindStop();
};

}