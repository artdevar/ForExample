#pragma once

#include "Framework/SlateDelegates.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Layout/Margin.h"
#include "Widgets/SCompoundWidget.h"

class SBackgroundBlurMasked : public SCompoundWidget
{
  SLATE_DECLARE_WIDGET(SBackgroundBlurMasked, SCompoundWidget)

public:

  SLATE_BEGIN_ARGS(SBackgroundBlurMasked)
    : _HAlign(HAlign_Fill)
    , _VAlign(VAlign_Fill)
    , _Padding(FMargin(2.0f))
    , _bApplyAlphaToBlur(true)
    , _BlurStrength(0.f)
    , _BlurRadius()
    , _CornerRadius(FVector4(0, 0, 0, 0))
    , _LowQualityFallbackBrush(nullptr)
    , _Mask(nullptr)
    {
      _Visibility = EVisibility::SelfHitTestInvisible;
    }
    SLATE_DEFAULT_SLOT(FArguments, Content)

    SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
    SLATE_ARGUMENT(EVerticalAlignment,   VAlign)
    SLATE_ATTRIBUTE(FMargin,             Padding)

    SLATE_ARGUMENT(bool,                bApplyAlphaToBlur)
    SLATE_ATTRIBUTE(float,              BlurStrength)
    SLATE_ATTRIBUTE(TOptional<int32>,   BlurRadius)
    SLATE_ATTRIBUTE(FVector4,           CornerRadius)
    SLATE_ARGUMENT(const FSlateBrush *, LowQualityFallbackBrush)
    SLATE_ARGUMENT(const FSlateBrush *, Mask)
  SLATE_END_ARGS()

public:

  SBackgroundBlurMasked();

  int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

  void Construct(const FArguments& InArgs);

  void SetContent(const TSharedRef<SWidget>& InContent);

  void SetApplyAlphaToBlur(bool bInApplyAlphaToBlur);

  void SetBlurRadius(TAttribute<TOptional<int32>> InBlurRadius);

  void SetBlurStrength(TAttribute<float> InStrength);

  void SetCornerRadius(TAttribute<FVector4> InCornerRadius);

  void SetLowQualityBackgroundBrush(const FSlateBrush * InBrush);

  void SetMask(const FSlateBrush * InBrush);

  void SetHAlign(EHorizontalAlignment HAlign);

  void SetVAlign(EVerticalAlignment VAlign);

  void SetPadding(TAttribute<FMargin> InPadding);

  bool IsUsingLowQualityFallbackBrush() const;

protected:

  void ComputeEffectiveKernelSize(float Strength, int32& OutKernelSize, int32& OutDownsampleAmount) const;

  /** @return an attribute reference of ColorAndOpacity */
  inline TSlateAttributeRef<float> GetBlurStrengthAttribute() const { return TSlateAttributeRef<float>{SharedThis(this), BlurStrengthAttribute}; }

  /** @return an attribute reference of ForegroundColor */
  inline TSlateAttributeRef<TOptional<int32>> GetBlurRadiusAttribute() const { return TSlateAttributeRef<TOptional<int32>>{SharedThis(this), BlurRadiusAttribute}; }

#if WITH_EDITORONLY_DATA
  TSlateDeprecatedTAttribute<float>            BlurStrength;
  TSlateDeprecatedTAttribute<TOptional<int32>> BlurRadius;
#endif

  bool                bApplyAlphaToBlur       = false;
  const FSlateBrush * LowQualityFallbackBrush = nullptr;
  const FSlateBrush * Mask                    = nullptr;

private:

  TSlateAttribute<float>            BlurStrengthAttribute;
  TSlateAttribute<TOptional<int32>> BlurRadiusAttribute;
  TSlateAttribute<FVector4>         CornerRadiusAttribute;
};