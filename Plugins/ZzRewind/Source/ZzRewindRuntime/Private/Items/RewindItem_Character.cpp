// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Items/RewindItem_Character.h"

#include "Components/CapsuleComponent.h"
#include "Items/RewindItem_SkMesh.h"
#include "GameFramework/Character.h"

namespace ZZ::Rewind
{
UE_DISABLE_OPTIMIZATION

FRewindItemCharacterData::FRewindItemCharacterData(ACharacter* Character)
{
	ActorTF = Character->GetActorTransform();
	auto CapComp = Character->GetCapsuleComponent();
	check(CapComp);
	CapsuleData.X = CapComp->GetScaledCapsuleRadius();
	CapsuleData.Y = CapComp->GetScaledCapsuleHalfHeight();
}

bool FRewindItemCharacterData::operator==(const FRewindItemCharacterData& Other) const
{
	return ActorTF.Equals(Other.ActorTF) && CapsuleData.Equals(Other.CapsuleData);
}

void FRewindItem_Character::OnSetup(UObject* Owner)
{
	FRewindItem_Actor::OnSetup(Owner);
	
	ACharacter* Char = Cast<ACharacter>(Owner); 
	check(Char);
	Character = Char;
	
	AddChildItem<FRewindItem_SkMesh>(Char->GetMesh());

	if (Char->IsPlayerControlled())
	{
		Tags.Add(TEXT("Player"));
	}
}

void FRewindItem_Character::OnRecord(int32 Frame)
{
	FRewindItem_Actor::OnRecord(Frame);
	
	if (Character.WeakObject.IsValid())
	{
		FrameData.Append(Frame, Character.WeakObject.Get());
	}
}

void FRewindItem_Character::OnPostComponentCreated(UActorComponent* InComp)
{
	FRewindItem_Actor::OnPostComponentCreated(InComp);
	
	if (FRewindItem_SkMesh::IsPrimarySkMeshComp(Cast<USkeletalMeshComponent>(InComp)))
	{
		if (InComp != Character.WeakObject->GetMesh())
		{
			AddChildItem<FRewindItem_SkMesh>(InComp);
		}
	}
}

UE_ENABLE_OPTIMIZATION
}