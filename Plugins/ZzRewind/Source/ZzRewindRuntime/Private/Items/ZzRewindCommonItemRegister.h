// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindItemRegister.h"
#include "ZzRewindCommonItemRegister.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent))
class ZZREWINDRUNTIME_API UZzRewindMarkerComponent : public UActorComponent
{
	GENERATED_BODY()
};

UCLASS()
class ZZREWINDRUNTIME_API UZzRewindCommonItemRegister : public UZzRewindItemRegister
{
	GENERATED_BODY()
public:
	virtual void OnActorSpawned(AActor* NewActor) override;
	virtual void OnPostComponentCreated(TSharedRef<ZZ::Rewind::FRewindItem> Item, UActorComponent* Component) override;
	
};
