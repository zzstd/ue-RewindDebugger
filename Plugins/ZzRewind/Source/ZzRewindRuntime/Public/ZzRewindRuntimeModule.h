// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_DELEGATE_RetVal(TSharedRef<SWidget>, FZzRewindDebuggerToolbarBuilder)

namespace ZZ::Rewind
{
	class FZzRewindRuntime;
}

class FZzRewindRuntimeModule : public IModuleInterface
{
public:
	static FZzRewindRuntimeModule& Get()
	{
		return FModuleManager::Get().LoadModuleChecked<FZzRewindRuntimeModule>(TEXT("ZzRewindRuntime"));
	}
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FZzRewindDebuggerToolbarBuilder ToolbarBuilder;

	TSharedPtr<ZZ::Rewind::FZzRewindRuntime> RewindRuntime;
};
