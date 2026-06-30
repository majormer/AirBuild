#include "AirBuild.h"

#include "AirBuildHologramHook.h"

IMPLEMENT_MODULE(FAirBuildModule, AirBuild)

DEFINE_LOG_CATEGORY(LogAirBuild);

void FAirBuildModule::StartupModule()
{
	UE_LOG(LogAirBuild, Display, TEXT("AirBuild module starting up"));

	// IMPORTANT: do NOT install the native hook at module startup. An unhookable
	// target (funchook "Too short instructions") fatal-crashes the editor on load,
	// and a fatal check can't be caught. During development the hook is installed
	// on demand via the `airbuild.InstallHook` console command, so a bad seam only
	// crashes when explicitly tested, never on boot. Register only sets up that
	// console command here.
	FAirBuildHologramHook::Register();
}

void FAirBuildModule::ShutdownModule()
{
	FAirBuildHologramHook::Unregister();
}
