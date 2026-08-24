// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindFrameItem.h"
#include "Utils/FrameData.h"
#include "Utils/ObjectInfo.h"

class APlayerCameraManager;
class AZzRewindPreviewCameraActor;

namespace ZZ::Rewind
{

struct FRewindItemCameraData
{
	FVector Loc;
	FRotator Rot;
	float FOV;
	float AspectRatio;
	TOptional<EAspectRatioAxisConstraint> AspectRatioAxisConstraint;
	
	FRewindItemCameraData(APlayerCameraManager* PCM);
	
	bool operator==(const FRewindItemCameraData& Other) const;
};

// TODO: record switch view target, 
class ZZREWINDRUNTIME_API FRewindItem_Camera : public FRewindFrameItem
{
public:
	FRewindItem_Camera();
	
	virtual void OnSetup(UObject* Owner) override;
	virtual FText GetDisplayName() const override;

	virtual void OnRecord(int32 Frame) override;
	// TODO: debug the pos and rot if the viewport to checj for jumps
	//virtual void OnPaintFrame(int32 Frame, FPaintFrameData& OutPaint) const override;

	virtual void OnRewind(UWorld* World, int32 Frame) override;
	virtual void OnRewindCleanup(UWorld* World) override;

	// replay recorded camera
	bool bEnableRewindCamera = true;

	AActor* GetPreviewCameraActor() const;
private:
	TObjectInfo<APlayerCameraManager> CameraManager;
	
	TWeakObjectPtr<AZzRewindPreviewCameraActor> PreviewCameraActor;
	TFrameData<FRewindItemCameraData> FrameData;

	void UpdatePreviewCamera(UWorld* World, const FRewindItemCameraData& InViewData);
};

}