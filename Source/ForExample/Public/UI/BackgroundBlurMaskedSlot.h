#pragma once

#include "UObject/ScriptMacros.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/PanelSlot.h"
#include "Layout/Margin.h"
#include "BackgroundBlurMaskedSlot.generated.h"

class SBackgroundBlurMasked;
class UBackgroundBlurMasked;

/**
* The Slot for the UBackgroundBlurSlot, contains the widget displayed in a BackgroundBlur's single slot
*/
UCLASS(MinimalAPI)
class UBackgroundBlurMaskedSlot : public UPanelSlot
{
  GENERATED_UCLASS_BODY()

public:

  //~ Begin UPanelSlot interface
  void SynchronizeProperties() override;
  //~ End of UPanelSlot interface

  /** Builds the underlying slot for the slate BackgroundBlur. */
  void BuildSlot(const TSharedRef<SBackgroundBlurMasked> & InBackgroundBlur);

  void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR

  //~ Begin UObject interface
  void PostEditChangeProperty(struct FPropertyChangedEvent & PropertyChangedEvent) override;
  //~ End of UObject interface

#endif

  UFUNCTION(BlueprintPure)
  const FMargin & GetPadding() const;

  UFUNCTION(BlueprintCallable, Category = "Layout|Background Blur Slot")
  void SetPadding(const FMargin & InPadding);

  UFUNCTION(BlueprintPure)
  EHorizontalAlignment GetHorizontalAlignment() const;

  UFUNCTION(BlueprintCallable, Category = "Layout|Background Blur Slot")
  void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

  UFUNCTION(BlueprintPure)
  EVerticalAlignment GetVerticalAlignment() const;

  UFUNCTION(BlueprintCallable, Category = "Layout|Background Blur Slot")
  void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

protected:

  /** The padding area between the slot and the content it contains. */
  UPROPERTY(EditAnywhere, BlueprintSetter = "SetPadding", Category = "Layout|Background Blur Slot")
  FMargin Padding;

  /** The alignment of the object horizontally. */
  UPROPERTY(EditAnywhere, BlueprintSetter = "SetHorizontalAlignment", Category = "Layout|Background Blur Slot")
  TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

  /** The alignment of the object vertically. */
  UPROPERTY(EditAnywhere, BlueprintSetter = "SetVerticalAlignment", Category = "Layout|Background Blur Slot")
  TEnumAsByte<EVerticalAlignment> VerticalAlignment;

private:

  /** A pointer to the BackgroundBlur to allow us to adjust the size, padding...etc at runtime. */
  TSharedPtr<SBackgroundBlurMasked> BackgroundBlur;

  friend UBackgroundBlurMasked;
};
