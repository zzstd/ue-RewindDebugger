// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Items/RewindItem_Actor.h"

#include "ZzRewindItemRegister.h"
#include "Styling/SlateIconFinder.h"


namespace ZZ::Rewind
{

void FRewindItem_Actor::OnSetup(UObject* Owner)
{
	AActor* Actor = Cast<AActor>(Owner); 
	check(Actor);
	
	WeakActor = Actor;
	CachedActorNameOrLabel = Actor->GetActorNameOrLabel();

	Icon = FSlateIconFinder::FindIconForClass(WeakActor.ObjectClass.Get());
}

UObject* FRewindItem_Actor::GetObject() const
{
	return WeakActor.WeakObject.Pin().Get();
}

FText FRewindItem_Actor::GetDisplayName() const
{
	return FText::FromString(CachedActorNameOrLabel);
}

void FRewindItem_Actor::OnRecord(int32 Frame)
{
	if (AActor* StrongActor = WeakActor.WeakObject.Pin().Get())
	{
		UpdateActorComponents(StrongActor);

		if (bRecordActorData)
		{
			ActorRecordData.Append(Frame, StrongActor);
		}
	}

	FRewindFrameItem::OnRecord(Frame);
}

void FRewindItem_Actor::OnPostComponentCreated(UActorComponent* InComp)
{
	UZzRewindItemRegister::ForEachRegisterCallFunction(&UZzRewindItemRegister::OnPostComponentCreated, SharedThis(this), InComp);
}

void FRewindItem_Actor::OnPostComponentRemoved(UActorComponent* InComp)
{
	// todo
}

TArray<UActorComponent*> FRewindItem_Actor::GetCachedActorComponents() const
{
	TArray<UActorComponent*> Result;
	for (auto Comp : CachedActorComponents)
	{
		if (Comp.IsValid())
		{
			Result.Add(Comp.Get());
		}
	}

	return Result;
}

void FRewindItem_Actor::UpdateActorComponents(const AActor* InActor)
{
	CachedActorNameOrLabel = InActor->GetActorNameOrLabel();

	TArray<UActorComponent*> ActorAllComponents = InActor->GetComponents().Array();
	TArray<UActorComponent*> PendingRemovedComponents = GetCachedActorComponents();
	SetCachedActorComponents(ActorAllComponents);
		
	for (auto Comp : ActorAllComponents)
	{
		auto FindInx = PendingRemovedComponents.Find(Comp);
		if (FindInx >= 0)
		{
			PendingRemovedComponents.RemoveAt(FindInx);
		}
		else
		{
			OnPostComponentCreated(Comp);
		}
	}
		
	for (auto Comp : PendingRemovedComponents)
	{
		OnPostComponentRemoved(Comp);
	}
	
}

void FRewindItem_Actor::SetCachedActorComponents(const TArray<UActorComponent*>& InComps)
{
	CachedActorComponents.Empty();
	for (auto Comp : InComps)
	{
		CachedActorComponents.Add(Comp);
	}
}

TOptional<FTransform> FRewindItem_Actor::GetItemWorldTransform(int32 Frame) const
{
	if (auto p = ActorRecordData.Find(Frame))
	{
		return p->ActorTF;
	}

	return TOptional<FTransform>();
}

FRewindItemActorData::FRewindItemActorData(AActor* Actor)
{
	bActorHiddenInGame = Actor->IsHidden();
	Velocity = Actor->GetVelocity();
	ActorTF = Actor->GetTransform();
	Actor->GetActorBounds(true, BoundsOrigin, BoundsBoxExtent);
}

bool FRewindItemActorData::operator==(const FRewindItemActorData& Other) const
{
	return bActorHiddenInGame == Other.bActorHiddenInGame &&
		Velocity.Equals(Other.Velocity) &&
		ActorTF.Equals(Other.ActorTF) &&
		BoundsOrigin.Equals(Other.BoundsOrigin) &&
		BoundsBoxExtent.Equals(Other.BoundsBoxExtent)
		;
}

}