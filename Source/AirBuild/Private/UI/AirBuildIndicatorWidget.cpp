#include "UI/AirBuildIndicatorWidget.h"

#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Font.h"
#include "Layout/Margin.h"

void UAirBuildIndicatorWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
}

FSlateFontInfo UAirBuildIndicatorWidget::MakeFont(int32 Size) const
{
	// FactoryGame's runtime DescriptionText (multi-script), falling back to Roboto.
	static TWeakObjectPtr<UFont> CachedFont;
	if (!CachedFont.IsValid())
	{
		UFont* F = LoadObject<UFont>(nullptr, TEXT("/Game/FactoryGame/Interface/Font/DescriptionText.DescriptionText"));
		if (!F)
		{
			F = LoadObject<UFont>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto"));
		}
		CachedFont = F;
	}

	FSlateFontInfo Info;
	if (CachedFont.IsValid())
	{
		Info = FSlateFontInfo(CachedFont.Get(), Size);
	}
	else
	{
		Info.Size = Size;
	}
	return Info;
}

UTextBlock* UAirBuildIndicatorWidget::CreateLine()
{
	UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	T->SetFont(MakeFont(FMath::RoundToInt(14.f * Scale)));
	T->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	T->SetShadowOffset(FVector2D(1.f, 1.f));
	T->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.9f));
	return T;
}

void UAirBuildIndicatorWidget::BuildWidgetTree()
{
	if (OuterBorder)
	{
		return;
	}

	OuterBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	OuterBorder->SetBrushColor(FLinearColor(1.f, 0.55f, 0.13f, 1.f)); // FICSIT orange edge
	OuterBorder->SetPadding(FMargin(2.f));

	InnerBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	InnerBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.85f));
	InnerBorder->SetPadding(FMargin(10.f, 6.f));

	Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

	PlayerLine = CreateLine();
	HeightLine = CreateLine();
	GroundLine = CreateLine();
	ReachLine  = CreateLine();

	for (UTextBlock* Line : { PlayerLine.Get(), HeightLine.Get(), GroundLine.Get(), ReachLine.Get() })
	{
		if (UVerticalBoxSlot* VBSlot = Box->AddChildToVerticalBox(Line))
		{
			VBSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));
		}
	}

	InnerBorder->SetContent(Box);
	OuterBorder->SetContent(InnerBorder);
	WidgetTree->RootWidget = OuterBorder;
}

void UAirBuildIndicatorWidget::SetIndicatorScale(float InScale)
{
	const float Clamped = FMath::Clamp(InScale, 0.5f, 3.f);
	if (FMath::IsNearlyEqual(Clamped, Scale))
	{
		return;
	}
	Scale = Clamped;
	const int32 S = FMath::RoundToInt(14.f * Scale);
	if (PlayerLine) PlayerLine->SetFont(MakeFont(S));
	if (HeightLine) HeightLine->SetFont(MakeFont(S));
	if (GroundLine) GroundLine->SetFont(MakeFont(S));
	if (ReachLine)  ReachLine->SetFont(MakeFont(S));
}

void UAirBuildIndicatorWidget::UpdateContent(float DistFromPlayerM, float WorldHeightM, float DistFromGroundM, float ReachM)
{
	if (PlayerLine)
	{
		PlayerLine->SetText(FText::FromString(FString::Printf(TEXT("Distance: %.1f m"), DistFromPlayerM)));
	}
	if (HeightLine)
	{
		HeightLine->SetText(FText::FromString(FString::Printf(TEXT("Height: %.1f m"), WorldHeightM)));
	}
	if (GroundLine)
	{
		GroundLine->SetText(FText::FromString(
			DistFromGroundM < 0.f ? FString(TEXT("Ground: --")) : FString::Printf(TEXT("Ground: %.1f m"), DistFromGroundM)));
	}
	if (ReachLine)
	{
		ReachLine->SetText(FText::FromString(FString::Printf(TEXT("Reach: %.1f m"), ReachM)));
	}
}
