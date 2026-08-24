// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Items/RewindItem_SkMesh.h"

#include "Items/RewindItem_MontageInstance.h"
#include "Items/ZzRewindPreviewActor.h"
#include "Styling/SlateIconFinder.h"

namespace ZZ::Rewind
{
	
bool FRewindItem_SkMesh::IsPrimarySkMeshComp(const USkeletalMeshComponent* InComp)
{
	return InComp && !InComp->LeaderPoseComponent.IsValid();
}

void FRewindItem_SkMesh::OnSetup(UObject* Owner)
{
	SkMeshComp = Cast<USkeletalMeshComponent>(Owner);
	check(SkMeshComp.WeakObject.IsValid());

	Icon = FSlateIconFinder::FindIconForClass(USkeletalMeshComponent::StaticClass());
}

void FRewindItem_SkMesh::OnRecord(int32 Frame)
{
	FRewindFrameItem::OnRecord(Frame);
	
	if (SkMeshComp.WeakObject.IsValid())
	{
		PoseData.Append(Frame,  {
			SkMeshComp.WeakObject->GetComponentTransform(),
			SkMeshComp.WeakObject->GetComponentSpaceTransforms()
		});
		
		AssetData.Append(Frame, {
			SkMeshComp.WeakObject->GetSkeletalMeshAsset()
		});

		if (auto AnimInst = SkMeshComp.WeakObject->GetAnimInstance())
		{
			for (auto MontageInst : AnimInst->MontageInstances)
			{
				if (MontageInst->IsPlaying())
				{
					FRewindItem_MontageInstance::FMontageInst MontageRef;
					MontageRef.Pos = MontageInst->GetPosition();
					MontageRef.PlayRate = MontageInst->GetPlayRate();
					MontageRef.Widget = MontageInst->GetWeight();
					MontageRef.DesiredWidget = MontageInst->GetDesiredWeight();

					FindOrCreateMontageItem(MontageInst->Montage)->MontageInstData.Append(Frame, MontageRef);
				}
			}
		}
	}
}

void FRewindItem_SkMesh::OnRewind(UWorld* World, int32 Frame)
{
	FRewindFrameItem::OnRewind(World, Frame);
	
	auto p = PoseData.Find(Frame);
	if (!p)
	{
		return;
	}
	
	AZzRewindPreviewMeshActor* Actor = PreviewMeshActor.Get();
	if (!Actor)
	{
		if (SkMeshComp.WeakObject.IsValid())
		{
			Actor = SpawnPreviewActor(World);
			Actor->SetupFromSkMeshComp(SkMeshComp.WeakObject.Get());
		}
		else if (auto FindSkMesh = AssetData.Find(Frame))
		{
			Actor = SpawnPreviewActor(World);
			Actor->SetupFromSkMeshAsset(FindSkMesh->SkMesh.WeakObject.Get());
		}
		
		PreviewMeshActor = Actor;
	}

	if (!Actor || !Actor->Mesh)
	{
		return;
	}
	
	Actor->UpdatePose(p->ComponentTF, p->PoseTF);
}

void FRewindItem_SkMesh::OnRewindCleanup(UWorld* World)
{
	FRewindFrameItem::OnRewindCleanup(World);
	
	if (AZzRewindPreviewMeshActor* Actor = PreviewMeshActor.Get())
	{
		Actor->GuardSkMeshComp();
		Actor->Destroy();
	}
}

FText FRewindItem_SkMesh::GetDisplayName() const
{
	return FText::FromString(SkMeshComp.ObjectName);
}

bool FRewindItem_SkMesh::FSkMeshPoseData::operator==(const FSkMeshPoseData& Other) const
{
	if (!ComponentTF.Equals(Other.ComponentTF))
	{
		return false;
	}

	if (PoseTF.Num() != Other.PoseTF.Num())
	{
		return false;
	}

	for (auto i = 0; i < PoseTF.Num(); i++)
	{
		if (!PoseTF[i].Equals(Other.PoseTF[i]))
		{
			return false;
		}
	}
	
	return true;
}

bool FRewindItem_SkMesh::FSkMeshAssetData::operator==(const FSkMeshAssetData& Other) const
{
	return SkMesh == Other.SkMesh;
}


TSharedRef<FRewindItem_MontageInstance> FRewindItem_SkMesh::FindOrCreateMontageItem(UAnimMontage* InMontage)
{
	if (TSharedPtr<FRewindItem_MontageInstance>* MontageItem = Montages.Find(InMontage))
	{
		return MontageItem->ToSharedRef();
	}
	
	auto NewItem = AddChildItem<FRewindItem_MontageInstance>(nullptr);
	Montages.Add(InMontage, NewItem);
	NewItem->Montage = InMontage;
	NewItem->MontageName = InMontage->GetName();
	return NewItem;
}

AZzRewindPreviewMeshActor* FRewindItem_SkMesh::SpawnPreviewActor(UWorld* World)
{
	FActorSpawnParameters ActorSpawnParameters;
#if WITH_EDITOR
	ActorSpawnParameters.bHideFromSceneOutliner = true;
#endif
	ActorSpawnParameters.ObjectFlags |= RF_Transient;
	return World->SpawnActor<AZzRewindPreviewMeshActor>(ActorSpawnParameters);
}
}
