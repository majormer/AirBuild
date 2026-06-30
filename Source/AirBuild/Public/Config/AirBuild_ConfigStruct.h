#pragma once

#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "AirBuild_ConfigStruct.generated.h"

/*
 * Air Build mod configuration. Uses SML's nested-sections pattern: a named config section maps to a
 * NESTED struct field, so the in-game Mod Settings menu layout (Air Placement / Indicator) is mirrored by
 * the sub-structs below and FAirBuild_ConfigStruct_Sections. The public flat FAirBuild_ConfigStruct is what
 * runtime code consumes; GetActiveConfig() fills the nested sections from the live SML config system and
 * copies the values down. Reach values are authored in METERS in the menu and converted to cm by callers.
 */

// -- Section mirror sub-structs (field names MUST match the config asset's leaf keys) --

USTRUCT(BlueprintType)
struct FAirBuild_AirPlacementSection
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) int32 AirPlaceMode{0};     // 0 = Off (manual), 1 = Always, 2 = Smart (gap-fill)
	UPROPERTY(BlueprintReadWrite) float DefaultReach{15.f};  // meters
	UPROPERTY(BlueprintReadWrite) float MinReach{2.f};       // meters
	UPROPERTY(BlueprintReadWrite) float MaxReach{200.f};     // meters
	UPROPERTY(BlueprintReadWrite) float ReachStep{0.5f};     // meters per wheel notch
};

USTRUCT(BlueprintType)
struct FAirBuild_IndicatorSection
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) bool bShowIndicator{true};
	UPROPERTY(BlueprintReadWrite) float IndicatorPosX{0.5f};  // 0 = left, 1 = right
	UPROPERTY(BlueprintReadWrite) float IndicatorPosY{0.15f}; // 0 = top, 1 = bottom (up, clear of the centred hologram)
	UPROPERTY(BlueprintReadWrite) float IndicatorScale{1.f};
};

USTRUCT(BlueprintType)
struct FAirBuild_ConfigStruct_Sections
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) FAirBuild_AirPlacementSection AirPlacement;
	UPROPERTY(BlueprintReadWrite) FAirBuild_IndicatorSection Indicator;
};

// -- Flat struct consumed by runtime code --

USTRUCT(BlueprintType)
struct FAirBuild_ConfigStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) int32 AirPlaceMode{0};
	UPROPERTY(BlueprintReadWrite) float DefaultReach{15.f};
	UPROPERTY(BlueprintReadWrite) float MinReach{2.f};
	UPROPERTY(BlueprintReadWrite) float MaxReach{200.f};
	UPROPERTY(BlueprintReadWrite) float ReachStep{0.5f};

	UPROPERTY(BlueprintReadWrite) bool bShowIndicator{true};
	UPROPERTY(BlueprintReadWrite) float IndicatorPosX{0.5f};
	UPROPERTY(BlueprintReadWrite) float IndicatorPosY{0.15f};
	UPROPERTY(BlueprintReadWrite) float IndicatorScale{1.f};

	/* Reads the live SML config and returns the flat struct. */
	static FAirBuild_ConfigStruct GetActiveConfig(UObject* WorldContext)
	{
		FAirBuild_ConfigStruct Config{};
		FAirBuild_ConfigStruct_Sections Sections{};
		FConfigId ConfigId{"AirBuild", ""};
		if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull))
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UConfigManager* ConfigManager = GI->GetSubsystem<UConfigManager>())
				{
					ConfigManager->FillConfigurationStruct(
						ConfigId,
						FDynamicStructInfo{FAirBuild_ConfigStruct_Sections::StaticStruct(), &Sections});
				}
			}
		}

		Config.AirPlaceMode    = Sections.AirPlacement.AirPlaceMode;
		Config.DefaultReach    = Sections.AirPlacement.DefaultReach;
		Config.MinReach        = Sections.AirPlacement.MinReach;
		Config.MaxReach        = Sections.AirPlacement.MaxReach;
		Config.ReachStep       = Sections.AirPlacement.ReachStep;

		Config.bShowIndicator  = Sections.Indicator.bShowIndicator;
		Config.IndicatorPosX   = Sections.Indicator.IndicatorPosX;
		Config.IndicatorPosY   = Sections.Indicator.IndicatorPosY;
		Config.IndicatorScale  = Sections.Indicator.IndicatorScale;

		return Config;
	}
};
