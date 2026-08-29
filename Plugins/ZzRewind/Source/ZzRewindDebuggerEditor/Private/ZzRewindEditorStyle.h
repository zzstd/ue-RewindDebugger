// Copyright 2024-2026 zz studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"


class FZzRewindEditorStyle : FSlateStyleSet
{
public:
	FZzRewindEditorStyle();
	static void Initialize();
	static void Shutdown();

	static const ISlateStyle& Get();

private:
	void InternalInitialize();
	static TSharedPtr<FZzRewindEditorStyle> StyleInstance;
};

