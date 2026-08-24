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

	SetSelectedItem(nullptr);
	
	PrimaryItems.Empty();
	WeakAllItems.Empty();

	MaxFrame = 0;
	ViewRange.X = -5;
	ViewRange.Y = 0;
	DesiredViewRange = ViewRange;
	
	OnCleanupDelegate.Broadcast();
}

void FZzRewindRuntime::ReceiveTick(int32 Frame)
{
	if (bShouldRecord)
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
	
	if (ensure(!CachedRewindWorld || CachedRewindWorld == World))
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

	if (ensure(World == CachedRewindWorld))
	{
		CachedRewindWorld = nullptr;
	
		auto TempItems = PrimaryItems;
		for (auto Item : TempItems)
		{
			Item->OnRewindCleanup(World);
		}
	}
}

bool FZzRewindRuntime::HasRecordData() const
{
	return MaxFrame > 0 && !PrimaryItems.IsEmpty();
}

void FZzRewindRuntime::SetDesiredViewRange(double NewBeginTime, double NewEndTime)
{
	DesiredViewRange.X = FUtils::Align(NewBeginTime);
	DesiredViewRange.Y = FUtils::Align(NewEndTime);
}

void FZzRewindRuntime::UpdateSmoothViewRange(float InDeltaTime)
{
	ViewRange.X = FMath::FInterpTo(ViewRange.X, DesiredViewRange.X, InDeltaTime, 12);
	ViewRange.Y = FMath::FInterpTo(ViewRange.Y, DesiredViewRange.Y, InDeltaTime, 12);
}

void FZzRewindRuntime::SetScrubFrame(int32 NewFrame, EScrubTimeInfo::Type Info)
{
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