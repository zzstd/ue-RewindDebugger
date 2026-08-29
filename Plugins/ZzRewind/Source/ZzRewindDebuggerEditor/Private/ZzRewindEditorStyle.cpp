// Copyright 2024-2026 zz studio. All Rights Reserved.

#include "ZzRewindEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"

TSharedPtr<FZzRewindEditorStyle> FZzRewindEditorStyle::StyleInstance = nullptr;

FZzRewindEditorStyle::FZzRewindEditorStyle() 
	:FSlateStyleSet("ZzRewindEditorStyle")
{
}

void FZzRewindEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = MakeShared<FZzRewindEditorStyle>();
		StyleInstance->InternalInitialize();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FZzRewindEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

const ISlateStyle& FZzRewindEditorStyle::Get()
{
	return *StyleInstance;
}

void FZzRewindEditorStyle::InternalInitialize()
{
	const FVector2D Icon24x24(24.0f, 24.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon12x12(12.0f, 12.0f);
	
	SetParentStyleName(FAppStyle::GetAppStyleSetName());
	SetContentRoot(IPluginManager::Get().FindPlugin("ZzRewind")->GetBaseDir() / TEXT("Resources"));
	
	// Playback controls
	Set("ToolBar.StartRecording", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsRecord", Icon20x20));
	Set("ToolBar.StartRecording.StatusBar", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsRecord", Icon12x12));
	Set("ToolBar.StopRecording", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsStop", Icon20x20));
	Set("ToolBar.FirstFrame", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsToFront", Icon20x20));
	Set("ToolBar.PreviousFrame", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsToPrevious", Icon20x20));
	Set("ToolBar.ReversePlay", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsPlayReverse", Icon20x20));
	Set("ToolBar.Pause", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsPause", Icon20x20));
	Set("ToolBar.Play", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsPlayForward", Icon20x20));
	Set("ToolBar.NextFrame", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsToNext", Icon20x20));
	Set("ToolBar.LastFrame", new IMAGE_BRUSH_SVG("PlaybackControls/PlayControlsToEnd", Icon20x20));
}
