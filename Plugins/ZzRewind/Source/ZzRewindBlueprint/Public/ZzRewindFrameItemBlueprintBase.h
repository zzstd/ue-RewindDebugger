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
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UObject> SupportedClass;
	
	UPROPERTY(EditDefaultsOnly)
	bool bPaintFrame = false;
	
	UPROPERTY(EditDefaultsOnly)
	bool bPaintItem = false;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnSetup();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnRecord(int32 Frame);

	UFUNCTION(BlueprintImplementableEvent)
	FText GetDisplayName() const;
	
	// Use the `|` as a delimiter to distinguish between names and values
	UFUNCTION(BlueprintImplementableEvent)
	class UUserWidget* GenerateInspectorWidget(int32 Frame);
	
	// Use the `|` as a delimiter to distinguish between names and values
	UFUNCTION(BlueprintImplementableEvent)
	void FullVariantInspector(int32 Frame, TArray<FString>& Values);
	
	UFUNCTION(BlueprintImplementableEvent)
	FString GetTooltipText(int32 Frame) const;
	
	UFUNCTION(BlueprintImplementableEvent)
	bool OnPaintFrame(int32 Frame, FLinearColor& Color) const;
	
	UFUNCTION(BlueprintImplementableEvent)
	bool OnPaintItem(int32 Frame, int32& Start, int32& End, FString& Text, FLinearColor& Color) const;
	
	TWeakObjectPtr<UObject> Object;
	
	UFUNCTION(BlueprintPure)
	UObject* TryGetObject() const;
	
	virtual class UWorld* GetWorld() const override;
};