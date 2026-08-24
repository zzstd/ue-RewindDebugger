// Copyright 2024-2026 zz studio. All Rights Reserved.


#include "ZzRewindBlueprintRegister.h"

#include "ZzRewindFrameItemBlueprintBase.h"
#include "ZzRewindItem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/SVariantValueView.h"

UE_DISABLE_OPTIMIZATION

namespace Private
{
	bool IsValidClass(const UClass* Class)
	{
		if (!Class)
			return false;
		if (Class->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
			return false;
		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
			return false;
		auto Name = Class->GetName();
		if (Name.Contains("SKEL_") || Name.Contains("REINST_"))
			return false;
		return true;
	}

	void GetAllBlueprintClasses(const UClass* BaseClass, TArray<FAssetData>& OutAssetData)
	{
		IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();

		FARFilter Filter;
		Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;
		Filter.PackagePaths.Add(FName("/Game"));
		Filter.bRecursivePaths = true;

		TArray<FAssetData> Assets;
		AssetRegistry.GetAssets(Filter, Assets);

		for (const FAssetData& Asset : Assets)
		{
			FString NativeParentClassPath;
			if (Asset.GetTagValue(FBlueprintTags::NativeParentClassPath, NativeParentClassPath))
			{
				const FString ObjectPath = FPackageName::ExportTextPathToObjectPath(NativeParentClassPath);
				const FTopLevelAssetPath TopLevelPath(ObjectPath);
				UClass* NativeParentClass = FSoftClassPath(TopLevelPath.ToString()).TryLoadClass<UObject>();
				
				if (NativeParentClass && NativeParentClass->IsChildOf(BaseClass))
				{
					OutAssetData.Add(Asset);
				}
			}
		}
	}
	
	// Get all native and Blueprint-derived classes of BaseClass.
	void GetAllClasses(const UClass* BaseClass, TArray<UClass*>& OutClasses)
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* Class = *It;
			if (Class && !Cast<UBlueprintGeneratedClass>(Class) && IsValidClass(Class) && Class->IsChildOf(BaseClass))
			{
				OutClasses.Add(Class);
			}
		}
		
		TArray<FAssetData> BlueprintAssets;
		GetAllBlueprintClasses(BaseClass, BlueprintAssets);
		for (auto Asset : BlueprintAssets)
		{
			UBlueprint* BP = Cast<UBlueprint>(Asset.GetAsset());
			if (BP && BP->GeneratedClass && IsValidClass(BP->GeneratedClass))
			{
				OutClasses.Add(BP->GeneratedClass);
			}
		}
	}
}

void UZzRewindBlueprintRegister::OnActorSpawned(AActor* NewActor)
{
	if (NewActor->IsA<AWorldSettings>())
	{
		BlueprintClasses.Empty();
		Private::GetAllClasses(UZzRewindFrameItemBlueprintBase::StaticClass(), BlueprintClasses);
	}
}

void UZzRewindBlueprintRegister::OnPostComponentCreated(TSharedRef<ZZ::Rewind::FRewindItem> Item, UActorComponent* Component)
{
	for (auto Class : BlueprintClasses)
	{
		auto CDO = Class->GetDefaultObject<UZzRewindFrameItemBlueprintBase>();
		if (CDO && CDO->SupportedClass && Component->GetClass()->IsChildOf(CDO->SupportedClass))
		{
			auto NewItemBP = NewObject<UZzRewindFrameItemBlueprintBase>(this, Class);
			NewItemBP->Object = Component;
			Item->AddChildItem<ZZ::Rewind::FRewindItem_BlueprintBase>(NewItemBP);
		}
	}
}

namespace ZZ::Rewind
{

void FRewindItem_BlueprintBase::OnSetup(UObject* Owner)
{
	ItemBP = TStrongObjectPtr(Cast<UZzRewindFrameItemBlueprintBase>(Owner));
	check(ItemBP);
	
	bPaintFrame = ItemBP->bPaintFrame;
	bPaintItem = ItemBP->bPaintItem;
	
	ItemBP->OnSetup();
}

void FRewindItem_BlueprintBase::OnRecord(int32 Frame)
{
	FRewindFrameItem::OnRecord(Frame);
	
	ItemBP->OnRecord(Frame);
}

FText FRewindItem_BlueprintBase::GetDisplayName() const
{
	return ItemBP->GetDisplayName();
}

TSharedRef<SWidget> FRewindItem_BlueprintBase::GenerateInspector(int32 Frame)
{
	if (auto MyWidget = ItemBP->GenerateInspectorWidget(Frame))
	{
		return MyWidget->TakeWidget();
	}
	
	if (!InspectorWidget)
	{
		SAssignNew(InspectorWidget, SVariantValueView);
	}

	auto RootNode = InspectorWidget->GetRoot();
	RootNode->HiddenChildren();
	{
		TArray<FString> Values;
		ItemBP->FullVariantInspector(Frame, Values);
		for (auto v : Values)
		{
			FString Name;
			FString Value;
			if (!v.Split(TEXT("|"), &Name, &Value))
			{
				//  If no delimiter is present, treat the entire value as the name.
				Name = v;
			}
			RootNode->AddValueReturn(Name, Value)->Show();
		}
	}
	return InspectorWidget.ToSharedRef();
}

FText FRewindItem_BlueprintBase::GetTooltipText(int32 Frame) const
{
	return FText::FromString(ItemBP->GetTooltipText(Frame));
}

void FRewindItem_BlueprintBase::OnPaintFrame(int32 Frame, FPaintFrameData& OutPaint) const
{
	OutPaint.Valid = ItemBP->OnPaintFrame(Frame, OutPaint.Color);
}

void FRewindItem_BlueprintBase::OnPaintItem(int32 Frame, FPaintItemData& OutItem) const
{
	OutItem.Valid = ItemBP->OnPaintItem(Frame, OutItem.StartFrame, OutItem.EndFrame, OutItem.Text, OutItem.Color);
}
}


UE_ENABLE_OPTIMIZATION