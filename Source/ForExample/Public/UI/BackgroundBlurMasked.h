#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "BackgroundBlurMasked.generated.h"

UCLASS(MinimalAPI)
class UBackgroundBlurMasked : public UContentWidget
{
  GENERATED_UCLASS_BODY()

public:

  void ReleaseSlateResources(bool bReleaseChildren) override;

  void Serialize(FArchive & Ar) override;

  void PostLoad() override;

protected:
  /** UWidget interface */
  UClass * GetSlotClass() const override;

  TSharedRef<SWidget> RebuildWidget() override;

  void SynchronizeProperties() override;

  /** UPanelWidget interface */
  void OnSlotAdded(UPanelSlot * Slot) override;

  void OnSlotRemoved(UPanelSlot * Slot) override;

public:

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetPadding(const FMargin & InPadding);

  UFUNCTION(BlueprintPure)
  const FMargin & GetPadding() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

  UFUNCTION(BlueprintPure)
  EHorizontalAlignment GetHorizontalAlignment() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

  UFUNCTION(BlueprintPure)
  EVerticalAlignment GetVerticalAlignment() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetApplyAlphaToBlur(bool bInApplyAlphaToBlur);

  UFUNCTION(BlueprintPure)
  bool GetApplyAlphaToBlur() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetBlurStrength(float InStrength);

  UFUNCTION(BlueprintPure)
  float GetBlurStrength() const;

  UFUNCTION()
  void SetOverrideAutoRadiusCalculation(bool InOverrideAutoRadiusCalculation);

  UFUNCTION()
  bool GetOverrideAutoRadiusCalculation() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetBlurRadius(int32 InBlurRadius);

  UFUNCTION(BlueprintPure)
  int32 GetBlurRadius() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetCornerRadius(FVector4 InCornerRadius);

  UFUNCTION(BlueprintPure)
  FVector4 GetCornerRadius() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetLowQualityFallbackBrush(const FSlateBrush & InBrush);

  UFUNCTION(BlueprintPure)
  const FSlateBrush & GetLowQualityFallbackBrush() const;

  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetMask(const FSlateBrush & InBrush);

  UFUNCTION(BlueprintPure)
  const FSlateBrush & GetMask() const;

#if WITH_EDITOR
  void PostEditChangeProperty(struct FPropertyChangedEvent & PropertyChangedEvent) override;

  const FText GetPaletteCategory() override;
#endif

public:

  /** The padding area between the slot and the content it contains. */
  UPROPERTY(EditAnywhere, BlueprintSetter = SetPadding, BlueprintGetter = GetPadding, Category = Content)
  FMargin Padding;

  /** The alignment of the content horizontally. */
  UPROPERTY(EditAnywhere, BlueprintSetter = SetHorizontalAlignment, BlueprintGetter = GetHorizontalAlignment, Category = Content)
  TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

  /** The alignment of the content vertically. */
  UPROPERTY(EditAnywhere, BlueprintSetter = SetVerticalAlignment, BlueprintGetter = GetVerticalAlignment, Category = Content)
  TEnumAsByte<EVerticalAlignment> VerticalAlignment;

  /** True to modulate the strength of the blur based on the widget alpha. */
  UPROPERTY(EditAnywhere, BlueprintSetter = SetApplyAlphaToBlur, BlueprintGetter = GetApplyAlphaToBlur, Category = Content)
  bool bApplyAlphaToBlur;

  /**
  * How blurry the background is.  Larger numbers mean more blurry but will result in larger runtime cost on the GPU.
  */
  UPROPERTY(EditAnywhere, BlueprintSetter = SetBlurStrength, BlueprintGetter = GetBlurStrength, Category = Appearance, meta = (ClampMin = 0, ClampMax = 100))
  float BlurStrength;

  /** When OverrideAutoRadiusCalculation is set to true, BlurRadius is used for the radius of the blur. When false, it's automatically calculated using the BlurStength value. */
  UPROPERTY(Getter = GetOverrideAutoRadiusCalculation, Setter = SetOverrideAutoRadiusCalculation)
  bool bOverrideAutoRadiusCalculation;

  /**
  * This is the number of pixels which will be weighted in each direction from any given pixel when computing the blur
  * A larger value is more costly but allows for stronger blurs.
  */
  UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintSetter = SetBlurRadius, BlueprintGetter = GetBlurRadius, Category = Appearance, meta = (ClampMin = 0, ClampMax = 255, EditCondition = "bOverrideAutoRadiusCalculation"))
  int32 BlurRadius;

  /**
  * This is the number of pixels which will be weighted in each direction from any given pixel when computing the blur
  * A larger value is more costly but allows for stronger blurs.
  */
  UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintSetter = SetCornerRadius, BlueprintGetter = GetCornerRadius, Category = Appearance)
  FVector4 CornerRadius;

  /**
  * An image to draw instead of applying a blur when low quality override mode is enabled.
  * You can enable low quality mode for background blurs by setting the cvar Slate.ForceBackgroundBlurLowQualityOverride to 1.
  * This is usually done in the project's scalability settings
  */
  UPROPERTY(EditAnywhere, BlueprintSetter = SetLowQualityFallbackBrush, BlueprintGetter = GetLowQualityFallbackBrush, Category = Appearance)
  FSlateBrush LowQualityFallbackBrush;

  UPROPERTY(EditAnywhere, BlueprintSetter = SetMask, BlueprintGetter = GetMask, Category = Appearance)
  FSlateBrush Mask;

protected:

  TSharedPtr<class SBackgroundBlurMasked> BlurWidget;

};
