// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZzRewindPreviewActor.generated.h"

UCLASS()
class ZZREWINDRUNTIME_API AZzRewindPreviewMeshActor : public AActor
{
	GENERATED_BODY()
public:
	AZzRewindPreviewMeshActor();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PreviewMeshActor")
	USkeletalMeshComponent* Mesh;

	void SetupFromSkMeshComp(USkeletalMeshComponent* InMesh);
	void SetupFromSkMeshAsset(USkeletalMesh* InMeshAsset);
	void GuardSkMeshComp();
	
	void UpdatePose(const FTransform& WorldTransform, const TArray<FTransform>& InPose);
protected:
	using FSrcToCopedSkMeshMap = TMap<const USkeletalMeshComponent*, USkeletalMeshComponent*>;
	
	FSrcToCopedSkMeshMap CreateSubSkMeshWithLeaderPose(const USkeletalMeshComponent* InLeaderPose);
	void CreateSubStaticMesh(const AActor* SrcActor, FSrcToCopedSkMeshMap Map);
	
	void CopyAndHidden(USkeletalMeshComponent* Des, USkeletalMeshComponent* Src);
	
	UPROPERTY()
	TMap<USceneComponent*, bool> SrcMeshVVisMap;
};

UCLASS()
class ZZREWINDRUNTIME_API AZzRewindPreviewCameraActor : public AActor
{
	GENERATED_BODY()
public:
	AZzRewindPreviewCameraActor();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PreviewCameraActor")
	class UCameraComponent* Camera;
};
