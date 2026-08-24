// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindType.h"
#include "Widgets/SCompoundWidget.h"


namespace ZZ::Rewind
{


class ZZREWINDRUNTIME_API SRewindInspector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRewindInspector)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
private:
	TSharedPtr<SBox> Content;
	
	void UpdateInspector();
	void OnScrubFrameChanged(int32, EScrubTimeInfo::Type);
	
	TSharedPtr<class FZzRewindRuntime> Debugger;
};


}