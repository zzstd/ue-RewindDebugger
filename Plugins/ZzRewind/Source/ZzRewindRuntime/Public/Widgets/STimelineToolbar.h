// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

namespace ZZ::Rewind
{

class ZZREWINDRUNTIME_API STimelineToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STimelineToolbar)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};

}