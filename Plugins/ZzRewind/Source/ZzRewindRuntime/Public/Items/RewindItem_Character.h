// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RewindItem_Actor.h"
#include "Utils/FrameData.h"
#include "Utils/ObjectInfo.h"


namespace ZZ::Rewind
{

struct FRewindItemCharacterData
{
	FTransform ActorTF;

	FVector CapsuleData;

	FRewindItemCharacterData(ACharacter* Character);
	
	bool operator==(const FRewindItemCharacterData& Other) const;
};

class ZZREWINDRUNTIME_API FRewindItem_Character : public FRewindItem_Actor
{
public:
	virtual void OnSetup(UObject* Owner) override;
	virtual void OnRecord(int32 Frame) override;

	virtual void OnPostComponentCreated(UActorComponent* InComp) override;
private:
	TObjectInfo<ACharacter> Character;
	
	TFrameData<FRewindItemCharacterData> FrameData;
};


}