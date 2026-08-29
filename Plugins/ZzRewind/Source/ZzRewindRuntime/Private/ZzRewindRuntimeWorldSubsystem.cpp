// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindRuntimeWorldSubsystem.h"

#include "ZzRewindRuntime.h"
#include "ZzRewindSetting.h"

void UZzRewindRuntimeWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RewindDebugger = ZZ::Rewind::FZzRewindRuntime::Get();
}

void UZzRewindRuntimeWorldSubsystem::Deinitialize()
{
	if (RewindDebugger)
	{
		RewindDebugger->HandleWorldCleanup(GetWorld());
		RewindDebugger.Reset();
	}

	Super::Deinitialize();
}

TStatId UZzRewindRuntimeWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UZzRewindDebuggerSubsystem, STATGROUP_Tickables);
}

void UZzRewindRuntimeWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RewindDebugger && UZzRewindSetting::Get().RecordTickMode == UZzRewindSetting::EngineTickModeName)
	{
		RewindDebugger->ReceiveTick(GetWorld(), TickCounter++);
	}
}
