#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AirBuildIndicatorWidget.generated.h"

class UBorder;
class UVerticalBox;
class UTextBlock;
class UFont;

/**
 * Native air-place indicator. Builds its own Slate tree programmatically -
 * a small backdrop: distance from player, world height, height above ground, and current reach.
 * No Blueprint asset, no external UI dependency.
 */
UCLASS()
class AIRBUILD_API UAirBuildIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetIndicatorScale(float InScale);
	/** Ground < 0 => show a dash (no ground within range). Values in meters. */
	void UpdateContent(float DistFromPlayerM, float WorldHeightM, float DistFromGroundM, float ReachM);

protected:
	virtual void NativeOnInitialized() override;

private:
	void BuildWidgetTree();
	UTextBlock* CreateLine();
	FSlateFontInfo MakeFont(int32 Size) const;

	UPROPERTY() TObjectPtr<UBorder> OuterBorder = nullptr;
	UPROPERTY() TObjectPtr<UBorder> InnerBorder = nullptr;
	UPROPERTY() TObjectPtr<UVerticalBox> Box = nullptr;
	UPROPERTY() TObjectPtr<UTextBlock> PlayerLine = nullptr;
	UPROPERTY() TObjectPtr<UTextBlock> HeightLine = nullptr;
	UPROPERTY() TObjectPtr<UTextBlock> GroundLine = nullptr;
	UPROPERTY() TObjectPtr<UTextBlock> ReachLine = nullptr;

	float Scale = 1.f;
};
