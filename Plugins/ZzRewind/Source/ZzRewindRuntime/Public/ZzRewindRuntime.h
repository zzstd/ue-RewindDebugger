// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZzRewindType.h"

namespace ZZ::Rewind
{

class FRewindItem;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FZzRewindDebuggerAddItem, TSharedRef<FRewindItem>, UObject*, bool /*Primary*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FZzRewindDebuggerScrubFrameChanged, int32, EScrubTimeInfo::Type);
DECLARE_MULTICAST_DELEGATE_OneParam(FZzRewindDebuggerOpenAssetEditor, UObject*);
	
class ZZREWINDRUNTIME_API FZzRewindRuntime : public TSharedFromThis<FZzRewindRuntime>
{
public:
	static TSharedRef<FZzRewindRuntime> Get();

	void Init();

	void Cleanup();

	// Receive world tick，will call OnRecord to record one frame
	void ReceiveTick(const UWorld* World, int32 Frame);

	void StartRecord(const UWorld* InWorld);
	
	void StopRecord();

	bool IsRecording() const { return bShouldRecord; }

	void OnRecord(int32 Frame);

	void OnRewind(UWorld* World, int32 Frame);
	void OnRewindCleanup(UWorld* World);
	void HandleWorldCleanup(UWorld* World);

	bool HasRecordData() const;

	template<typename T = FRewindItem>
	TSharedRef<T> NewPrimaryItem(UObject* Owner)
	{
		TSharedRef<T> NewPrimaryItem = MakeShared<T>();
		InternalAddItem(NewPrimaryItem, Owner, true);
		return NewPrimaryItem;
	}

	void SetDesiredViewRange(double NewBeginTime, double NewEndTime);

	// called with STimelineView::Tick
	void UpdateSmoothViewRange(float InDeltaTime);

	void SetScrubFrame(int32 NewFrame, EScrubTimeInfo::Type Info);

	void SetSelectedItem(TSharedPtr<FRewindItem> NewSelectedItem);

	TSharedPtr<FRewindItem> SelectedItem;
	int32 ScrubFrame = 0;
	int32 MaxFrame = 0;
	bool bHasRecordedFrame = false;
	FVector2D ViewRange;
	// smooth view motion
	FVector2D DesiredViewRange;

	FZzRewindDebuggerAddItem OnPreAddItemDelegate;
	FZzRewindDebuggerAddItem OnPostAddItemDelegate;
	FSimpleMulticastDelegate OnCleanupDelegate;
	FSimpleMulticastDelegate OnTimelineSectionChanged;
	FZzRewindDebuggerScrubFrameChanged OnScrubFrameChanged;

	FZzRewindDebuggerOpenAssetEditor OnOpenAssetEditorDelegate;

	bool bShouldRecord = false;
	TWeakObjectPtr<const UWorld> RecordingWorld;
	FDelegateHandle OnActorSpawnedHandle;

	friend class FRewindItem;
	void InternalAddItem(const TSharedRef<FRewindItem>& NewInst, UObject* Owner, bool Primary);

	TArray<TSharedPtr<FRewindItem>> PrimaryItems;
	TArray<TWeakPtr<FRewindItem>> WeakAllItems; // no primary

	TSharedPtr<FRewindItem> FindItemWithTag(FName InTag, bool bPrimaryOnly = true) const;

	bool bRequestViewRefresh = false;
	TWeakObjectPtr<UWorld> CachedRewindWorld;
};


}
	
