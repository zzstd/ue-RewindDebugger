// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindFrameItem.h"
#include "Utils/FrameData.h"
#include "Utils/ObjectInfo.h"


namespace ZZ::Rewind
{

struct FRewindItemActorData
{
	bool bActorHiddenInGame;

	FVector Velocity;

	FTransform ActorTF;

	FVector BoundsOrigin;

	FVector BoundsBoxExtent;

	FRewindItemActorData(AActor* Actor);

	bool operator==(const FRewindItemActorData& Other) const;
};

/*
 * tracking components created or destroyed under the actor through OnPostComponentCreated. 
 * 
 * see: UZzRewindItemRegister::OnPostComponentCreated
 */
class ZZREWINDRUNTIME_API FRewindItem_Actor : public FRewindFrameItem
{
public:
	virtual void OnSetup(UObject* Owner) override;
	virtual UObject* GetObject() const override;
	virtual FText GetDisplayName() const override;

	virtual void OnRecord(int32 Frame) override;

	virtual void OnPostComponentCreated(UActorComponent* InComp);
	virtual void OnPostComponentRemoved(UActorComponent* InComp);
	
	TObjectInfo<AActor> WeakActor;
	FString CachedActorNameOrLabel;
	
	TArray<UActorComponent*> GetCachedActorComponents() const;
private:
	void UpdateActorComponents(const AActor* InActor);
	void SetCachedActorComponents(const TArray<UActorComponent*>& InComps);
	TArray<TWeakObjectPtr<UActorComponent>> CachedActorComponents;
public:
	TOptional<FTransform> GetItemWorldTransform(int32 Frame) const;
protected:
	// subclass can disable actor data record.
	bool bRecordActorData = true;
	TFrameData<FRewindItemActorData> ActorRecordData;
};

}