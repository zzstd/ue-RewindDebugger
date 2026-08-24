// Copyright 2024-2026 zz studio. All Rights Reserved.

#include "ZzRewindRuntimeModule.h"

#include "ZzRewindRuntime.h"

#define LOCTEXT_NAMESPACE "FZzRewindRuntimeModule"

void FZzRewindRuntimeModule::StartupModule()
{
	RewindRuntime = MakeShared<ZZ::Rewind::FZzRewindRuntime>();
	RewindRuntime->Init();
}

void FZzRewindRuntimeModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FZzRewindRuntimeModule, ZzRewindRuntime)