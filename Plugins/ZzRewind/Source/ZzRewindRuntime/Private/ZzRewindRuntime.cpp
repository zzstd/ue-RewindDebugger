// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindRuntime.h"

#include "ZzRewindRuntimeModule.h"
#include "ZzRewindItem.h"
#include "ZzRewindItemRegister.h"
#include "Utils/ZzRewindUtils.h"

#include "EngineUtils.h"

UE_DISABLE_OPTIMIZATION


namespace ZZ::Rewind
{

TSharedRef<FZzRewindRuntime> FZzRewindRuntime::Get()
{
	return FModuleManager::Get().LoadModuleChecked<FZzRewindRuntimeModule>(TEXT("ZzRewindRuntime")).RewindRuntime.ToSharedRef();
}

void FZzRewindRuntime::Init()
{
	Cleanup();
}

void FZzRewindRuntime::Cleanup()
{
	StopRecord();

	if (CachedRewindWorld.IsValid())
	{
		OnRewindCleanup(CachedRewindWorld.Get());
	}

	SetSelectedItem(nullptr);
	
	PrimaryItems.Empty();
	WeakAllItems.Empty();

	MaxFrame = 0;
	bHasRecordedFrame = false;
	ViewRange.X = -5;
	ViewRange.Y = 0;
	DesiredViewRange = ViewRange;
	
	OnCleanupDelegate.Broadcast();
}

void FZzRewindRuntime::ReceiveTick(const UWorld* World, int32 Frame)
{
	if (bShouldRecord && World && RecordingWorld.Get() == World)
	{
		OnRecord(Frame);
	}
}

void FZzRewindRuntime::OnRecord(int32 Frame)
{
	auto TempItems = PrimaryItems;
	for (auto Item : TempItems)
	{
		Item->OnRecord(Frame);
	}

	bHasRecordedFrame = true;
	MaxFrame = Frame;
	SetScrubFrame(Frame, EScrubTimeInfo::OnRecording);
	
	const double ViewLength = FUtils::Align(DesiredViewRange.Y - DesiredViewRange.X);
	ViewRange.Y = Frame / 60.0;
	ViewRange.X = ViewRange.Y - ViewLength;
	DesiredViewRange = ViewRange;
}

void FZzRewindRuntime::OnRewind(UWorld* World, int32 Frame)
{
	check(World)
	
	if (ensure(!CachedRewindWorld.IsValid() || CachedRewindWorld.Get() == World))
	{ 
		CachedRewindWorld = World;
	
		auto TempItems = PrimaryItems;
		for (auto Item : TempItems)
		{
			Item->OnRewind(World, Frame);
		}
	}
}

void FZzRewindRuntime::OnRewindCleanup(UWorld* World)
{
	check(World)

	if (CachedRewindWorld.Get() == World)
	{
		CachedRewindWorld.Reset();
	
		auto TempItems = PrimaryItems;
		for (auto Item : TempItems)
		{
			Item->OnRewindCleanup(World);
		}
	}
}

void FZzRewindRuntime::HandleWorldCleanup(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (RecordingWorld.Get() == World)
	{
		StopRecord();
	}

	if (CachedRewindWorld.Get() == World)
	{
		OnRewindCleanup(World);
	}
}

bool FZzRewindRuntime::HasRecordData() const
{
	return bHasRecordedFrame && !PrimaryItems.IsEmpty();
}

void FZzRewindRuntime::SetDesiredViewRange(double NewBeginTime, double NewEndTime)
{
	NewBeginTime = FUtils::Align(NewBeginTime);
	NewEndTime = FUtils::Align(NewEndTime);
	if (!FMath::IsFinite(NewBeginTime) || !FMath::IsFinite(NewEndTime))
	{
		return;
	}

	DesiredViewRange.X = NewBeginTime;
	DesiredViewRange.Y = FMath::Max(NewEndTime, NewBeginTime + 1.0 / 60.0);
}

void FZzRewindRuntime::UpdateSmoothViewRange(float InDeltaTime)
{
	ViewRange.X = FMath::FInterpTo(ViewRange.X, DesiredViewRange.X, InDeltaTime, 12);
	ViewRange.Y = FMath::FInterpTo(ViewRange.Y, DesiredViewRange.Y, InDeltaTime, 12);
}

void FZzRewindRuntime::SetScrubFrame(int32 NewFrame, EScrubTimeInfo::Type Info)
{
	NewFrame = FMath::Clamp(NewFrame, 0, MaxFrame);
	ScrubFrame = NewFrame;
	OnScrubFrameChanged.Broadcast(NewFrame, Info);
}

void FZzRewindRuntime::SetSelectedItem(TSharedPtr<FRewindItem> NewSelectedItem)
{
	SelectedItem = NewSelectedItem;
	OnTimelineSectionChanged.Broadcast();
}

void FZzRewindRuntime::StartRecord(const UWorld* InWorld)
{
	check(InWorld);
	
	Cleanup();
	
	bShouldRecord = true;
	RecordingWorld = InWorld;

	if (InWorld)
	{
		for (TActorIterator<AActor> It(InWorld); It; ++It)
		{
			UZzRewindItemRegister::ForEachRegisterCallFunction(&UZzRewindItemRegister::OnActorSpawned, *It);
		}

		OnActorSpawnedHandle = InWorld->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([](AActor* NewActor)
		{
			UZzRewindItemRegister::ForEachRegisterCallFunction(&UZzRewindItemRegister::OnActorSpawned, NewActor);
		}));
	}
}

void FZzRewindRuntime::StopRecord()
{
	if (!bShouldRecord)
		return;
	
	if (RecordingWorld.IsValid())
	{
		RecordingWorld->RemoveOnActorSpawnedHandler(OnActorSpawnedHandle);
	}
	
	bShouldRecord = false;
	RecordingWorld.Reset();
}

void FZzRewindRuntime::InternalAddItem(const TSharedRef<FRewindItem>& NewInst, UObject* Owner, bool Primary)
{
	if (Primary)
	{
		PrimaryItems.Add(NewInst);
	}
	else
	{
		WeakAllItems.Add(NewInst);
	}
	OnPreAddItemDelegate.Broadcast(NewInst, Owner, Primary);
	NewInst->OnSetup(Owner);
	OnPostAddItemDelegate.Broadcast(NewInst, Owner, Primary);
}

TSharedPtr<FRewindItem> FZzRewindRuntime::FindItemWithTag(FName InTag, bool bPrimaryOnly) const
{
	for (auto Item : PrimaryItems)
	{
		if (Item->Tags.Contains(InTag))
		{
			return Item;
		}
	}
	
	if (!bPrimaryOnly)
	{
		for (auto Item : WeakAllItems)
		{
			if (Item.IsValid() && Item.Pin()->Tags.Contains(InTag))
			{
				return Item.Pin();
			}
		}
	}
	
	return nullptr;
}

UE_ENABLE_OPTIMIZATION
}
