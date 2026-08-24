// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindItemRegister.h"

#include "ZzRewindRuntime.h"
#include "GameFramework/Character.h"
#include "Items/RewindItem_Camera.h"
#include "Items/RewindItem_Character.h"

bool UZzRewindItemRegister::IsValidRewindItemRegisterClass(const UClass* Class)
{
	if (!Class->IsChildOf(UZzRewindItemRegister::StaticClass()))
	{
		return false;
	}
	if (Class->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return false;
	}
	auto Name = Class->GetName();
	if (Name.Contains("SKEL_") || Name.Contains("REINST_"))
	{
		return false;
	}
	return !Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists);
}
