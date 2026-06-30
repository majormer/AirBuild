#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "Configuration/ModConfiguration.h"
#include "AirBuildGameInstanceModule.generated.h"

/**
 * Air Build root Game Instance Module.
 *
 * Exists so the SML Mod Configuration can be registered: the Blueprint child (the actual root module,
 * bRootModule = true) adds the AirBuild_Config Blueprint override to the inherited ModConfigurations array
 * (EditDefaultsOnly). UGameInstanceModule::RegisterDefaultContent registers it with the ConfigManager
 * automatically during the INITIALIZATION lifecycle phase - so there is no runtime registration call here
 * (a manual RegisterModConfiguration would duplicate the entry).
 */
UCLASS(Abstract)
class AIRBUILD_API UAirBuildGameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()

public:
	UAirBuildGameInstanceModule();
};
