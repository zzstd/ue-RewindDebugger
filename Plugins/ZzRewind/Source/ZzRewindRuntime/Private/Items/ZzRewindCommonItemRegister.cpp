// Copyright 2024-2026 zz studio. All Rights Reserved.

#include "Items/ZzRewindCommonItemRegister.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#include "ZzRewindRuntime.h"
#include "Items/RewindItem_Camera.h"
#include "Items/RewindItem_Character.h"
#include "Items/RewindItem_PlayerController.h"
#include "Items/RewindItem_SkMesh.h"


void UZzRewindCommonItemRegister::OnActorSpawned(AActor* NewActor)
{
	using namespace ZZ::Rewind;

	if (NewActor->IsA(ACharacter::StaticClass()))
	{
		FZzRewindRuntime::Get()->NewPrimaryItem<FRewindItem_Character>(NewActor);
	}

	if (NewActor->IsA(APlayerCameraManager::StaticClass()))
	{
		FZzRewindRuntime::Get()->NewPrimaryItem<FRewindItem_Camera>(NewActor);
	}

	if (NewActor->IsA(APlayerController::StaticClass()))
	{
		FZzRewindRuntime::Get()->NewPrimaryItem<FRewindItem_PlayerController>(NewActor);
	}

	if (NewActor->FindComponentByClass<UZzRewindMarkerComponent>())
	{
		auto ActorItem = FZzRewindRuntime::Get()->NewPrimaryItem<FRewindItem_Actor>(NewActor);
		ActorItem->Tags.Add("ZzRewindMarkerComponent");
	}
}

void UZzRewindCommonItemRegister::OnPostComponentCreated(TSharedRef<ZZ::Rewind::FRewindItem> Item, UActorComponent* Component)
{
	using namespace ZZ::Rewind;

	if (Item->HasTag("ZzRewindMarkerComponent"))
	{
		if (FRewindItem_SkMesh::IsPrimarySkMeshComp(Cast<USkeletalMeshComponent>(Component)))
		{
			auto SkMeshItem = Item->AddChildItem<FRewindItem_SkMesh>(Component);
		}
	}
}