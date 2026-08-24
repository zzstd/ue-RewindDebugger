// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Widgets/SRewindInspector.h"
#include "ZzRewindRuntime.h"
#include "ZzRewindItem.h"

#include "SlateOptMacros.h"

namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SRewindInspector::Construct(const FArguments& InArgs)
{
	Debugger = FZzRewindRuntime::Get();
	
	ChildSlot
	[
		SAssignNew(Content, SBox)
	];
	
	Debugger->OnScrubFrameChanged.AddSP(this, &SRewindInspector::OnScrubFrameChanged);
	Debugger->OnTimelineSectionChanged.AddSP(this, &SRewindInspector::UpdateInspector);
}

void SRewindInspector::UpdateInspector()
{ 
	if (Content)
	{
		if (Debugger->SelectedItem)
		{
			Content->SetContent(Debugger->SelectedItem->GenerateInspector(Debugger->ScrubFrame));
		}
		else
		{
			Content->SetContent(SNullWidget::NullWidget);
		}
	}
}

void SRewindInspector::OnScrubFrameChanged(int32, EScrubTimeInfo::Type)
{
	UpdateInspector();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}