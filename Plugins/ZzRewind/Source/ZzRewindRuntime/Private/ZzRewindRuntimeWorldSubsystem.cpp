// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindRuntimeWorldSubsystem.h"

#include "ZzRewindRuntime.h"
#include "ZzRewindSetting.h"

void UZzRewindRuntimeWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UZzRewindSetting::Get().RecordTickMode == UZzRewindSetting::EngineTickModeName)
	{
		RewindDebugger = ZZ::Rewind::FZzRewindRuntime::Get();
	}
}

TStatId UZzRewindRuntimeWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UZzRewindDebuggerSubsystem, STATGROUP_Tickables);
}

void UZzRewindRuntimeWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RewindDebugger)
	{
		RewindDebugger->ReceiveTick(TickCounter++);
	}
}
