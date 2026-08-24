// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/VariantValue.h"
#include "Widgets/SCompoundWidget.h"


namespace ZZ::Rewind
{

class ZZREWINDRUNTIME_API FVariantValueNode : public TSharedFromThis<FVariantValueNode>
{
public:
	void Set(FVariantValue InValue);
	
	TSharedRef<FVariantValueNode> Begin(FString InName);
	TSharedRef<FVariantValueNode> Begin(FString InName, FVariantValue InValue);
	TSharedRef<FVariantValueNode> End() const;
	
	TSharedRef<FVariantValueNode> FindOrAddChildByName(FString InName);
	
	TSharedRef<FVariantValueNode> AddValue(FString InName, FVariantValue InValue);
	TSharedRef<FVariantValueNode> AddValueReturn(FString InName, FVariantValue InValue);
	
	TSharedRef<FVariantValueNode> Show(bool bShow = true);
	TSharedRef<FVariantValueNode> Expand(bool bExpand = true);
	
	template<typename T>
	TSharedRef<FVariantValueNode> SetValue_ArrayWithName(FString InName, TArray<TTuple<FName, T>> Array, bool bHiddenIfEmpty = false)
	{
		auto Node = Begin(InName, FString::Printf(TEXT(" %d Array element"), Array.Num()));
		if (bHiddenIfEmpty)
		{
			Node->SetHidden(Array.IsEmpty());
		}
		Node->HiddenChildren();
		for (const auto& Data : Array)
		{
			Node->Begin(Data.Key.ToString(), Data.Value)->Show();
		}
		return SharedThis(this);
	}

	void ClearChildren();
	int32 GetChildrenNum() const;
	
	void SetHidden(bool bNewHidden);
	void HiddenChildren();
	void ShowChildren();
	void SetHiddenR(bool bNewHidden);
	
	void SetExpansion(bool bShouldExpansion, bool bRecursive);

	bool HasDirty() const;

	void ClearDirty();
	
	FString DisplayName;
	
	FVariantValue VariantValue;
	
	TWeakPtr<FVariantValueNode> Parent;
	
	TArray<TSharedRef<FVariantValueNode>> Children;

	bool bHidden = false;
	
	bool bDirty = false;
	
	TWeakPtr<class SVariantValueView> Widget;
};

class ZZREWINDRUNTIME_API SVariantValueView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVariantValueView)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedRef<FVariantValueNode> GetRoot() const { return Root.ToSharedRef(); }
	
	void RequestRefresh();

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	static TSharedRef<SWidget> MakeVariantValueWidget(const FVariantValue& InValue, const TAttribute<FText>& InHighlightText);
private:
	using NodeRef = TSharedRef<FVariantValueNode>;
	friend FVariantValueNode; 
	
	TSharedPtr<FVariantValueNode> Root;
	TSharedPtr<STreeView<NodeRef>> VariantTreeView;
	TSharedRef<ITableRow> HandleGeneratePropertyRow(NodeRef InNode, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleGetChildren(NodeRef InNode, TArray<NodeRef>& OutChildren);
};


}