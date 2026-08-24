// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindSetting.h"

#include "ZzRewindItemRegister.h"


const FName UZzRewindSetting::EngineTickModeName("EngineTick");

UZzRewindSetting::UZzRewindSetting()
{
	RecordTickMode = EngineTickModeName;
}

TArray<FName> UZzRewindSetting::GetRecordTickModeNames() const
{
	TArray<FName> Result;
	
	Result.Add(TEXT("Disable"));
	//Use the engine's tick to record rewind data, which is the default mode.
	Result.Add(EngineTickModeName);
	
	UZzRewindItemRegister::ForEachRegisterCallFunction(&UZzRewindItemRegister::RegisterRecordTickMode, Result);
	
	return Result;
}
