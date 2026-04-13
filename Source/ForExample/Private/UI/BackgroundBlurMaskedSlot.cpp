#include "UI/BackgroundBlurMaskedSlot.h"
#include "UI/BackgroundBlurMasked.h"
#include "UI/SBackgroundBlurMasked.h"
#include "ObjectEditorUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BackgroundBlurMaskedSlot)


UBackgroundBlurMaskedSlot::UBackgroundBlurMaskedSlot(const FObjectInitializer & ObjectInitializer)
  : Super(ObjectInitializer)
{
  Padding             = FMargin(4.f, 2.f);
  HorizontalAlignment = HAlign_Fill;
  VerticalAlignment   = VAlign_Fill;
}

void UBackgroundBlurMaskedSlot::ReleaseSlateResources(bool bReleaseChildren)
{
  Super::ReleaseSlateResources(bReleaseChildren);

  BackgroundBlur.Reset();
}

void UBackgroundBlurMaskedSlot::BuildSlot(const TSharedRef<SBackgroundBlurMasked> & InBackgroundBlur)
{
  BackgroundBlur = InBackgroundBlur;

  BackgroundBlur->SetPadding(Padding);
  BackgroundBlur->SetHAlign(HorizontalAlignment);
  BackgroundBlur->SetVAlign(VerticalAlignment);

  BackgroundBlur->SetContent(Content ? Content->TakeWidget() : SNullWidget::NullWidget);
}

const FMargin & UBackgroundBlurMaskedSlot::GetPadding() const
{
  return Padding;
}

void UBackgroundBlurMaskedSlot::SetPadding(const FMargin & InPadding)
{
  Padding = InPadding;
  CastChecked<UBackgroundBlurMasked>(Parent)->SetPadding(InPadding);
}

EHorizontalAlignment UBackgroundBlurMaskedSlot::GetHorizontalAlignment() const
{
  return HorizontalAlignment;
}

void UBackgroundBlurMaskedSlot::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment)
{
  HorizontalAlignment = InHorizontalAlignment;
  CastChecked<UBackgroundBlurMasked>(Parent)->SetHorizontalAlignment(InHorizontalAlignment);
}

EVerticalAlignment UBackgroundBlurMaskedSlot::GetVerticalAlignment() const
{
  return VerticalAlignment;
}

void UBackgroundBlurMaskedSlot::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment)
{
  VerticalAlignment = InVerticalAlignment;
  CastChecked<UBackgroundBlurMasked>(Parent)->SetVerticalAlignment(InVerticalAlignment);
}

void UBackgroundBlurMaskedSlot::SynchronizeProperties()
{
  if (BackgroundBlur.IsValid())
  {
    SetPadding(Padding);
    SetHorizontalAlignment(HorizontalAlignment);
    SetVerticalAlignment(VerticalAlignment);
  }
}

#if WITH_EDITOR

void UBackgroundBlurMaskedSlot::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
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

      if (auto * ParentBackgroundBlur = CastChecked<UBackgroundBlurMasked>(Parent))
      {
        if (PropertyName == PaddingName)
        {
          FObjectEditorUtils::MigratePropertyValue(this, PaddingName, ParentBackgroundBlur, PaddingName);
        }
        else if (PropertyName == HorizontalAlignmentName)
        {
          FObjectEditorUtils::MigratePropertyValue(this, HorizontalAlignmentName, ParentBackgroundBlur, HorizontalAlignmentName);
        }
        else if (PropertyName == VerticalAlignmentName)
        {
          FObjectEditorUtils::MigratePropertyValue(this, VerticalAlignmentName, ParentBackgroundBlur, VerticalAlignmentName);
        }
      }
    }

    IsReentrant = false;
  }
}

#endif

