// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZzRewindFrameItemBlueprintBase.generated.h"

UCLASS(Abstract, Blueprintable)
class ZZREWINDBLUEPRINT_API UZzRewindFrameItemBlueprintBase : public UObject
{
	GENERATED_BODY()
public:
	// Actor or Component
	UPROPERTY(EditDefaultsOnly, Category="RewindItem")
	TSubclassOf<UObject> SupportedClass;
	
	UPROPERTY(EditDefaultsOnly, Category="RewindItem")
	bool bPaintFrame = false;
	
	UPROPERTY(EditDefaultsOnly, Category="RewindItem")
	bool bPaintItem = false;
	
	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	void OnSetup();
	
	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	void OnRecord(int32 Frame);

	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	FText GetDisplayName() const;
	
	// Use the `|` as a delimiter to distinguish between names and values
	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	class UUserWidget* GenerateInspectorWidget(int32 Frame);
	
	// Use the `|` as a delimiter to distinguish between names and values
	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	void FullVariantInspector(int32 Frame, TArray<FString>& Values);
	
	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	FString GetTooltipText(int32 Frame) const;
	
	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	bool OnPaintFrame(int32 Frame, FLinearColor& Color) const;
	
	UFUNCTION(BlueprintImplementableEvent, Category="RewindItem")
	bool OnPaintItem(int32 Frame, int32& Start, int32& End, FString& Text, FLinearColor& Color) const;
	
	TWeakObjectPtr<UObject> Object;
	
	UFUNCTION(BlueprintPure, Category="RewindItem")
	UObject* TryGetObject() const;
	
	virtual class UWorld* GetWorld() const override;
};