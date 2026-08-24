// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RewindItem_Actor.h"
#include "Utils/FrameData.h"
#include "Utils/ObjectInfo.h"


class APawn;
class APlayerController;

namespace ZZ::Rewind
{

struct FZzRewindItemPlayerControllerData
{
	// TODO: record switch controled pawn
	TObjectInfo<APawn> Pawn;

	FZzRewindItemPlayerControllerData(APlayerController* PC);

	bool operator==(const FZzRewindItemPlayerControllerData& Other) const;
};

class ZZREWINDRUNTIME_API FRewindItem_PlayerController : public FRewindItem_Actor
{
public:
	FRewindItem_PlayerController();

	virtual void OnSetup(UObject* Owner) override;
	virtual void OnRecord(int32 Frame) override;
private:
	TObjectInfo<APlayerController> PlayerController;
	TFrameData<FZzRewindItemPlayerControllerData> FrameData;
};

}