// Copyright 2024-2026 zz studio. All Rights Reserved.

#include "ZzRewindDebuggerEditorModule.h"


#include "ZzRewindRuntime.h"
#include "ZzRewindEditorSetting.h"
#include "ZzRewindItem.h"
#include "ZzRewindItemRegister.h"
#include "ZzRewindSetting.h"
#include "Items/RewindItem_Actor.h"
#include "Items/RewindItem_Camera.h"
#include "Items/ZzRewindPreviewActor.h"
#include "Widgets/STimelineEditorToolbar.h"
#include "Widgets/STimelineView.h"
#include "Widgets/SRewindInspector.h"

#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Kismet/GameplayStatics.h"
#include "SLevelViewport.h"
#include "UnrealEdGlobals.h"
#include "ZzRewindEditorStyle.h"
#include "ZzRewindRuntimeModule.h"
#include "GameFramework/Character.h"
#include "Editor/UnrealEdEngine.h"
#include "Kismet2/DebuggerCommands.h"

#define LOCTEXT_NAMESPACE "FZzRewindDebuggerEditorModule"

const FName FZzRewindDebuggerEditorModule::MainTabName("ZzRewindDebugger");
const FName FZzRewindDebuggerEditorModule::InspectorTabName("ZzRewindDebuggerInspector");

UE_DISABLE_OPTIMIZATION

void FZzRewindDebuggerEditorModule::StartupModule()
{
	using namespace ZZ::Rewind;
	
	FZzRewindEditorStyle::Initialize();
	RewindRuntime = FZzRewindRuntime::Get();

	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	PIEHandle[0] = LevelEditorModule.OnTabManagerChanged().AddRaw(this, &FZzRewindDebuggerEditorModule::OnTabManagerChanged);
	OnTabManagerChanged();
	
	PIEHandle[1] = FEditorDelegates::PostPIEStarted.AddRaw(this, &FZzRewindDebuggerEditorModule::OnPIEStarted);
	PIEHandle[2] = FEditorDelegates::PausePIE.AddRaw(this, &FZzRewindDebuggerEditorModule::OnPIEPause);
	PIEHandle[3] = FEditorDelegates::ResumePIE.AddRaw(this, &FZzRewindDebuggerEditorModule::OnPIEResume);
	PIEHandle[4] = FEditorDelegates::ShutdownPIE.AddRaw(this, &FZzRewindDebuggerEditorModule::OnPIEEnd);

	PIEHandle[5] = FWorldDelegates::OnWorldCleanup.AddRaw(this, &FZzRewindDebuggerEditorModule::OnWorldCleanup);

	FZzRewindRuntimeModule::Get().ToolbarBuilder.BindLambda([]()
	{
		return SNew(STimelineEditorToolbar);
	});

	RewindRuntime->OnScrubFrameChanged.AddRaw(this, &FZzRewindDebuggerEditorModule::OnScrubFrameChanged);
	RewindRuntime->OnPostAddItemDelegate.AddRaw(this, &FZzRewindDebuggerEditorModule::OnPostAddItem);
	RewindRuntime->OnOpenAssetEditorDelegate.AddRaw(this, &FZzRewindDebuggerEditorModule::OnOpenAssetEditor);
	RewindRuntime->OnTimelineSectionChanged.AddRaw(this, &FZzRewindDebuggerEditorModule::TryInvokeInspector);
}
 
void FZzRewindDebuggerEditorModule::ShutdownModule()
{
	CleanupRewind();

	if (TSharedPtr<SDockTab> Tab = RewindTimelineTab.Pin())
	{
		Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback());
		Tab->SetContent(SNullWidget::NullWidget);
		Tab->RequestCloseTab();
	}
	if (TSharedPtr<SDockTab> Tab = RewindInspectorTab.Pin())
	{
		Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback());
		Tab->SetContent(SNullWidget::NullWidget);
		Tab->RequestCloseTab();
	}
	RewindTimelineTab.Reset();
	RewindInspectorTab.Reset();
	RewindTimelineWidget.Reset();
	RewindInspectorWidget.Reset();

	if (RewindRuntime)
	{
		RewindRuntime->OnScrubFrameChanged.RemoveAll(this);
		RewindRuntime->OnPostAddItemDelegate.RemoveAll(this);
		RewindRuntime->OnOpenAssetEditorDelegate.RemoveAll(this);
		RewindRuntime->OnTimelineSectionChanged.RemoveAll(this);
	}

	if (FZzRewindRuntimeModule* RuntimeModule = FModuleManager::GetModulePtr<FZzRewindRuntimeModule>("ZzRewindRuntime"))
	{
		RuntimeModule->ToolbarBuilder.Unbind();
	}

	if (TSharedPtr<FTabManager> TabManager = RegisteredTabManager.Pin())
	{
		TabManager->UnregisterTabSpawner(MainTabName);
		TabManager->UnregisterTabSpawner(InspectorTabName);
	}
	RegisteredTabManager.Reset();

	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		LevelEditorModule->OnTabManagerChanged().Remove(PIEHandle[0]);
	}
	
	FEditorDelegates::PostPIEStarted.Remove(PIEHandle[1]);
	FEditorDelegates::PausePIE.Remove(PIEHandle[2]);
	FEditorDelegates::ResumePIE.Remove(PIEHandle[3]);
	FEditorDelegates::ShutdownPIE.Remove(PIEHandle[4]);
	
	FWorldDelegates::OnWorldCleanup.Remove(PIEHandle[5]);
	RewindRuntime.Reset();
	FZzRewindEditorStyle::Shutdown();
}

FZzRewindDebuggerEditorModule& FZzRewindDebuggerEditorModule::Get()
{
	return FModuleManager::Get().LoadModuleChecked<FZzRewindDebuggerEditorModule>(TEXT("ZzRewindDebuggerEditor"));
}

void FZzRewindDebuggerEditorModule::OnTabManagerChanged()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	if (!LevelEditorTabManager)
	{
		return;
	}

	if (TSharedPtr<FTabManager> OldTabManager = RegisteredTabManager.Pin())
	{
		OldTabManager->UnregisterTabSpawner(MainTabName);
		OldTabManager->UnregisterTabSpawner(InspectorTabName);
	}
	RegisteredTabManager = LevelEditorTabManager;

	LevelEditorTabManager->RegisterTabSpawner(
		MainTabName, FOnSpawnTab::CreateRaw(this, &FZzRewindDebuggerEditorModule::SpawnRewindDebuggerTab))
		.SetDisplayName(INVTEXT("Zz Rewind Debugger"))
		.SetIcon(FSlateIcon())
		.SetTooltipText(INVTEXT("Open Zz Rewind Debugger"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory());

	LevelEditorTabManager->RegisterTabSpawner(
		InspectorTabName, FOnSpawnTab::CreateRaw(this, &FZzRewindDebuggerEditorModule::SpawnRewindDebuggerInspectorTab))
		.SetDisplayName(INVTEXT("Zz Rewind Debugger Inspector"))
		.SetIcon(FSlateIcon())
		.SetTooltipText(INVTEXT("Open Zz Rewind Debugger Inspector"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory());
}

TSharedRef<SDockTab> FZzRewindDebuggerEditorModule::SpawnRewindDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	using namespace ZZ::Rewind;
	
	const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.OnTabClosed_Lambda([this](TSharedRef<SDockTab> ClosedTab)
		{
			if (RewindTimelineTab.Pin() == ClosedTab)
			{
				RewindTimelineWidget.Reset();
				RewindTimelineTab.Reset();
				ResetDebuggerEditor();
			}
		});

	MajorTab->SetContent(SAssignNew(RewindTimelineWidget, STimelineView));
	RewindTimelineTab = MajorTab;
	return MajorTab;
}

TSharedRef<SDockTab> FZzRewindDebuggerEditorModule::SpawnRewindDebuggerInspectorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	using namespace ZZ::Rewind;
	
	const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.OnTabClosed_Lambda([this](TSharedRef<SDockTab> ClosedTab)
		{
			if (RewindInspectorTab.Pin() == ClosedTab)
			{
				RewindInspectorWidget.Reset();
				RewindInspectorTab.Reset();
			}
		});

	MajorTab->SetContent(SAssignNew(RewindInspectorWidget, SRewindInspector));
	RewindInspectorTab = MajorTab;
	return MajorTab;
}

void FZzRewindDebuggerEditorModule::OnPIEStarted(bool)
{
	ResetDebuggerEditor();

	if (UZzRewindEditorSetting::Get().bAutoRecord)
	{
		if (auto PIEWorldContext = GEditor->GetPIEWorldContext())
		{
			RewindRuntime->StartRecord(PIEWorldContext->World());
		}
	}
}

void FZzRewindDebuggerEditorModule::OnPIEPause(bool)
{
	if (UZzRewindEditorSetting::Get().bAutoEject && FPlayWorldCommandCallbacks::IsInPIE())
	{
		bool CanEject = false;
		for (auto It = GUnrealEd->SlatePlayInEditorMap.CreateIterator(); It; ++It)
		{
			CanEject = CanEject || It.Value().DestinationSlateViewport.IsValid();
		}

		if (CanEject)
		{
			bIsEjected = true;
			GEditor->RequestToggleBetweenPIEandSIE();
		}
	}
	
}

void FZzRewindDebuggerEditorModule::OnPIEResume(bool)
{
	if (bIsEjected)
	{
		GEditor->RequestToggleBetweenPIEandSIE();
	}
	
	ResetDebuggerEditor();
}

void FZzRewindDebuggerEditorModule::OnPIEEnd(bool)
{
	RewindRuntime->StopRecord();
}

void FZzRewindDebuggerEditorModule::OnWorldCleanup(UWorld* InWorld, bool, bool)
{
	if (InWorld == CachedRewindWorld)
	{
		CleanupRewind();
	}
}

void FZzRewindDebuggerEditorModule::OnScrubFrameChanged(int32 NewFrame, ZZ::Rewind::EScrubTimeInfo::Type InType)
{
	using namespace ZZ::Rewind;
	
	UWorld* RewindWorld;
	if (auto PIEWorldContext = GEditor->GetPIEWorldContext())
	{
		RewindWorld = PIEWorldContext->World();
	}
	else
	{
		RewindWorld = GEditor->GetEditorWorldContext().World();
	}

	if (RewindWorld != CachedRewindWorld)
	{
		CleanupRewind();
	}

	if (RewindWorld)
	{
		if (InType == EScrubTimeInfo::OnRecording)
		{
			if (CachedRewindWorld)
			{
				CleanupRewind();
			}
		}
		else
		{
			// If the frame changes but is not recorded, the playback system will be enabled
			CachedRewindWorld = RewindWorld;
			if (LastRewindFrame != NewFrame)
			{
				check(RewindRuntime->ScrubFrame == NewFrame);
				OnRewindUpdate(NewFrame, InType == EScrubTimeInfo::OnRewinding);
				LastRewindFrame = NewFrame;
			}
		}
	}
}

void FZzRewindDebuggerEditorModule::OnPostAddItem(TSharedRef<ZZ::Rewind::FRewindItem> InItem, UObject* Owner, bool bPrimary)
{
	using namespace ZZ::Rewind;
	
	if (InItem->HasTag("RewindCamera"))
	{
		// Record our camera for easy playback of lens trajectories
		CameraItem = StaticCastSharedRef<FRewindItem_Camera>(InItem);
		
		CameraItem.Pin()->bEnableRewindCamera = CameraMode == 2;
	}

	if (bPrimary)
	{
		if (FilteredInfo.Item.IsValid())
		{
			// If filtering is enabled, the current ITem needs to be filtered
			InItem->bFiltered = true;
		}
		else
		{
			TryInheritFilteredInfo(InItem);
		}
	}
}

void FZzRewindDebuggerEditorModule::OnOpenAssetEditor(UObject* Asset)
{
	if (UAssetEditorSubsystem* AssetEditorSS = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSS->OpenEditorForAsset(Asset);
	}
}

void FZzRewindDebuggerEditorModule::TryInvokeInspector()
{
	const TSharedPtr<FTabManager> TabManager = RegisteredTabManager.Pin();
	if (!TabManager)
	{
		return;
	}

	// if we now have no selection, don't force the tab into focus - this happens when tracks disappear and can cause PIE to lose focus while playing
	const bool bInvokeAsInactive = !RewindRuntime->SelectedItem.IsValid();
	TabManager->TryInvokeTab(InspectorTabName, bInvokeAsInactive);

}

void FZzRewindDebuggerEditorModule::ResetDebuggerEditor()
{
	bIsEjected = false;
	CleanupRewind();
}

void FZzRewindDebuggerEditorModule::CleanupRewind()
{
	LastRewindFrame = 0;
	CleanupPreviewCamera();

	if (CachedRewindWorld)
	{
		RewindRuntime->OnRewindCleanup(CachedRewindWorld);
		CachedRewindWorld = nullptr;
	}

	CameraItem.Reset();
}

void FZzRewindDebuggerEditorModule::OnRewindUpdate(int32 NewFrame, bool bPlaying)
{
	RewindRuntime->OnRewind(CachedRewindWorld, NewFrame);
	
	if (CameraMode == 0)
	{
		// disable camera and pass.
	}
	else if (CameraMode == 1)
	{
		// follow rewind camera to target actor.
		if (CachedRewindWorld)
		{
			auto TargetItem = FilteredInfo.Item.Pin();
			if (!TargetItem.IsValid())
			{
				// If there is no target, use the player as the target
				TargetItem = RewindRuntime->FindItemWithTag("Player");
			}
			
			if (TargetItem.IsValid())
			{
				UpdateFollowCamera(NewFrame, TargetItem);
			}
		}
	}
	else if (CameraMode == 2)
	{
		// replay recorded camera. updated by FRewindItemCamera::OnRewind().
		if (CameraItem.IsValid())
		{
			SetViewportCameraTo(CameraItem.Pin()->GetPreviewCameraActor());
		}
	}
}
	

void FZzRewindDebuggerEditorModule::UpdateFollowCamera(int32 NewFrame, TSharedPtr<ZZ::Rewind::FRewindItem> InTargetItem)
{
	using namespace ZZ::Rewind;
	
	auto GetItemLocation = [InTargetItem](int32 InFrame)
	{
		// Primary items are expected to be FRewindItem_Actor instances.
		auto ItemTF = StaticCastSharedPtr<FRewindItem_Actor>(InTargetItem)->GetItemWorldTransform(InFrame);
		return ItemTF.Get(FTransform::Identity).GetLocation();
	};
	
	AZzRewindPreviewCameraActor* Actor = PreviewFollowCameraActor.Get();
	if (!Actor)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags |= RF_Transient;
		SpawnParameters.bHideFromSceneOutliner = true;
		Actor = CachedRewindWorld->SpawnActor<AZzRewindPreviewCameraActor>(SpawnParameters);
		if (!Actor)
		{
			return;
		}

		if (CachedRewindWorld->CachedViewInfoRenderedLastFrame.Num() > 0)
		{
			FMatrix ViewToWorld = CachedRewindWorld->CachedViewInfoRenderedLastFrame[0].ViewToWorld;
			FMatrix ViewRot = FRotationMatrix::MakeFromX(ViewToWorld.GetUnitAxis(EAxis::Z));
			auto CamLoc = ViewToWorld.GetOrigin();
			auto CamLook = ViewToWorld.GetUnitAxis(EAxis::Z);
			Actor->SetActorLocationAndRotation(CamLoc, CamLook.Rotation());
		}
		else
		{
			Actor->SetActorLocation(GetItemLocation(NewFrame));
		}
		PreviewFollowCameraActor = Actor;
	}
	
	if (!Actor || !Actor->Camera)
		return;
	
	if (LastRewindFrame == 0)
		return;
	
	const FVector TargetPos = GetItemLocation(NewFrame);
	const FVector LastTargetPos = GetItemLocation(LastRewindFrame);
	
	SetViewportCameraTo(Actor);
	Actor->AddActorWorldOffset(TargetPos - LastTargetPos);
}

void FZzRewindDebuggerEditorModule::SetViewportCameraTo(AActor* InPreviewActor)
{
	if (CachedRewindCameraActor != InPreviewActor)
	{
		CachedRewindCameraActor = InPreviewActor;
		
		FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		if (auto LevelViewport = LevelEditor.GetFirstActiveLevelViewport())
		{
			FLevelEditorViewportClient& LevelViewportClient = LevelViewport->GetLevelViewportClient();
			LevelViewportClient.SetActorLock(InPreviewActor);
		}
	}
}

void FZzRewindDebuggerEditorModule::CleanupPreviewCamera()
{
	if (FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		if (TSharedPtr<SLevelViewport> LevelViewport = LevelEditor->GetFirstActiveLevelViewport())
		{
			FLevelEditorViewportClient& ViewportClient = LevelViewport->GetLevelViewportClient();
			if (ViewportClient.GetActiveActorLock() == CachedRewindCameraActor)
			{
				ViewportClient.SetActorLock(nullptr);
			}
		}
	}

	CachedRewindCameraActor.Reset();
	if (AZzRewindPreviewCameraActor* Actor = PreviewFollowCameraActor.Get())
	{
		Actor->Destroy();
	}
	PreviewFollowCameraActor.Reset();
}

void FZzRewindDebuggerEditorModule::SetCameraMode(int32 InMode)
{
	CameraMode = InMode;

	if (CameraItem.IsValid())
	{
		CameraItem.Pin()->bEnableRewindCamera = CameraMode == 2;
	}
	
	if (CameraMode != 1)
	{
		CleanupPreviewCamera();
	}
}

int32 FZzRewindDebuggerEditorModule::GetCameraMode() const
{
	return CameraMode;
}

void FZzRewindDebuggerEditorModule::TryInheritFilteredInfo(TSharedPtr<ZZ::Rewind::FRewindItem> InItem)
{
	// Restore the previous filter when a new play session creates matching rewind items
	if (!FilteredInfo.Item.IsValid())
	{
		if (FilteredInfo.bPlayer)
		{
			if (auto MyPawn = Cast<APawn>(InItem->GetObject()))
			{
				if (MyPawn->IsPlayerControlled())
				{
					UpdateFilteredPrimaryItems(InItem);
				}
			}
		}
		else if (!FilteredInfo.Name.IsEmpty())
		{
			if (FilteredInfo.Name == InItem->GetDisplayName().ToString())
			{
				UpdateFilteredPrimaryItems(InItem);
			}
		}
	}
}

TSharedPtr<ZZ::Rewind::FRewindItem> FZzRewindDebuggerEditorModule::GetFilteredItem() const
{
	return FilteredInfo.Item.Pin();
}

void FZzRewindDebuggerEditorModule::UpdateFilteredPrimaryItems(TSharedPtr<ZZ::Rewind::FRewindItem> InItem)
{
	FilteredInfo.Item = InItem;

	if (InItem.IsValid())
	{
		FilteredInfo.Name = InItem->GetDisplayName().ToString();
		FilteredInfo.bPlayer = InItem->Tags.Contains(TEXT("Player"));
	}
	else
	{
		FilteredInfo.Name.Empty();
		FilteredInfo.bPlayer = false;
	}

	for (auto PrimaryItem : RewindRuntime->PrimaryItems)
	{
		PrimaryItem->bFiltered = InItem && PrimaryItem != InItem;
	}
	RewindRuntime->bRequestViewRefresh = true;
}

UE_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZzRewindDebuggerEditorModule, ZzRewindDebuggerEditor)
