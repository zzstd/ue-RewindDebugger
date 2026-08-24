// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindFrameItemBlueprintBase.h"

UObject* UZzRewindFrameItemBlueprintBase::TryGetObject() const
{
	return Object.Get();
}

UWorld* UZzRewindFrameItemBlueprintBase::GetWorld() const
{
#if WITH_EDITOR
	if (GEditor)
	{
		return GEditor->GetEditorWorldContext().World();
	}
#endif
	return nullptr;
}
