// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "STimelineEditorToolbar.h"

#include "ZzRewindRuntime.h"
#include "ZzRewindEditorSetting.h"
#include "STimelineItemEditorFilter.h"

#include "SlateOptMacros.h"
#include "Styling/SlateIconFinder.h"
#include "ZzRewindDebuggerEditorModule.h"

namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STimelineEditorToolbar::Construct(const FArguments& InArgs)
{
	RewindDebugger = FZzRewindRuntime::Get();

	ChildSlot
	[
		SNew(SHorizontalBox)
		
		// play
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(INVTEXT("Start/Stop Play"))
			.IsEnabled(this, &STimelineEditorToolbar::IsEnabledTogglePlay)
			.OnClicked(this, &STimelineEditorToolbar::HandleTogglePlay)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.DesiredSizeOverride(FVector2D(16))
					.Image(this, &STimelineEditorToolbar::GetTogglePlayIcon)
				]
			]
		]

		// record
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(INVTEXT("Start/Stop Record"))
			.IsEnabled(this, &STimelineEditorToolbar::IsEnabledToggleRecord)
			.OnClicked(this, &STimelineEditorToolbar::HandleToggleRecord)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.DesiredSizeOverride(FVector2D(16))
					.Image(this, &STimelineEditorToolbar::GetToggleRecordIcon)
				]
			]
		]

		// actor filter
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(STimelineItemEditorFilter)
			]
		]

		// setting
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboButton)
			.ContentPadding(0.f)
			.ComboButtonStyle(&FAppStyle::Get().GetWidgetStyle<FComboButtonStyle>("SimpleComboButton"))
			.OnGetMenuContent(this, &STimelineEditorToolbar::GetSettingMenuContent)
			.HasDownArrow(false)
			.ButtonContent()
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FAppStyle::Get().GetBrush("Icons.Settings"))
			]
		]
	];


	RewindDebugger->OnScrubFrameChanged.AddRaw(this, &STimelineEditorToolbar::OnScrubFrameChanged);
}

void STimelineEditorToolbar::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bRewindPlaying)
	{
		CurrentRewindTime += InDeltaTime;

		int32 NewFrame = CurrentRewindTime * 60;
		if (NewFrame > RewindDebugger->MaxFrame)
		{
			RewindStop();
		}
		else
		{
			RewindDebugger->SetScrubFrame(NewFrame, EScrubTimeInfo::OnRewinding);
		}
	}
}

void STimelineEditorToolbar::OnScrubFrameChanged(int32 NewFrame, EScrubTimeInfo::Type InType)
{
	if (InType != EScrubTimeInfo::OnRewinding)
	{
		if (bRewindPlaying)
		{
			RewindStop();
		}
	}
}

TSharedRef<SWidget> STimelineEditorToolbar::GetSettingMenuContent()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	
	{
		MenuBuilder.BeginSection("Camera", INVTEXT("Camera"));

		MenuBuilder.AddMenuEntry(
			INVTEXT("Disable"),
			INVTEXT(""),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([]()
				{
					FZzRewindDebuggerEditorModule::Get().SetCameraMode(0);
				}),
				FCanExecuteAction(),
				FGetActionCheckState::CreateLambda([]()
				{
					return FZzRewindDebuggerEditorModule::Get().GetCameraMode() == 0 ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);

		MenuBuilder.AddMenuEntry(
			INVTEXT("Follow Target Actor"),
			INVTEXT(""),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([]()
				{
					FZzRewindDebuggerEditorModule::Get().SetCameraMode(1);
				}),
				FCanExecuteAction(),
				FGetActionCheckState::CreateLambda([]()
				{
					return FZzRewindDebuggerEditorModule::Get().GetCameraMode() == 1 ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);

		MenuBuilder.AddMenuEntry(
			INVTEXT("Replay Recorded Camera"),
			INVTEXT(""),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([]()
				{
					FZzRewindDebuggerEditorModule::Get().SetCameraMode(2);
				}),
				FCanExecuteAction(),
				FGetActionCheckState::CreateLambda([]()
				{
					return FZzRewindDebuggerEditorModule::Get().GetCameraMode() == 2 ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);

		MenuBuilder.EndSection();
	}

	{
		MenuBuilder.BeginSection("Editor", INVTEXT("Editor"));

		MenuBuilder.AddMenuEntry(
			INVTEXT("Auto Record"),
			INVTEXT("Automatically start recording when PIE is started"),
			FSlateIconFinder::FindIcon("RewindDebugger.AutoRecord"),
			FUIAction(
				FExecuteAction::CreateLambda([]()
				{
					UZzRewindEditorSetting::GetMutable().bAutoRecord = !UZzRewindEditorSetting::Get().bAutoRecord;
				}),
				FCanExecuteAction(),
				FGetActionCheckState::CreateLambda([]()
				{
					return UZzRewindEditorSetting::Get().bAutoRecord ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);

		MenuBuilder.AddMenuEntry(
			INVTEXT("Auto Eject"),
			INVTEXT("Automatically detach player control when PIE is paused"),
			FSlateIconFinder::FindIcon("RewindDebugger.AutoEject"),
			FUIAction(
				FExecuteAction::CreateLambda([]()
				{
					UZzRewindEditorSetting::GetMutable().bAutoEject = !UZzRewindEditorSetting::Get().bAutoEject;
				}),
				FCanExecuteAction(),
				FGetActionCheckState::CreateLambda([]()
				{
					return UZzRewindEditorSetting::Get().bAutoEject ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		MenuBuilder.EndSection();
	}
	return MenuBuilder.MakeWidget();
}


bool STimelineEditorToolbar::IsEnabledToggleRecord() const
{
	if (auto PIEWorldContext = GEditor->GetPIEWorldContext())
	{
		if (!PIEWorldContext->World())
			return false;

		/*if (PIEWorldContext->World()->IsPaused())
			return false;*/

		return true;
	}

	return false;
}

FReply STimelineEditorToolbar::HandleToggleRecord()
{
	if (FZzRewindRuntime::Get()->IsRecording())
	{
		FZzRewindRuntime::Get()->StopRecord();
	}
	else
	{
		if (auto PIEWorldContext = GEditor->GetPIEWorldContext())
		{
			FZzRewindRuntime::Get()->StartRecord(PIEWorldContext->World());
		}
	}

	return FReply::Handled();
}


const FSlateBrush* STimelineEditorToolbar::GetToggleRecordIcon() const
{
	return FZzRewindRuntime::Get()->IsRecording()
		? FSlateIconFinder::FindIcon("RewindDebugger.StopRecording.small").GetIcon()
		: FSlateIconFinder::FindIcon("RewindDebugger.StartRecording.small").GetIcon();
}


bool STimelineEditorToolbar::IsEnabledTogglePlay() const
{
	if (auto PIEWorldContext = GEditor->GetPIEWorldContext())
	{
		if (!PIEWorldContext->World())
			return false;

		if (!PIEWorldContext->World()->IsPaused())
			return false;
	}

	return FZzRewindRuntime::Get()->HasRecordData();
}

FReply STimelineEditorToolbar::HandleTogglePlay()
{
	if (bRewindPlaying)
		RewindStop();
	else
		RewindPlay();

	return FReply::Handled();
}

const FSlateBrush* STimelineEditorToolbar::GetTogglePlayIcon() const
{
	return bRewindPlaying
		? FSlateIconFinder::FindIcon("RewindDebugger.Pause.small").GetIcon()
		: FSlateIconFinder::FindIcon("RewindDebugger.Play.small").GetIcon();
}

void STimelineEditorToolbar::RewindPlay()
{
	bRewindPlaying = true;
	CurrentRewindTime = FZzRewindRuntime::Get()->ScrubFrame / 60.f;
}

void STimelineEditorToolbar::RewindStop()
{
	bRewindPlaying = false;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}