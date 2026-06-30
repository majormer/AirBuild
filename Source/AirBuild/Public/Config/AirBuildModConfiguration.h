#pragma once

#include "CoreMinimal.h"
#include "Configuration/ModConfiguration.h"
#include "AirBuildModConfiguration.generated.h"

/**
 * C++ archetype for the Air Build mod configuration.
 *
 * Why this exists: a purely Blueprint/script-authored config whose RootSection
 * tree is created programmatically does NOT survive cooking - the instanced sub-objects get stripped, so the
 * shipped config tree is empty and settings fall back to defaults. Giving the config a C++ class whose
 * constructor builds the section tree as default sub-objects provides the archetype the Blueprint override
 * (/AirBuild/Config/AirBuild_Config) is serialized against, so the cooked asset retains the full tree. The
 * Blueprint override re-skins the same keys with the renderable BP_ConfigProperty* classes for the Mods menu.
 * Keys here MUST match FAirBuild_ConfigStruct_Sections and the Blueprint override.
 */
UCLASS()
class AIRBUILD_API UAirBuildModConfiguration : public UModConfiguration
{
	GENERATED_BODY()

public:
	UAirBuildModConfiguration();

private:
	class UConfigPropertySection* CreateSection(const FName& Name, const FText& DisplayName, const FText& Tooltip);
	class UConfigPropertyBool* CreateBoolProperty(const FName& Name, const FText& DisplayName, const FText& Tooltip, bool Value);
	class UConfigPropertyFloat* CreateFloatProperty(const FName& Name, const FText& DisplayName, const FText& Tooltip, float Value);
};
