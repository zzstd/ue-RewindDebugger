// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZzRewindEditorSetting.generated.h"

/**
 * 
 */
UCLASS(config = ZzRewindEditor, defaultconfig, meta=(DisplayName="ZzRewindEditor"))
class ZZREWINDDEBUGGEREDITOR_API UZzRewindEditorSetting : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	virtual FName GetCategoryName() const override { return TEXT("ZzAction"); }
	
	static const UZzRewindEditorSetting& Get()
	{
		return *GetDefault<UZzRewindEditorSetting>();
	}
	
	static UZzRewindEditorSetting& GetMutable()
	{
		return *GetMutableDefault<UZzRewindEditorSetting>();
	}
	
	bool bAutoRecord = false;
	bool bAutoEject = true;
};
