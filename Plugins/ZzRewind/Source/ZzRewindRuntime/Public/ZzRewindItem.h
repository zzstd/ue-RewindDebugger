// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


namespace ZZ::Rewind
{

class ZZREWINDRUNTIME_API FRewindItem : public TSharedFromThis<FRewindItem>
{
public:
	virtual ~FRewindItem() = default;
	
	virtual void OnSetup(UObject* Owner);
	virtual void OnRecord(int32 Frame);
	virtual void OnRewind(UWorld* World, int32 Frame);
	virtual void OnRewindCleanup(UWorld* World);
	
	virtual TSharedRef<SWidget> GenerateHeadWidget() = 0;
	virtual TSharedRef<SWidget> GenerateTrackWidget() = 0;
	virtual TSharedRef<SWidget> GenerateInspector(int32 Frame) = 0;
	
	virtual UObject* GetObject() const { return nullptr; }
	virtual FText GetDisplayName() const = 0;
	virtual FSlateIcon GetIcon() const { return Icon; }

	template<typename T>
	TSharedRef<T> AddChildItem(UObject* Owner)
	{
		TSharedRef<T> NewItemInst = MakeShared<T>();
		InternalAddChild(NewItemInst, Owner);
		return NewItemInst;
	}
	
	bool bHidden = false;
	
	// True means filtered out and not show
	bool bFiltered = false;
	
	/*
	 * Used to add tags to items (e.g. RewindCamera, Player), 
	 * which can be used to operate without knowing the specific type.
	 */
	TSet<FName> Tags;
	
	bool HasTag(FName InTag) const;

	TSharedPtr<FRewindItem> GetParent() const { return Parent.Pin(); }
	TArray<TSharedPtr<FRewindItem>> GetChildren() const { return Children; }
protected:
	FSlateIcon Icon;
	TWeakPtr<FRewindItem> Parent;
	TArray<TSharedPtr<FRewindItem>> Children;
	
	void InternalAddChild(TSharedRef<FRewindItem> NewItem, UObject* Owner);
};


}