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
	const float NormalizeX = (Geo.AbsoluteToLocal(FVector2f(LocalX, 0.f)) / Geo.GetLocalSize()).X;
	return FMath::GetMappedRangeValueUnclamped(FVector2D(0.f, 1.f), RewindDebugger->ViewRange * 60, NormalizeX);
}

float FUtils::SingleFrameSizeX(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo)
{
	return Geo.GetLocalSize().X / (RewindDebugger->ViewRange.Y - RewindDebugger->ViewRange.X) / 60.0;
}

void FUtils::ExpandViewRangeFrame(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, int32& OutBegin,
	int32& OutEnd)
{
	OutBegin = FMath::FloorToInt32(RewindDebugger->ViewRange.X * 60.0);
	OutEnd	= FMath::CeilToInt32(RewindDebugger->ViewRange.Y * 60.0);
}

}