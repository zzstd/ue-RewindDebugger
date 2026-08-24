// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "STimelineItemEditorFilter.h"

#include "ZzRewindDebuggerEditorModule.h"
#include "ZzRewindRuntime.h"
#include "ZzRewindEditorSetting.h"
#include "ZzRewindItem.h"

#include "SlateOptMacros.h"
#include "Styling/SlateIconFinder.h"
#include "ActorPickerMode.h"
#include "Selection.h"
#include "Kismet/GameplayStatics.h"

namespace ZZ::Rewind
{
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STimelineItemEditorFilter::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SComboButton)
			.OnGetMenuContent(this, &STimelineItemEditorFilter::MakeSelectActorMenu)
			.ButtonContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(3)
				[
					SNew(SImage)
					.Image_Lambda([this]() -> const FSlateBrush*
					{
						if (GetFilteredItem().IsValid())
						{
							return GetFilteredItem()->GetIcon().GetIcon();
						}
						return nullptr;
					})
				]

				+ SHorizontalBox::Slot()
				.Padding(3)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						if (GetFilteredItem().IsValid())
						{
							return GetFilteredItem()->GetDisplayName();
						}
						return INVTEXT("");
					} )
				]
			]
		]
		
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.OnClicked(this, &STimelineItemEditorFilter::OnSelectActorClicked)
			.ToolTipText(INVTEXT("Select Target Actor in Scene"))
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.DesiredSizeOverride(FVector2D(16))
					.Image(FSlateIconFinder::FindIcon("RewindDebugger.SelectActor").GetIcon())
				]
			]
		]
	];
}

TSharedRef<SWidget> STimelineItemEditorFilter::MakeSelectActorMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.AddSearchWidget();

	{
		MenuBuilder.BeginSection("Common", INVTEXT("Common"));

		MenuBuilder.AddMenuEntry(INVTEXT("Reset Filters"),
			INVTEXT(""),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this](){
				SetShowItem(nullptr);
			}),
			FCanExecuteAction::CreateLambda([this]() { return GetFilteredItem().IsValid(); })));

		if (auto PIEWorldContext = GEditor->GetPIEWorldContext())
		{
			if (AActor* ControlledPawn = UGameplayStatics::GetPlayerPawn(PIEWorldContext->World(), 0))
			{
				FText SelectedLabel = FText::FromString(TEXT("Player 0  (") + ControlledPawn->GetActorLabel());
				FSlateIcon ActorIcon = FSlateIconFinder::FindIconForClass(ControlledPawn->GetClass());

				MenuBuilder.AddMenuEntry(SelectedLabel, FText(), ActorIcon, FExecuteAction::CreateLambda([this] 
				{
					FSlateApplication::Get().DismissAllMenus();
					SetShowItem(FindItemByPlayer(1));
				}));
			}
		}

		MenuBuilder.EndSection();
	}
	
	{
		MenuBuilder.BeginSection("From Selection Section", INVTEXT("From Scene Selection"));
		// Menu entry for each actor selected in the scene
		TArray<AActor*> SelectedActors;
		GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);

		if (SelectedActors.Num() > 0)
		{
			for(AActor* SelectedActor : SelectedActors)
			{
				FText SelectedLabel = FText::FromString(SelectedActor->GetActorLabel());
				FSlateIcon ActorIcon = FSlateIconFinder::FindIconForClass(SelectedActors[0]->GetClass());

				auto FindItem = FindItemByActor(SelectedActor);
				auto SetShowAction = FUIAction(
					FExecuteAction::CreateLambda([this, FindItem]
					{
						FSlateApplication::Get().DismissAllMenus();
						SetShowItem(FindItem);
					}) ,
					FCanExecuteAction::CreateLambda([FindItem]() { return FindItem.IsValid(); })
				);
				MenuBuilder.AddMenuEntry(SelectedLabel, FText(), ActorIcon, SetShowAction);
			}
		}
		else
		{
			MenuBuilder.AddMenuEntry(INVTEXT("No scene selection"),
			 INVTEXT("If you select an object in the scene, then it will be listed here"),
			 FSlateIcon(),
			  FUIAction(FExecuteAction(),
				  FCanExecuteAction::CreateLambda([](){return false;})));
		}
		MenuBuilder.EndSection();
	}
	
	MenuBuilder.BeginSection("From Recording Section", INVTEXT("From Recording:"));
	for (auto Item : FZzRewindRuntime::Get()->PrimaryItems)
	{
		FText ItemLabel = Item->GetDisplayName();
		FSlateIcon ActorIcon = Item->GetIcon();
		MenuBuilder.AddMenuEntry(ItemLabel, FText(), ActorIcon, FExecuteAction::CreateLambda([this, Item]()
		{
			FSlateApplication::Get().DismissAllMenus();
			SetShowItem(Item);
		}));
	}
	MenuBuilder.EndSection();
	
	return MenuBuilder.MakeWidget();
}

FReply STimelineItemEditorFilter::OnSelectActorClicked()
{
	FActorPickerModeModule& ActorPickerMode = FModuleManager::Get().GetModuleChecked<FActorPickerModeModule>("ActorPickerMode");
	
	const bool bShouldForceEject = GEditor->PlayWorld && !GEditor->bIsSimulatingInEditor;
	if (bShouldForceEject)
	{
		// Eject PIE
		GEditor->RequestToggleBetweenPIEandSIE();
	}

	ActorPickerMode.BeginActorPickingMode(
		FOnGetAllowedClasses(), 
		FOnShouldFilterActor(),
		FOnActorSelected::CreateLambda([this, bShouldForceEject](AActor* InActor)
		{
			SetShowItem(FindItemByActor(InActor));

			if (bShouldForceEject && GEditor->bIsSimulatingInEditor)
			{
				// If we force ejected PIE, revert this after actor selection.
				GEditor->RequestToggleBetweenPIEandSIE();
			}
		}));
	
	return FReply::Handled();
}

TSharedPtr<FRewindItem> STimelineItemEditorFilter::FindItemByActor(const AActor* Actor) const
{
	if (Actor)
	{
		for (auto PrimaryItem : FZzRewindRuntime::Get()->PrimaryItems)
		{
			if (PrimaryItem->GetObject() == Actor)
			{
				return PrimaryItem;
			}
		}
	}

	return nullptr;
}

TSharedPtr<FRewindItem> STimelineItemEditorFilter::FindItemByPlayer(int32 InPlayerIndex) const
{
	if (InPlayerIndex == 1)
	{
		for (auto PrimaryItem : FZzRewindRuntime::Get()->PrimaryItems)
		{
			if (auto Pawn = Cast<APawn>(PrimaryItem->GetObject()))
			{
				if (Pawn->IsPlayerControlled())
				{
					return PrimaryItem;
				}
			}
			else if (PrimaryItem->Tags.Contains(TEXT("Player")))
			{
				return PrimaryItem;
			}
		}
	}

	return nullptr;
}

void STimelineItemEditorFilter::SetShowItem(TSharedPtr<FRewindItem> InItem)
{
	FZzRewindDebuggerEditorModule::Get().UpdateFilteredPrimaryItems(InItem);
}

TSharedPtr<FRewindItem> STimelineItemEditorFilter::GetFilteredItem() const
{
	return FZzRewindDebuggerEditorModule::Get().GetFilteredItem();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
}