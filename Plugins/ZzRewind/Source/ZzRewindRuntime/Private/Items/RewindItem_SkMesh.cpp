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
					MontageRef.Weight = MontageInst->GetWeight();
					MontageRef.DesiredWeight = MontageInst->GetDesiredWeight();

					FindOrCreateMontageItem(MontageInst->Montage)->MontageInstData.Append(Frame, MontageRef);
				}
			}
		}
	}
}

void FRewindItem_SkMesh::OnRewind(UWorld* World, int32 Frame)
{
	FRewindFrameItem::OnRewind(World, Frame);
	
	const FSkMeshPoseData* PoseDataAtFrame = PoseData.Find(Frame);
	const FSkMeshAssetData* MeshDataAtFrame = AssetData.Find(Frame);
	USkeletalMesh* RecordedMesh = MeshDataAtFrame ? MeshDataAtFrame->SkMesh.WeakObject.Get() : nullptr;
	if (!PoseDataAtFrame || !RecordedMesh)
	{
		return;
	}
	
	AZzRewindPreviewMeshActor* Actor = PreviewMeshActor.Get();
	if (!Actor)
	{
		Actor = SpawnPreviewActor(World);
		if (Actor)
		{
			if (SkMeshComp.WeakObject.IsValid())
			{
				Actor->SetupFromSkMeshComp(SkMeshComp.WeakObject.Get());
			}
			else
			{
				Actor->SetupFromSkMeshAsset(RecordedMesh);
			}
		}
		
		PreviewMeshActor = Actor;
	}

	if (!Actor || !Actor->Mesh)
	{
		return;
	}

	if (Actor->Mesh->GetSkeletalMeshAsset() != RecordedMesh)
	{
		Actor->SetupFromSkMeshAsset(RecordedMesh);
	}
	
	TArray<FTransform> Pose;
	Pose.Reserve(PoseDataAtFrame->PoseTF.Num());
	for (const FTransform3f& Transform : PoseDataAtFrame->PoseTF)
	{
		Pose.Emplace(Transform);
	}

	Actor->UpdatePose(PoseDataAtFrame->ComponentTF, Pose);
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

FRewindItem_SkMesh::FSkMeshPoseData::FSkMeshPoseData(const FTransform& WorldTF, const TArray<FTransform>& InTf)
	: ComponentTF(WorldTF)
{
	PoseTF.Reserve(InTf.Num());
	for (const FTransform& Transform : InTf)
	{
		PoseTF.Emplace(Transform);
	}
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
