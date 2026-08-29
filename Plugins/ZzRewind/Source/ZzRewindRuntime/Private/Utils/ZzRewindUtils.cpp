// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Utils/ZzRewindUtils.h"

#include "ZzRewindRuntime.h"

namespace ZZ::Rewind
{

float FUtils::TimeToLocal(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo, float Time)
{
	const double Normalize = FMath::GetMappedRangeValueUnclamped(RewindDebugger->ViewRange, FVector2D(0.f, 1.f), Time);
	return Normalize * Geo.GetLocalSize().X;
}

int32 FUtils::LocalToFrame(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo, float LocalX)
{
	if (Geo.GetLocalSize().X <= 0.f)
	{
		return RewindDebugger->ScrubFrame;
	}

	const float NormalizeX = (Geo.AbsoluteToLocal(FVector2f(LocalX, 0.f)) / Geo.GetLocalSize()).X;
	return FMath::GetMappedRangeValueUnclamped(FVector2D(0.f, 1.f), RewindDebugger->ViewRange * 60, NormalizeX);
}

float FUtils::SingleFrameSizeX(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo)
{
	const double Duration = RewindDebugger->ViewRange.Y - RewindDebugger->ViewRange.X;
	return Duration > 0.0 ? Geo.GetLocalSize().X / Duration / 60.0 : 0.f;
}

void FUtils::ExpandViewRangeFrame(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, int32& OutBegin,
	int32& OutEnd)
{
	OutBegin = FMath::Max(0, FMath::FloorToInt32(RewindDebugger->ViewRange.X * 60.0));
	OutEnd = FMath::Min(RewindDebugger->MaxFrame, FMath::CeilToInt32(RewindDebugger->ViewRange.Y * 60.0));
}

}
