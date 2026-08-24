// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindSetting.h"
#include "UObject/Object.h"
#include "ZzRewindItemRegister.generated.h"


namespace ZZ::Rewind
{
	class FRewindItem;
}

UCLASS(Abstract)
class ZZREWINDRUNTIME_API UZzRewindItemRegister : public UObject
{
	GENERATED_BODY()
public:
	virtual void OnActorSpawned(AActor* NewActor) { }

	virtual void OnPostComponentCreated(TSharedRef<ZZ::Rewind::FRewindItem> Item, UActorComponent* Component) { }
	
	/*
	 * Register to record Tick mode, @see: UZzRewindSetting::RecordTickMode
	 */
	virtual void RegisterRecordTickMode(TArray<FName>& Modes) const { }

	static bool IsValidRewindItemRegisterClass(const UClass* Class);
	
	template<typename FuncType, typename... ArgTypes>
	static void ForEachRegisterCallFunction(FuncType Func, ArgTypes&&... Args)
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (It && IsValidRewindItemRegisterClass(*It))
			{
				if (auto CDO = It->GetDefaultObject<UZzRewindItemRegister>())
				{
					(CDO->*Func)(Forward<ArgTypes>(Args)...);
				}
			}
		}
	}
};