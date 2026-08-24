// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "Widgets/SVariantValueView.h"

#include "SlateOptMacros.h"
#include "ZzRewindRuntime.h"
#include "Widgets/Input/SHyperlink.h"
#include "UObject/UObjectGlobals.h"

namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

class SZzRewindVariantValue_Object : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZzRewindVariantValue_Object) {}
	SLATE_ATTRIBUTE(FText, HighlightText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TObjectInfo<UObject> Object)
	{
		UObject* Asset;
		{
			FString PackagePathString = FPackageName::ObjectPathToPackageName(FString(Object.ObjectPath));
			if (UPackage* Package = LoadPackage(nullptr, ToCStr(PackagePathString), LOAD_NoRedirects))
			{
				Package->FullyLoad();
				FString AssetName = FPaths::GetBaseFilename(Object.ObjectPath);
				Asset = FindObject<UObject>(Package, *AssetName);
			}
			else
			{
				// fallback for unsaved assets
				Asset = FindObject<UObject>(nullptr, *Object.ObjectPath);
			}
		}

		if (Asset)
		{
			TWeakObjectPtr WeakAsset = Asset;
			ChildSlot
			[
				SNew(SHyperlink)
				.Text(FText::FromString(Object.ObjectName))
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("SmallText"))
				.ToolTipText(FText::Format(INVTEXT("Open asset '{0}'"), FText::FromString(Object.ObjectPath)))
				.HighlightText(InArgs._HighlightText)
				.OnNavigate_Lambda([WeakAsset]()
				{
					if (WeakAsset.IsValid())
					{
						FZzRewindRuntime::Get()->OnOpenAssetEditorDelegate.Broadcast(WeakAsset.Get());
					}
				})
			];
		}
		else
		{
			// TODO: Add hyperlinks and distinguish assets, classes, actors, components, destroyed objects, and collected objects
			const bool bGarbageCollected = !Object.WeakObject.IsValid() && !Object.ObjectName.IsEmpty();
			ChildSlot
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s%s"), 
					bGarbageCollected  ? TEXT("(Invalid) " : TEXT("")),
					Object.ObjectName.IsEmpty() ? TEXT("None") : *Object.ObjectName
				)))
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("SmallText"))
				.HighlightText(InArgs._HighlightText)
			];
		}
	}
};

class SZzRewindVariantValueNode : public SMultiColumnTableRow<TSharedRef<FVariantValueNode>>
{
	using NodeRef = TSharedRef<FVariantValueNode>;
public:
	SLATE_BEGIN_ARGS(SZzRewindVariantValueNode) {}
	SLATE_ATTRIBUTE(FText, HighlightText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, NodeRef InNode)
	{
		Node = InNode;
		HighlightText = InArgs._HighlightText;
		
		SMultiColumnTableRow<NodeRef>::Construct(
			FSuperRowType::FArguments()
			.Padding(1.0f),
			InOwnerTable
		);
	}
	
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override
	{
		if (Node->bHidden)
		{
			return SNullWidget::NullWidget;
		}
		
		const bool bIsRoot = !Node->Parent.IsValid();

		if (InColumnName == TEXT("Name"))
		{
			return 
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(6, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SExpanderArrow, SharedThis(this))
						.IndentAmount(4)
					]
					
					+SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(FCoreStyle::Get().GetFontStyle(bIsRoot ? "ExpandableArea.TitleFont" : "SmallFont"))
						.Text(FText::FromString(Node->DisplayName))
						.HighlightText(HighlightText)
					]
				];
		}
		
		return
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SVariantValueView::MakeVariantValueWidget(Node->VariantValue, HighlightText)
				]
			];
	}

	TSharedPtr<FVariantValueNode> Node;
	TAttribute<FText> HighlightText;
};

void FVariantValueNode::Set(FVariantValue InValue)
{
	if (VariantValue != InValue)
	{
		bDirty = true;
		VariantValue = InValue;
	}
}

TSharedRef<FVariantValueNode> FVariantValueNode::Begin(FString InName)
{
	return FindOrAddChildByName(InName);
}

TSharedRef<FVariantValueNode> FVariantValueNode::Begin(FString InName, FVariantValue InValue)
{
	TSharedRef<FVariantValueNode> NewNode = FindOrAddChildByName(InName);
	NewNode->Set(InValue);
	return NewNode;
}

TSharedRef<FVariantValueNode> FVariantValueNode::End() const
{
	return Parent.Pin().ToSharedRef();
}

TSharedRef<FVariantValueNode> FVariantValueNode::AddValue(FString InName, FVariantValue InValue)
{
	TSharedRef<FVariantValueNode> NewNode = FindOrAddChildByName(InName);
	NewNode->Set(InValue);
	return SharedThis(this);
}

TSharedRef<FVariantValueNode> FVariantValueNode::AddValueReturn(FString InName, FVariantValue InValue)
{
	TSharedRef<FVariantValueNode> NewNode = FindOrAddChildByName(InName);
	NewNode->Set(InValue);
	return NewNode;
}

TSharedRef<FVariantValueNode> FVariantValueNode::Show(bool bShow)
{
	SetHidden(!bShow);
	return SharedThis(this);
}

TSharedRef<FVariantValueNode> FVariantValueNode::Expand(bool bExpand)
{
	SetExpansion(bExpand, false);
	return SharedThis(this);
}

TSharedRef<FVariantValueNode> FVariantValueNode::FindOrAddChildByName(FString InName)
{
	for (auto Child : Children)
	{
		if (Child->DisplayName == InName)
		{
			return Child;
		}
	}

	auto NewNode = Children.Add_GetRef(MakeShared<FVariantValueNode>());
	NewNode->Parent = SharedThis(this);
	NewNode->Widget = Widget;
	NewNode->DisplayName = InName;
	bDirty = true;
	return NewNode;
}

void FVariantValueNode::ClearChildren()
{
	if (!Children.IsEmpty())
	{
		bDirty = true;
		Children.Reset();
	}
}

int32 FVariantValueNode::GetChildrenNum() const
{
	return Children.Num();
}

void FVariantValueNode::SetHidden(bool bNewHidden)
{
	if (bHidden != bNewHidden)
	{
		bHidden = bNewHidden;
		bDirty = true;
	}
}

void FVariantValueNode::HiddenChildren()
{
	for (auto Child : Children)
	{
		Child->SetHidden(true);
	}
}

void FVariantValueNode::ShowChildren()
{
	for (auto Child : Children)
	{
		Child->SetHidden(false);
	}
}

void FVariantValueNode::SetHiddenR(bool bNewHidden)
{
	SetHidden(bNewHidden);
	
	for (auto Child : Children)
	{
		Child->SetHiddenR(bNewHidden);
	}
}

void FVariantValueNode::SetExpansion(bool bShouldExpansion, bool bRecursive)
{
	Widget.Pin()->VariantTreeView->SetItemExpansion(SharedThis(this), true);

	if (bRecursive)
	{
		for (auto Child : Children)
		{
			Child->SetExpansion(bShouldExpansion, true);
		}
	}
}

bool FVariantValueNode::HasDirty() const
{
	if (bDirty)
	{
		return true;
	}

	for (auto Child : Children)
	{
		if (Child->HasDirty())
		{
			return true;
		}
	}

	return false;
}

void FVariantValueNode::ClearDirty()
{
	bDirty = false;
	for (auto Child : Children)
	{
		Child->ClearDirty();
	}
}


void SVariantValueView::Construct(const FArguments& InArgs)
{
	Root = MakeShared<FVariantValueNode>();
	Root->Widget = SharedThis(this);
	
	ChildSlot
	[
		SAssignNew(VariantTreeView, STreeView<NodeRef>)
		.SelectionMode(ESelectionMode::Multi)
		.OnGenerateRow(this, &SVariantValueView::HandleGeneratePropertyRow)
		.OnGetChildren(this, &SVariantValueView::HandleGetChildren)
		//.OnExpansionChanged(this, &SVariantValueView::HandleExpansionChanged)
		//.OnContextMenuOpening(InArgs._OnContextMenuOpening)
		.TreeItemsSource(&Root->Children)
		.HeaderRow(
			SNew(SHeaderRow)
			.Visibility(EVisibility::Collapsed)
			
			+SHeaderRow::Column(TEXT("Name"))
			.DefaultLabel(INVTEXT("Name"))
			
			+SHeaderRow::Column(TEXT("Value"))
			.DefaultLabel(INVTEXT("Value"))
		)
	];
}

void SVariantValueView::RequestRefresh()
{
	//VariantTreeView->RequestTreeRefresh();
	if (Root->HasDirty())
	{
		Root->ClearDirty();
		VariantTreeView->RebuildList();
	}
}

void SVariantValueView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	RequestRefresh();
}

TSharedRef<SWidget> SVariantValueView::MakeVariantValueWidget(const FVariantValue& InValue, const TAttribute<FText>& InHighlightText)
{
	constexpr float ValueWidth = 125.f;
	auto MakeVectorEntryBox = [&InHighlightText, ValueWidth](const TSharedRef<SHorizontalBox>& InBox, double Value)
	{
		InBox->AddSlot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(ValueWidth / 3.f)
			[
				SNew(STextBlock)
				.IsEnabled(false)
				.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
				.Text(FText::AsNumber(Value))
				.HighlightText(InHighlightText)
			]
		];
	};
	
	switch (InValue.Type)
	{
	case FVariantValue::EValueType::Bool:
		return 
			SNew(SCheckBox)
			.IsEnabled(false)
			.IsChecked(InValue.Bool ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	case FVariantValue::EValueType::Int32:
		return
			SNew(SBox)
			.WidthOverride(ValueWidth)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
				.Text(FText::AsNumber(InValue.Int32))
				.HighlightText(InHighlightText)
			];
	case FVariantValue::EValueType::Float:
		return 
			SNew(SBox)
			.WidthOverride(ValueWidth)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
				.Text(FText::AsNumber(InValue.Float))
				.HighlightText(InHighlightText)
			];
	case FVariantValue::EValueType::Vector:
		{
			auto VectorBox = SNew(SHorizontalBox);
			MakeVectorEntryBox(VectorBox, InValue.Vector.X);
			MakeVectorEntryBox(VectorBox, InValue.Vector.Y);
			MakeVectorEntryBox(VectorBox, InValue.Vector.Z);
			return VectorBox;
		}
	case FVariantValue::EValueType::Vector2D:
		{
			auto VectorBox = SNew(SHorizontalBox);
			MakeVectorEntryBox(VectorBox, InValue.Vector2D.X);
			MakeVectorEntryBox(VectorBox, InValue.Vector2D.Y);
			return VectorBox;
		}
	case FVariantValue::EValueType::Transform:
		{
			auto VectorBox = SNew(SHorizontalBox);
			MakeVectorEntryBox(VectorBox, InValue.Transform.Location.X);
			MakeVectorEntryBox(VectorBox, InValue.Transform.Location.Y);
			MakeVectorEntryBox(VectorBox, InValue.Transform.Location.Z);
			
			auto VectorBox2 = SNew(SHorizontalBox);
			MakeVectorEntryBox(VectorBox2, InValue.Transform.Location.X);
			MakeVectorEntryBox(VectorBox2, InValue.Transform.Location.Y);
			MakeVectorEntryBox(VectorBox2, InValue.Transform.Location.Z);
			
			auto VectorBox3 = SNew(SHorizontalBox);
			MakeVectorEntryBox(VectorBox3, InValue.Transform.Location.X);
			MakeVectorEntryBox(VectorBox3, InValue.Transform.Location.Y);
			MakeVectorEntryBox(VectorBox3, InValue.Transform.Location.Z);
			
			return SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[VectorBox]
				+ SVerticalBox::Slot().AutoHeight()[VectorBox2]
				+ SVerticalBox::Slot().AutoHeight()[VectorBox3];
		}
	case FVariantValue::EValueType::Object:
		return SNew(SZzRewindVariantValue_Object, InValue.Object);
	case FVariantValue::EValueType::String:
		return 
			SNew(SBox)
			//.WidthOverride(ValueWidth)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::Get().GetFontStyle("SmallFont"))
				.Text(FText::FromString(InValue.String))
				.HighlightText(InHighlightText)
			];
	case FVariantValue::EValueType::None:
	default:
		return SNullWidget::NullWidget;
	}
}

TSharedRef<ITableRow> SVariantValueView::HandleGeneratePropertyRow(NodeRef InNode,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return 
		SNew(SZzRewindVariantValueNode, OwnerTable, InNode);
		//.HighlightText(MakeAttributeLambda([this](){ return FilterText; }));
}

void SVariantValueView::HandleGetChildren(NodeRef InNode, TArray<NodeRef>& OutChildren)
{
	if (!InNode->bHidden)
	{
		for (auto Child : InNode->Children)
		{
			if (!Child->bHidden)
			{
				OutChildren.Add(Child);
			}
		}
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}