// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#include "ZzRewindType.h"

namespace ZZ::Rewind
{
	class FZzRewindRuntime;
	class FRewindItem;
	class FRewindItem_Camera;
	class STimelineView;
}

class FZzRewindDebuggerEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FZzRewindDebuggerEditorModule& Get();
	
    static const FName MainTabName;
    static const FName InspectorTabName;
    
    TSharedRef<SDockTab> SpawnRewindDebuggerTab(const FSpawnTabArgs& SpawnTabArgs);
    TSharedRef<SDockTab> SpawnRewindDebuggerInspectorTab(const FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<ZZ::Rewind::STimelineView> RewindTimelineWidget;
	TSharedPtr<SWidget> RewindInspectorWidget;

	TSharedPtr<ZZ::Rewind::FZzRewindRuntime> RewindRuntime;
private:
	FDelegateHandle PIEHandle[6];

	void OnTabManagerChanged();
	
	void OnPIEStarted(bool);
	void OnPIEPause(bool);
	void OnPIEResume(bool);
	void OnPIEEnd(bool);
	void OnWorldCleanup(UWorld* InWorld, bool, bool);
	
	void OnScrubFrameChanged(int32 NewFrame, ZZ::Rewind::EScrubTimeInfo::Type InType);
	void OnPostAddItem(TSharedRef<ZZ::Rewind::FRewindItem> InItem, UObject* Owner, bool bPrimary);
	void OnOpenAssetEditor(UObject* Asset);
	void TryInvokeInspector();

	void ResetDebuggerEditor();
	void CleanupRewind();

/************************************************** Rewind **********************************************************************************/
	TWeakPtr<ZZ::Rewind::FRewindItem_Camera> CameraItem;

	bool bIsEjected = false;

	UWorld* CachedRewindWorld = nullptr;
	int32 LastRewindFrame = 0;

	// Called when the rewind frame or playback state changes.
	void OnRewindUpdate(int32 NewFrame, bool bPlaying);

/************************************************** Camera Mode **********************************************************************************/
private:
	// 0: disable, 1: follow target actor, 2: replay recorded camera
	int32 CameraMode = 2;
	
	TWeakObjectPtr<class AZzRewindPreviewCameraActor> PreviewFollowCameraActor;

	void UpdateFollowCamera(int32 NewFrame, TSharedPtr<ZZ::Rewind::FRewindItem> InTargetItem);
	
	TWeakObjectPtr<class AActor> CachedRewindCameraActor;
	void SetViewportCameraTo(AActor* InPreviewActor);
public:
	void SetCameraMode(int32 InMode);
	int32 GetCameraMode() const;

/************************************************** Filtered Item **********************************************************************************/
private:
	struct FZzRewindDebuggerFilteredInfo
	{
		TWeakPtr<ZZ::Rewind::FRewindItem> Item;
		FString Name;
		bool bPlayer = false;
	} FilteredInfo;

	void TryInheritFilteredInfo(TSharedPtr<ZZ::Rewind::FRewindItem> InItem);
public:
	TSharedPtr<ZZ::Rewind::FRewindItem> GetFilteredItem() const;
	void UpdateFilteredPrimaryItems(TSharedPtr<ZZ::Rewind::FRewindItem> InItem);
};
