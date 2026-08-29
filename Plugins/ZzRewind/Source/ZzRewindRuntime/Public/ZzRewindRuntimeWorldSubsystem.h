// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZzRewindRuntimeWorldSubsystem.generated.h"

namespace ZZ::Rewind
{
	class FZzRewindRuntime;
}


UCLASS()
class ZZREWINDRUNTIME_API UZzRewindRuntimeWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;
	
	TSharedPtr<ZZ::Rewind::FZzRewindRuntime> RewindDebugger;

	int32 TickCounter = 0;
};
