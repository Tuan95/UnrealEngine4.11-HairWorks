// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CheckBoxWidgetStyle.h"

#include "CheckBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCheckBoxComponentStateChanged, bool, bIsChecked );

/**
 * Check box widget
 */
UCLASS(ClassGroup=UserInterface)
class UMG_API UCheckBox : public UContentWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** The checkbox bar style */
	UPROPERTY(VisibleAnywhere, Instanced, Category="Style")
	TSubobjectPtr<UCheckBoxWidgetStyle> CheckBoxStyle;

	/** Style of the check box */
	UPROPERTY()
	USlateWidgetStyleAsset* Style_DEPRECATED;

	/** Image to use when the checkbox is unchecked */
	UPROPERTY()
	USlateBrushAsset* UncheckedImage_DEPRECATED;
	
	/** Image to use when the checkbox is unchecked and hovered */
	UPROPERTY()
	USlateBrushAsset* UncheckedHoveredImage_DEPRECATED;
	
	/** Image to use when the checkbox is unchecked and pressed */
	UPROPERTY()
	USlateBrushAsset* UncheckedPressedImage_DEPRECATED;
	
	/** Image to use when the checkbox is checked */
	UPROPERTY()
	USlateBrushAsset* CheckedImage_DEPRECATED;
	
	/** Image to use when the checkbox is checked and hovered */
	UPROPERTY()
	USlateBrushAsset* CheckedHoveredImage_DEPRECATED;
	
	/** Image to use when the checkbox is checked and pressed */
	UPROPERTY()
	USlateBrushAsset* CheckedPressedImage_DEPRECATED;
	
	/** Image to use when the checkbox is in an ambiguous state and hovered */
	UPROPERTY()
	USlateBrushAsset* UndeterminedImage_DEPRECATED;
	
	/** Image to use when the checkbox is checked and hovered */
	UPROPERTY()
	USlateBrushAsset* UndeterminedHoveredImage_DEPRECATED;
	
	/** Image to use when the checkbox is in an ambiguous state and pressed */
	UPROPERTY()
	USlateBrushAsset* UndeterminedPressedImage_DEPRECATED;

	/** Whether the check box is currently in a checked state */
	UPROPERTY(EditDefaultsOnly, Category=Appearance)
	TEnumAsByte<ESlateCheckBoxState::Type> CheckedState;

	/** A bindable delegate for the IsChecked. */
	UPROPERTY()
	FGetCheckBoxState CheckedStateDelegate;

	/** How the content of the toggle button should align within the given space */
	UPROPERTY(EditDefaultsOnly, Category=Appearance)
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

	/** Spacing between the check box image and its content */
	UPROPERTY(EditDefaultsOnly, Category=Appearance)
	FMargin Padding;

	/** The color of the background border */
	UPROPERTY(EditDefaultsOnly, Category=Appearance)
	FSlateColor BorderBackgroundColor;

public:

	/** Called when the checked state has changed */
	UPROPERTY(BlueprintAssignable)
	FOnCheckBoxComponentStateChanged OnCheckStateChanged;

	/** Returns true if this button is currently pressed */
	UFUNCTION(BlueprintCallable, Category="Widget")
	bool IsPressed() const;
	
	/** Returns true if the checkbox is currently checked */
	UFUNCTION(BlueprintCallable, Category="Widget")
	bool IsChecked() const;

	/** @return the full current checked state. */
	UFUNCTION(BlueprintCallable, Category="Widget")
	ESlateCheckBoxState::Type GetCheckedState() const;

	/** Sets the checked state. */
	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetIsChecked(bool InIsChecked);

	/** Sets the checked state. */
	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetCheckedState(ESlateCheckBoxState::Type InCheckedState);
	
	// UWidget interface
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

	// UVisual interface
	virtual void ReleaseNativeWidget() override;
	// End of UVisual interface

	// Begin UObject interface
	virtual void PostLoad() override;
	// End of UObject interface

#if WITH_EDITOR
	virtual const FSlateBrush* GetEditorIcon() override;
	virtual const FText GetToolboxCategory() override;
#endif

protected:

	// UPanelWidget
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	void SlateOnCheckStateChangedCallback(ESlateCheckBoxState::Type NewState);
	
protected:
	TSharedPtr<SCheckBox> MyCheckbox;
};
