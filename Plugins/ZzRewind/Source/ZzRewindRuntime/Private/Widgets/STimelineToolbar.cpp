// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Widgets/STimelineToolbar.h"

#include "SlateOptMacros.h"
#include "ZzRewindRuntimeModule.h"

namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STimelineToolbar::Construct(const FArguments& InArgs)
{
	TSharedRef<SWidget> ToolbarWidget = SNullWidget::NullWidget;
	if (FZzRewindRuntimeModule::Get().ToolbarBuilder.IsBound())
	{
		ToolbarWidget = FZzRewindRuntimeModule::Get().ToolbarBuilder.Execute();
	}
	
	ChildSlot
	[
		ToolbarWidget
	];
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}