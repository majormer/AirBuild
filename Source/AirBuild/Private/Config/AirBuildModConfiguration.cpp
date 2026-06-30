#include "Config/AirBuildModConfiguration.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Configuration/Properties/ConfigPropertyBool.h"
#include "Configuration/Properties/ConfigPropertyInteger.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"

#define LOCTEXT_NAMESPACE "AirBuild"

UAirBuildModConfiguration::UAirBuildModConfiguration()
{
	ConfigId.ModReference = TEXT("AirBuild");
	DisplayName = LOCTEXT("Config.DisplayName", "Air Build");
	Description = LOCTEXT("Config.Description", "Air Build settings.");

	RootSection = CreateSection(TEXT("Root"),
		LOCTEXT("Config.Root.Name", "Air Build"),
		LOCTEXT("Config.Root.Tooltip", "Air Build settings."));

	// -- Air Placement (floats in METERS) --
	UConfigPropertySection* Air = CreateSection(TEXT("AirPlacement"), LOCTEXT("Sec.Air", "Air Placement"),
		LOCTEXT("Sec.Air.TT", "How buildings float in the air while air-place mode is engaged."));
	Air->SectionProperties.Add(TEXT("AirPlaceMode"), CreateIntegerProperty(TEXT("AirPlaceMode"), LOCTEXT("P.AirPlaceMode", "Auto Air Placement"),
		LOCTEXT("P.AirPlaceMode.TT", "How air placement engages. 0 = Off: use the toggle key to turn it on. 1 = Always: on whenever you're building. 2 = Smart: turns on only when there's nothing within reach to snap to - aim at open sky and the building floats, aim at a surface and it snaps as normal. The toggle key still overrides per build."), 0));
	Air->SectionProperties.Add(TEXT("DefaultReach"), CreateFloatProperty(TEXT("DefaultReach"), LOCTEXT("P.DefaultReach", "Default Reach (m)"),
		LOCTEXT("P.DefaultReach.TT", "Distance in meters the building floats out when you engage air-place mode."), 15.f));
	Air->SectionProperties.Add(TEXT("MinReach"), CreateFloatProperty(TEXT("MinReach"), LOCTEXT("P.MinReach", "Minimum Reach (m)"),
		LOCTEXT("P.MinReach.TT", "Closest the building can be pulled in."), 2.f));
	Air->SectionProperties.Add(TEXT("MaxReach"), CreateFloatProperty(TEXT("MaxReach"), LOCTEXT("P.MaxReach", "Maximum Reach (m)"),
		LOCTEXT("P.MaxReach.TT", "Furthest the building can be pushed out."), 200.f));
	Air->SectionProperties.Add(TEXT("ReachStep"), CreateFloatProperty(TEXT("ReachStep"), LOCTEXT("P.ReachStep", "Reach Step (m)"),
		LOCTEXT("P.ReachStep.TT", "How far each mouse-wheel notch moves the building while the adjust key is held."), 0.5f));
	RootSection->SectionProperties.Add(TEXT("AirPlacement"), Air);

	// -- Indicator --
	UConfigPropertySection* Ind = CreateSection(TEXT("Indicator"), LOCTEXT("Sec.Ind", "Indicator"),
		LOCTEXT("Sec.Ind.TT", "The on-screen readout shown while air-place mode is engaged."));
	Ind->SectionProperties.Add(TEXT("bShowIndicator"), CreateBoolProperty(TEXT("bShowIndicator"), LOCTEXT("P.bShowIndicator", "Show Indicator"),
		LOCTEXT("P.bShowIndicator.TT", "Show the on-screen distance/reach readout while air-placing."), true));
	Ind->SectionProperties.Add(TEXT("IndicatorPosX"), CreateFloatProperty(TEXT("IndicatorPosX"), LOCTEXT("P.IndicatorPosX", "Indicator Position X"),
		LOCTEXT("P.IndicatorPosX.TT", "Horizontal position of the indicator (0 = left edge, 1 = right edge)."), 0.5f));
	Ind->SectionProperties.Add(TEXT("IndicatorPosY"), CreateFloatProperty(TEXT("IndicatorPosY"), LOCTEXT("P.IndicatorPosY", "Indicator Position Y"),
		LOCTEXT("P.IndicatorPosY.TT", "Vertical position of the indicator (0 = top, 1 = bottom)."), 0.15f));
	Ind->SectionProperties.Add(TEXT("IndicatorScale"), CreateFloatProperty(TEXT("IndicatorScale"), LOCTEXT("P.IndicatorScale", "Indicator Scale"),
		LOCTEXT("P.IndicatorScale.TT", "Size of the on-screen indicator."), 1.f));
	RootSection->SectionProperties.Add(TEXT("Indicator"), Ind);
}

UConfigPropertySection* UAirBuildModConfiguration::CreateSection(const FName& Name, const FText& InDisplayName, const FText& Tooltip)
{
	UConfigPropertySection* Property = CreateDefaultSubobject<UConfigPropertySection>(Name);
	Property->DisplayName = InDisplayName;
	Property->Tooltip = Tooltip;
	Property->bRequiresWorldReload = false;
	Property->bHidden = false;
	return Property;
}

UConfigPropertyBool* UAirBuildModConfiguration::CreateBoolProperty(const FName& Name, const FText& InDisplayName, const FText& Tooltip, bool Value)
{
	UConfigPropertyBool* Property = CreateDefaultSubobject<UConfigPropertyBool>(Name);
	Property->DisplayName = InDisplayName;
	Property->Tooltip = Tooltip;
	Property->Value = Value;
	Property->bRequiresWorldReload = false;
	Property->bHidden = false;
	return Property;
}

UConfigPropertyInteger* UAirBuildModConfiguration::CreateIntegerProperty(const FName& Name, const FText& InDisplayName, const FText& Tooltip, int32 Value)
{
	UConfigPropertyInteger* Property = CreateDefaultSubobject<UConfigPropertyInteger>(Name);
	Property->DisplayName = InDisplayName;
	Property->Tooltip = Tooltip;
	Property->Value = Value;
	Property->DefaultValue = Value;
	Property->bRequiresWorldReload = false;
	Property->bHidden = false;
	return Property;
}

UConfigPropertyFloat* UAirBuildModConfiguration::CreateFloatProperty(const FName& Name, const FText& InDisplayName, const FText& Tooltip, float Value)
{
	UConfigPropertyFloat* Property = CreateDefaultSubobject<UConfigPropertyFloat>(Name);
	Property->DisplayName = InDisplayName;
	Property->Tooltip = Tooltip;
	Property->Value = Value;
	Property->bRequiresWorldReload = false;
	Property->bHidden = false;
	return Property;
}

#undef LOCTEXT_NAMESPACE
