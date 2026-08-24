// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


namespace ZZ::Rewind
{

class FZzRewindRuntime;

class ZZREWINDRUNTIME_API FUtils
{
public:
	static float TimeToLocal(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo, float Time);
	static float FrameToLocal(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo, int32 Frame)
	{
		return TimeToLocal(RewindDebugger, Geo, Frame / 60.0);
	}
	
	static int32 LocalToFrame(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo, float LocalX);

	static float SingleFrameSizeX(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, const FGeometry& Geo);
	
	static void ExpandViewRangeFrame(const TSharedPtr<FZzRewindRuntime>& RewindDebugger, int32& OutBegin, int32& OutEnd);
	
	static double Align(double Time) { return FMath::RoundToInt(Time * 60.0) / 60.0; }
};

}