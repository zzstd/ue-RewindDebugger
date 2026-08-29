// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Items/RewindItem_Camera.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Items/ZzRewindPreviewActor.h"
#include "Styling/SlateIconFinder.h"

namespace ZZ::Rewind
{

FRewindItemCameraData::FRewindItemCameraData(APlayerCameraManager* PCM)
{
	auto ViewInfo = PCM->GetCameraCacheView();
	Loc = ViewInfo.Location;
	Rot = ViewInfo.Rotation;
	FOV = ViewInfo.FOV;
	AspectRatio = ViewInfo.AspectRatio;
	AspectRatioAxisConstraint = ViewInfo.AspectRatioAxisConstraint;
}

bool FRewindItemCameraData::operator==(const FRewindItemCameraData& Other) const
{
	return Loc.Equals(Other.Loc)
		&& Rot.Equals(Other.Rot)
		&& FMath::IsNearlyEqual(FOV, Other.FOV)
		&& FMath::IsNearlyEqual(AspectRatio, Other.AspectRatio)
		&& AspectRatioAxisConstraint == Other.AspectRatioAxisConstraint;
}

FRewindItem_Camera::FRewindItem_Camera()
{
	Tags.Add("RewindCamera");
	Icon = FSlateIconFinder::FindIconForClass(UCameraComponent::StaticClass());
}

void FRewindItem_Camera::OnSetup(UObject* Owner)
{
	CameraManager = Cast<APlayerCameraManager>(Owner);
	check(CameraManager.WeakObject.IsValid());
}

FText FRewindItem_Camera::GetDisplayName() const
{
	return FText::FromString("Game Camera");
}

void FRewindItem_Camera::OnRecord(int32 Frame)
{
	FRewindFrameItem::OnRecord(Frame);
	
	if (CameraManager.WeakObject.IsValid())
	{
		FrameData.Append(Frame, CameraManager.WeakObject.Get());
	}
}

void FRewindItem_Camera::OnRewind(UWorld* World, int32 Frame)
{
	FRewindFrameItem::OnRewind(World, Frame);

	if (bEnableRewindCamera)
	{
		if (auto v = FrameData.Find(Frame))
		{
			UpdatePreviewCamera(World, *v);
		}
	}
}

void FRewindItem_Camera::OnRewindCleanup(UWorld* World)
{
	FRewindFrameItem::OnRewindCleanup(World);
	
	if (AZzRewindPreviewCameraActor* Actor = PreviewCameraActor.Get())
	{
		Actor->Destroy();
	}
}

AActor* FRewindItem_Camera::GetPreviewCameraActor() const
{
	return PreviewCameraActor.Get();
}

void FRewindItem_Camera::UpdatePreviewCamera(UWorld* World, const FRewindItemCameraData& InViewData)
{
	AZzRewindPreviewCameraActor* Actor = PreviewCameraActor.Get();
	if (!Actor)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags |= RF_Transient;
#if WITH_EDITOR
		SpawnParameters.bHideFromSceneOutliner = true;
#endif
		Actor = World->SpawnActor<AZzRewindPreviewCameraActor>(SpawnParameters);
		PreviewCameraActor = Actor;
	}

	if (!Actor || !Actor->Camera)
	{
		return;
	}

	Actor->Camera->SetWorldLocationAndRotation(InViewData.Loc, InViewData.Rot);
	Actor->Camera->SetFieldOfView(InViewData.FOV);
	Actor->Camera->SetAspectRatio(InViewData.AspectRatio);
	Actor->Camera->bOverrideAspectRatioAxisConstraint = InViewData.AspectRatioAxisConstraint.IsSet();
	if (InViewData.AspectRatioAxisConstraint.IsSet())
	{
		Actor->Camera->SetAspectRatioAxisConstraint(InViewData.AspectRatioAxisConstraint.GetValue());
	}
}

}
