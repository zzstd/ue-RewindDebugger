// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace ZZ::Rewind
{
namespace EScrubTimeInfo
{
	enum Type : int
	{
		Direct,
		OnRecording,
		OnNavigation,	// Single click mouse slider is also navigation
		OnMouseDrag,	// Drag and drop the slider with the mouse
		OnRewinding,
	};
}
}