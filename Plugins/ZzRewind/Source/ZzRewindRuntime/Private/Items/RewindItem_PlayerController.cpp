// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Items/RewindItem_PlayerController.h"

#include "GameFramework/PlayerController.h"

namespace ZZ::Rewind
{

FZzRewindItemPlayerControllerData::FZzRewindItemPlayerControllerData(APlayerController* PC)
{
	Pawn = nullptr;
}

bool FZzRewindItemPlayerControllerData::operator==(const FZzRewindItemPlayerControllerData& Other) const
{
	return Pawn == Other.Pawn;
}

FRewindItem_PlayerController::FRewindItem_PlayerController()
{
	Tags.Add("RewindPlayerController");
}

void FRewindItem_PlayerController::OnSetup(UObject* Owner)
{
	FRewindItem_Actor::OnSetup(Owner);

	APlayerController* PC = Cast<APlayerController>(Owner);
	check(PC);
	PlayerController = PC;
}

void FRewindItem_PlayerController::OnRecord(int32 Frame)
{
	FRewindItem_Actor::OnRecord(Frame);

	if (PlayerController.WeakObject.IsValid())
	{
		FrameData.Append(Frame, PlayerController.WeakObject.Get());
	}
}

}