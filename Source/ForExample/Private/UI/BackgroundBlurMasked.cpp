#include "UI/BackgroundBlurMasked.h"
#include "UI/SBackgroundBlurMasked.h"
#include "UI/BackgroundBlurMaskedSlot.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/DrawElements.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "ObjectEditorUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BackgroundBlurMasked)

#define LOCTEXT_NAMESPACE "UMG"

UBackgroundBlurMasked::UBackgroundBlurMasked(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  , Padding(0.f, 0.f)
  , bApplyAlphaToBlur(true)
  , BlurStrength(0.f)
  , bOverrideAutoRadiusCalculation(false)
  , BlurRadius(0)
  , CornerRadius(0,0,0,0)
  , LowQualityFallbackBrush(FSlateNoResource())
{
  bIsVariable = false;
  SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);
}

void UBackgroundBlurMasked::ReleaseSlateResources(bool bReleaseChildren)
{
  Super::ReleaseSlateResources(bReleaseChildren);

  BlurWidget.Reset();
}

void UBackgroundBlurMasked::Serialize(FArchive & Ar)
{
  Super::Serialize(Ar);

  Ar.UsingCustomVersion(FEditorObjectVersion::GUID);
}

void UBackgroundBlurMasked::PostLoad()
{
  Super::PostLoad();

  if (GetLinkerCustomVersion(FEditorObjectVersion::GUID) < FEditorObjectVersion::AddedBackgroundBlurContentSlot)
  {
    //Convert existing slot to new background blur slot slot.
    if (UPanelSlot * PanelSlot = GetContentSlot())
    {
      if (!PanelSlot->IsA<UBackgroundBlurMaskedSlot>())
      {
        auto * BlurSlot         = NewObject<UBackgroundBlurMaskedSlot>(this);
        BlurSlot->Content       = PanelSlot->Content;
        BlurSlot->Content->Slot = BlurSlot;
        BlurSlot->Parent        = this;

        Slots[0] = BlurSlot;

        // We don't want anyone considering this panel slot for anything, so mark it pending kill.  Otherwise
        // it will confuse the pass we do when doing template validation when it finds it outered to the blur widget.
        PanelSlot->MarkAsGarbage();
      }
    }
  }
}

UClass * UBackgroundBlurMasked::GetSlotClass() const
{
  return UBackgroundBlurMaskedSlot::StaticClass();
}

TSharedRef<SWidget> UBackgroundBlurMasked::RebuildWidget()
{
  BlurWidget = SNew(SBackgroundBlurMasked);

  if (GetChildrenCount() > 0)
    Cast<UBackgroundBlurMaskedSlot>(GetContentSlot())->BuildSlot(BlurWidget.ToSharedRef());

  return BlurWidget.ToSharedRef();
}

void UBackgroundBlurMasked::SynchronizeProperties()
{
  Super::SynchronizeProperties();

  if (BlurWidget.IsValid())
  {
    BlurWidget->SetPadding(Padding);
    BlurWidget->SetHAlign(HorizontalAlignment);
    BlurWidget->SetVAlign(VerticalAlignment);

    BlurWidget->SetApplyAlphaToBlur(bApplyAlphaToBlur);
    BlurWidget->SetBlurRadius(bOverrideAutoRadiusCalculation ? BlurRadius : TOptional<int32>());
    BlurWidget->SetBlurStrength(BlurStrength);
    BlurWidget->SetLowQualityBackgroundBrush(&LowQualityFallbackBrush);
    BlurWidget->SetMask(&Mask);
    BlurWidget->SetCornerRadius(CornerRadius);
  }
}

void UBackgroundBlurMasked::OnSlotAdded(UPanelSlot * InSlot)
{
  UBackgroundBlurMaskedSlot * BackgroundBlurSlot = CastChecked<UBackgroundBlurMaskedSlot>(InSlot);
  BackgroundBlurSlot->SetPadding(Padding);
  BackgroundBlurSlot->SetHorizontalAlignment(HorizontalAlignment);
  BackgroundBlurSlot->SetVerticalAlignment(VerticalAlignment);

  // Add the child to the live slot if it already exists
  if (BlurWidget.IsValid())
  {
    // Construct the underlying slot
    BackgroundBlurSlot->BuildSlot(BlurWidget.ToSharedRef());
  }
}

void UBackgroundBlurMasked::OnSlotRemoved(UPanelSlot * InSlot)
{
  // Remove the widget from the live slot if it exists.
  if (BlurWidget.IsValid())
    BlurWidget->SetContent(SNullWidget::NullWidget);
}

void UBackgroundBlurMasked::SetPadding(const FMargin & InPadding)
{
  Padding = InPadding;
  if (BlurWidget.IsValid())
    BlurWidget->SetPadding(InPadding);
}

const FMargin & UBackgroundBlurMasked::GetPadding() const
{
  return Padding;
}

void UBackgroundBlurMasked::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment)
{
  HorizontalAlignment = InHorizontalAlignment;
  if (BlurWidget.IsValid())
    BlurWidget->SetHAlign(InHorizontalAlignment);
}

EHorizontalAlignment UBackgroundBlurMasked::GetHorizontalAlignment() const
{
  return HorizontalAlignment;
}

void UBackgroundBlurMasked::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment)
{
  VerticalAlignment = InVerticalAlignment;
  if (BlurWidget.IsValid())
    BlurWidget->SetVAlign(InVerticalAlignment);
}

EVerticalAlignment UBackgroundBlurMasked::GetVerticalAlignment() const
{
  return VerticalAlignment;
}

void UBackgroundBlurMasked::SetApplyAlphaToBlur(bool bInApplyAlphaToBlur)
{
  bApplyAlphaToBlur = bInApplyAlphaToBlur;
  if (BlurWidget.IsValid())
    BlurWidget->SetApplyAlphaToBlur(bInApplyAlphaToBlur);
}

bool UBackgroundBlurMasked::GetApplyAlphaToBlur() const
{
  return bApplyAlphaToBlur;
}

void UBackgroundBlurMasked::SetBlurStrength(float InStrength)
{
  BlurStrength = InStrength;
  if (BlurWidget.IsValid())
    BlurWidget->SetBlurStrength(InStrength);
}

float UBackgroundBlurMasked::GetBlurStrength() const
{
  return BlurStrength;
}

void UBackgroundBlurMasked::SetOverrideAutoRadiusCalculation(bool InOverrideAutoRadiusCalculation)
{
  bOverrideAutoRadiusCalculation = InOverrideAutoRadiusCalculation;
  if (BlurWidget.IsValid())
  {
    // When set to false, it needs to reset the optional value and it will use the Blur strength to calculate the blur radius
    BlurWidget->SetBlurRadius(bOverrideAutoRadiusCalculation ? BlurRadius : TOptional<int32>());
  }
}

bool UBackgroundBlurMasked::GetOverrideAutoRadiusCalculation() const
{
  return bOverrideAutoRadiusCalculation;
}

void UBackgroundBlurMasked::SetBlurRadius(int32 InBlurRadius)
{
  BlurRadius = InBlurRadius;
  if (BlurWidget.IsValid())
  {
    BlurWidget->SetBlurRadius(InBlurRadius);
    bOverrideAutoRadiusCalculation = true;
  }
}

int32 UBackgroundBlurMasked::GetBlurRadius() const
{
  return BlurRadius;
}

void UBackgroundBlurMasked::SetCornerRadius(FVector4 InCornerRadius)
{
  CornerRadius = InCornerRadius;
  if (BlurWidget.IsValid())
    BlurWidget->SetCornerRadius(InCornerRadius);
}

FVector4 UBackgroundBlurMasked::GetCornerRadius() const
{
  return CornerRadius;
}

void UBackgroundBlurMasked::SetLowQualityFallbackBrush(const FSlateBrush & InBrush)
{
  LowQualityFallbackBrush = InBrush;
  if (BlurWidget.IsValid())
    BlurWidget->SetLowQualityBackgroundBrush(&LowQualityFallbackBrush);
}

const FSlateBrush & UBackgroundBlurMasked::GetLowQualityFallbackBrush() const
{
  return LowQualityFallbackBrush;
}

void UBackgroundBlurMasked::SetMask(const FSlateBrush & InBrush)
{
  Mask = InBrush;
  if (BlurWidget.IsValid())
    BlurWidget->SetMask(&Mask);
}

const FSlateBrush & UBackgroundBlurMasked::GetMask() const
{
  return Mask;
}

#if WITH_EDITOR

void UBackgroundBlurMasked::PostEditChangeProperty(struct FPropertyChangedEvent & PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);

  static bool IsReentrant = false;

  if (!IsReentrant)
  {
    IsReentrant = true;

    if (PropertyChangedEvent.Property)
    {
      static const FName PaddingName("Padding");
      static const FName HorizontalAlignmentName("HorizontalAlignment");
      static const FName VerticalAlignmentName("VerticalAlignment");

      FName PropertyName = PropertyChangedEvent.Property->GetFName();

      if (auto * BlurSlot = Cast<UBackgroundBlurMaskedSlot>(GetContentSlot()))
      {
        if (PropertyName == PaddingName)
        {
          FObjectEditorUtils::MigratePropertyValue(this, PaddingName, BlurSlot, PaddingName);
        }
        else if (PropertyName == HorizontalAlignmentName)
        {
          FObjectEditorUtils::MigratePropertyValue(this, HorizontalAlignmentName, BlurSlot, HorizontalAlignmentName);
        }
        else if (PropertyName == VerticalAlignmentName)
        {
          FObjectEditorUtils::MigratePropertyValue(this, VerticalAlignmentName, BlurSlot, VerticalAlignmentName);
        }
      }
    }

    IsReentrant = false;
  }
}

const FText UBackgroundBlurMasked::GetPaletteCategory()
{
  return LOCTEXT("SpecialFX", "Special Effects");
}

#endif

#undef LOCTEXT_NAMESPACE
