// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZzRewindSetting.generated.h"

UCLASS(config = ZzRewind, defaultconfig, meta=(DisplayName="ZzRewind"))
class ZZREWINDRUNTIME_API UZzRewindSetting : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UZzRewindSetting();
	
	virtual FName GetCategoryName() const override { return TEXT("ZzAction"); }
	
	static const UZzRewindSetting& Get()
	{
		return *GetDefault<UZzRewindSetting>();
	}
	
	static UZzRewindSetting& GetMutable()
	{
		return *GetMutableDefault<UZzRewindSetting>();
	}

	static const FName EngineTickModeName;
	
	UPROPERTY(config, EditAnywhere, Category = "Zz Rewind", meta=(GetOptions="GetRecordTickModeNames"))
	FName RecordTickMode;
	
	UFUNCTION()
	TArray<FName> GetRecordTickModeNames() const;
};
