// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindFrameItem.h"
#include "Utils/FrameData.h"
#include "Utils/ObjectInfo.h"


class AZzRewindPreviewMeshActor;
class USkeletalMeshComponent;


namespace ZZ::Rewind
{

class ZZREWINDRUNTIME_API FRewindItem_SkMesh: public FRewindFrameItem
{
public:
	static bool IsPrimarySkMeshComp(const USkeletalMeshComponent* InComp);
	
	virtual void OnSetup(UObject* Owner) override;
	
	virtual void OnRecord(int32 Frame) override;
	virtual void OnRewind(UWorld* World, int32 Frame) override;
	virtual void OnRewindCleanup(UWorld* World) override;
	
	virtual FText GetDisplayName() const override;
	
	TObjectInfo<USkeletalMeshComponent> SkMeshComp;

	struct FSkMeshPoseData
	{
		// world transform
		FTransform ComponentTF;

		TArray<FTransform> PoseTF;

		FSkMeshPoseData(const FTransform& WorldTF, const TArray<FTransform>& InTf)
			: ComponentTF(WorldTF), PoseTF(InTf)
		{ }

		bool operator==(const FSkMeshPoseData&) const;
	};

	struct FSkMeshAssetData
	{
		TObjectInfo<USkeletalMesh> SkMesh;
		
		bool operator==(const FSkMeshAssetData&) const;
	};
	
	TFrameData<FSkMeshPoseData> PoseData;
	TRangeData<FSkMeshAssetData> AssetData;

	TMap<TWeakObjectPtr<UAnimMontage>, TSharedPtr<class FRewindItem_MontageInstance>> Montages;
	TSharedRef<FRewindItem_MontageInstance> FindOrCreateMontageItem(UAnimMontage* InMontage);
	
	TWeakObjectPtr<AZzRewindPreviewMeshActor> PreviewMeshActor;
	static AZzRewindPreviewMeshActor* SpawnPreviewActor(UWorld* World);
};

}
