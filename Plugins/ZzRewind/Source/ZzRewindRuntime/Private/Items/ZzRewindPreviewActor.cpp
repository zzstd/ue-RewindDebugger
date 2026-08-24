// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Items/ZzRewindPreviewActor.h"

#include "Camera/CameraComponent.h"

UE_DISABLE_OPTIMIZATION

AZzRewindPreviewMeshActor::AZzRewindPreviewMeshActor()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetComponentTickEnabled(false);
	RootComponent = Mesh;
}

void AZzRewindPreviewMeshActor::SetupFromSkMeshComp(USkeletalMeshComponent* InMesh)
{
	check(InMesh);
	
	CopyAndHidden(Mesh, InMesh);
	
	FSrcToCopedSkMeshMap SkMap = CreateSubSkMeshWithLeaderPose(InMesh);
	SkMap.Add(InMesh, Mesh);
	//CreateSubStaticMesh(InMesh->GetOwner(), SkMap);
}

void AZzRewindPreviewMeshActor::SetupFromSkMeshAsset(USkeletalMesh* InMeshAsset)
{
	Mesh->SetSkeletalMeshAsset(InMeshAsset);
}

void AZzRewindPreviewMeshActor::GuardSkMeshComp()
{
	for (auto SkMesh : SrcMeshVVisMap)
	{
		SkMesh.Key->SetVisibility(SkMesh.Value);
	}
}

void AZzRewindPreviewMeshActor::UpdatePose(const FTransform& WorldTransform, const TArray<FTransform>& InPose)
{
	Mesh->SetWorldTransform(WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
	Mesh->GetEditableComponentSpaceTransforms() = InPose;
			
	Mesh->SetForcedLOD(0 + 1);
	Mesh->UpdateLODStatus();
	Mesh->UpdateChildTransforms(EUpdateTransformFlags::None, ETeleportType::TeleportPhysics);
	Mesh->MarkRenderStateDirty();
#if WITH_EDITOR
	Mesh->ApplyEditedComponentSpaceTransforms();
#endif
}

AZzRewindPreviewMeshActor::FSrcToCopedSkMeshMap AZzRewindPreviewMeshActor::CreateSubSkMeshWithLeaderPose(const USkeletalMeshComponent* InLeaderPose)
{
	auto GetOuterLeaderPoseComp = [](USkeletalMeshComponent* InComp)
	{
		USkinnedMeshComponent* CurComp = InComp;

		while (CurComp && CurComp->LeaderPoseComponent.IsValid())
		{
			CurComp = CurComp->LeaderPoseComponent.Get();
		}

		return CurComp;
	};
	
	FSrcToCopedSkMeshMap Result;
	
	TInlineComponentArray<USkeletalMeshComponent*> PrimComponents(InLeaderPose->GetOwner());
	for (auto SrcComp : PrimComponents)
	{
		if (SrcComp == InLeaderPose)
			continue;

		if (SrcComp->GetSkeletalMeshAsset() == nullptr)
			continue;

		if (GetOuterLeaderPoseComp(SrcComp) != InLeaderPose)
			continue;
	
		auto NewComp = Cast<USkeletalMeshComponent>(
			AddComponentByClass(USkeletalMeshComponent::StaticClass(), false, SrcComp->GetRelativeTransform(), false));
		
		NewComp->SetLeaderPoseComponent(Mesh);
		NewComp->PrimaryComponentTick.bStartWithTickEnabled = false;
		NewComp->PrimaryComponentTick.bCanEverTick = false;
		NewComp->SetAnimationMode(EAnimationMode::AnimationCustomMode);
		NewComp->RegisterComponentWithWorld(GetWorld());
		
		CopyAndHidden(NewComp, SrcComp);
		Result.Add(SrcComp, NewComp);
	}

	return Result;
}

void AZzRewindPreviewMeshActor::CreateSubStaticMesh(const AActor* SrcActor, FSrcToCopedSkMeshMap Map)
{
	TInlineComponentArray<UStaticMeshComponent*> SMComponents(SrcActor);
	for (auto StaticMeshComp : SMComponents)
	{
		if (StaticMeshComp->GetStaticMesh() == nullptr)
			continue;
		
		if (!StaticMeshComp->IsVisible())
			continue;
		
		USkeletalMeshComponent* ParentSKM = Cast<USkeletalMeshComponent>(StaticMeshComp->GetAttachParent());
		if (!ParentSKM)
			continue;
		
		if (!Map.Contains(ParentSKM))
			continue;
		
		auto NewComp = Cast<UStaticMeshComponent>(
			AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform::Identity, false));

		NewComp->SetStaticMesh(StaticMeshComp->GetStaticMesh());
		NewComp->OverrideMaterials = StaticMeshComp->OverrideMaterials;
		NewComp->AttachToComponent(Map[ParentSKM], FAttachmentTransformRules::KeepRelativeTransform, StaticMeshComp->GetAttachSocketName());
		NewComp->SetRelativeTransform(StaticMeshComp->GetRelativeTransform());
	}
}

void AZzRewindPreviewMeshActor::CopyAndHidden(USkeletalMeshComponent* Des, USkeletalMeshComponent* Src)
{
	Des->SetSkeletalMesh(Src->GetSkeletalMeshAsset());
	Des->OverrideMaterials = Src->OverrideMaterials;
	
	SrcMeshVVisMap.FindOrAdd(Src, Src->GetVisibleFlag());
	Src->SetVisibility(false);
}

AZzRewindPreviewCameraActor::AZzRewindPreviewCameraActor()
{
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetComponentTickEnabled(false);
	RootComponent = Camera;
}


UE_ENABLE_OPTIMIZATION