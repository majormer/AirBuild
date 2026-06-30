#include "AirBuildSubsystem.h"

#include "AirBuild.h"
#include "AirBuildHologramHook.h"
#include "Config/AirBuild_ConfigStruct.h"
#include "UI/AirBuildIndicatorWidget.h"

#include "Hologram/FGHologram.h"
#include "Input/FGEnhancedInputComponent.h"
#include "Input/FGInputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
bool UAirBuildSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		// Client-only: this is a local placement/preview concern. Never on a dedicated server.
		return World->IsGameWorld() && World->GetNetMode() != NM_DedicatedServer;
	}
	return false;
}

void UAirBuildSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAirBuildSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Install the placement hook EAGERLY (the UpdateHologramPlacement seam is validated). This is load-
	// bearing: the keybind context is scoped to "hologram active", and that freshness signal comes from
	// the hook firing each placement frame. If we waited to install the hook until the first toggle press,
	// the context would never activate (no hook -> no freshness -> context off -> the press never fires) -
	// a deadlock. Installing here breaks it. The hook is a no-op while disengaged.
	FAirBuildHologramHook::EnsureInstalled();

	// Light poll: bind input once the local PlayerController exists, and scope the keybind context
	// to hologram-active. A light timer that binds input once the local PlayerController exists.
	InWorld.GetTimerManager().SetTimer(PollTimer, this, &UAirBuildSubsystem::PollTick, 0.1f, true);
}

void UAirBuildSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollTimer);
	}
	SetAdjustContextActive(false);
	SetMainContextActive(false);
	if (AHUD* HUD = CachedHUD.Get())
	{
		HUD->OnHUDPostRender.RemoveAll(this);
	}
	CachedHUD.Reset();
	if (Widget)
	{
		if (Widget->IsInViewport())
		{
			Widget->RemoveFromParent();
		}
		Widget = nullptr;
	}
	Super::Deinitialize();
}

UAirBuildSubsystem* UAirBuildSubsystem::Get(const UObject* WorldContext)
{
	if (!GEngine)
	{
		return nullptr;
	}
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull))
	{
		return World->GetSubsystem<UAirBuildSubsystem>();
	}
	return nullptr;
}

UEnhancedInputLocalPlayerSubsystem* UAirBuildSubsystem::GetEILP() const
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			if (ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			}
		}
	}
	return nullptr;
}

// ---------------------------------------------------------------------------
// Poll: bind input + scope context to hologram-active
// ---------------------------------------------------------------------------
void UAirBuildSubsystem::PollTick()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!bInputBound)
	{
		TryBindInput();
	}

	EnsureHudBinding();

	// Hologram-active scoping: the hook stamps LastHologramSeenSeconds every frame a non-excluded
	// hologram is up. While fresh -> the keybind context is active; once stale (left build) -> remove it
	// and reset engaged state so '-'/'=' never act outside building.
	const double Now = World->GetTimeSeconds();
	const bool bHoloActive = (Now - LastHologramSeenSeconds) < 0.25;
	if (bHoloActive != bMainContextActive)
	{
		SetMainContextActive(bHoloActive);
		if (bHoloActive)
		{
			// Entering a build session: refresh config + reset reach to the configured default, and clear the
			// manual override so the session starts at the mode's default behavior.
			ReadConfig();
			ReachCm = FMath::Clamp(DefaultReachCm, MinReachCm, MaxReachCm);
			bManualToggle = false;
		}
		else
		{
			// Left the build gun: stop floating and clear per-session state.
			bFloatingNow = false;
			bManualToggle = false;
			if (bAdjustHeld)
			{
				bAdjustHeld = false;
				SetAdjustContextActive(false);
			}
		}
	}
}

void UAirBuildSubsystem::TryBindInput()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC || !PC->GetLocalPlayer())
	{
		return; // local player not ready yet
	}

	auto LoadIA = [](const TCHAR* Path) -> UInputAction*
	{
		FSoftObjectPath S(Path);
		UObject* O = S.TryLoad();
		if (!O) { O = LoadObject<UInputAction>(nullptr, Path); }
		return Cast<UInputAction>(O);
	};
	auto LoadCtx = [](const TCHAR* Path) -> UFGInputMappingContext*
	{
		FSoftObjectPath S(Path);
		UObject* O = S.TryLoad();
		if (!O) { O = LoadObject<UFGInputMappingContext>(nullptr, Path); }
		return Cast<UFGInputMappingContext>(O);
	};

	if (!IA_Toggle)     IA_Toggle     = LoadIA(TEXT("/AirBuild/Input/Actions/IA_AirBuild_Toggle.IA_AirBuild_Toggle"));
	if (!IA_AdjustHold) IA_AdjustHold = LoadIA(TEXT("/AirBuild/Input/Actions/IA_AirBuild_AdjustHold.IA_AirBuild_AdjustHold"));
	if (!IA_Wheel)      IA_Wheel      = LoadIA(TEXT("/AirBuild/Input/Actions/IA_AirBuild_MouseWheel.IA_AirBuild_MouseWheel"));
	if (!MainContext)   MainContext   = LoadCtx(TEXT("/AirBuild/Input/Contexts/MC_AirBuild_BuildGunBuild.MC_AirBuild_BuildGunBuild"));
	if (!AdjustContext) AdjustContext = LoadCtx(TEXT("/AirBuild/Input/Contexts/MC_AirBuild_Adjust.MC_AirBuild_Adjust"));

	if (!IA_Toggle || !IA_AdjustHold || !IA_Wheel || !MainContext || !AdjustContext)
	{
		return; // input assets not present yet (created in-editor); keep polling
	}

	UFGEnhancedInputComponent* EIC = Cast<UFGEnhancedInputComponent>(PC->InputComponent);
	if (!EIC)
	{
		return;
	}

	PC->EnableInput(PC);
	EIC->BindAction(IA_Toggle,     ETriggerEvent::Started,   this, &UAirBuildSubsystem::OnToggleAirPlace);
	EIC->BindAction(IA_AdjustHold, ETriggerEvent::Started,   this, &UAirBuildSubsystem::OnAdjustHoldStarted);
	EIC->BindAction(IA_AdjustHold, ETriggerEvent::Completed, this, &UAirBuildSubsystem::OnAdjustHoldCompleted);
	EIC->BindAction(IA_Wheel,      ETriggerEvent::Triggered, this, &UAirBuildSubsystem::OnReachWheel);

	ReadConfig();
	bInputBound = true;
	UE_LOG(LogAirBuild, Display, TEXT("AirBuild: Enhanced Input bound (toggle/adjust/wheel)."));
}

void UAirBuildSubsystem::ReadConfig()
{
	const FAirBuild_ConfigStruct C = FAirBuild_ConfigStruct::GetActiveConfig(this);
	// These sliders drag continuously (SML's slider widget doesn't expose a per-property step), so snap to a
	// clean grid here rather than carrying odd drag-precision values into gameplay: 0.5 m for the reach
	// bounds (so they land on whole numbers), 0.1 m for the step size.
	const float QuantizedDefaultReach = FMath::RoundToFloat(C.DefaultReach * 2.f) / 2.f;
	const float QuantizedMinReach     = FMath::RoundToFloat(C.MinReach * 2.f) / 2.f;
	const float QuantizedMaxReach     = FMath::RoundToFloat(C.MaxReach * 2.f) / 2.f;
	const float QuantizedReachStep    = FMath::RoundToFloat(C.ReachStep * 10.f) / 10.f;
	DefaultReachCm = QuantizedDefaultReach * 100.f;
	MinReachCm     = QuantizedMinReach * 100.f;
	MaxReachCm     = QuantizedMaxReach * 100.f;
	ReachStepCm    = FMath::Max(1.f, QuantizedReachStep * 100.f);
	if (MinReachCm > MaxReachCm)
	{
		Swap(MinReachCm, MaxReachCm); // defend against a user setting Min > Max in the menu
	}
	AirPlaceMode   = C.AirPlaceMode;
	bShowIndicator = C.bShowIndicator;
	// The position sliders drag continuously (SML's slider widget doesn't expose a per-property step), so
	// snap to a clean 0.01 grid here rather than carrying odd drag-precision values into the HUD.
	IndicatorPosX  = FMath::RoundToFloat(C.IndicatorPosX * 100.f) / 100.f;
	IndicatorPosY  = FMath::RoundToFloat(C.IndicatorPosY * 100.f) / 100.f;
	IndicatorScale = C.IndicatorScale;
	ReachCm = FMath::Clamp(ReachCm, MinReachCm, MaxReachCm);

	UE_LOG(LogAirBuild, Verbose, TEXT("[AirBuild] ReadConfig: Mode=%d DefaultReach=%.1fm Min=%.1f Max=%.1f Step=%.1f"), C.AirPlaceMode, C.DefaultReach, C.MinReach, C.MaxReach, C.ReachStep);
}

// Per-frame decision the hook applies. bSurfaceWithinReach = vanilla found a snap surface no farther than the
// current reach. Modes: 0 Off (float only while toggled on), 1 Always (float unless toggled off),
// 2 Smart (float only when there's no surface in reach, or when the toggle forces it).
bool UAirBuildSubsystem::ResolveShouldFloat(bool bSurfaceWithinReach) const
{
	switch (AirPlaceMode)
	{
	case 1:  return !bManualToggle;
	case 2:  return bManualToggle || !bSurfaceWithinReach;
	default: return bManualToggle;
	}
}

// ---------------------------------------------------------------------------
// Context activation
// ---------------------------------------------------------------------------
void UAirBuildSubsystem::SetMainContextActive(bool bActive)
{
	if (bActive == bMainContextActive || !MainContext)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* EI = GetEILP())
	{
		if (bActive) { EI->AddMappingContext(MainContext, 100); }
		else         { EI->RemoveMappingContext(MainContext); }
		bMainContextActive = bActive;
	}
}

void UAirBuildSubsystem::SetAdjustContextActive(bool bActive)
{
	if (bActive == bAdjustContextActive || !AdjustContext)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* EI = GetEILP())
	{
		// Higher than the main context (and vanilla) so the wheel is consumed for reach ONLY during the
		// '=' hold window; outside the window this context isn't present, so plain-wheel rotation is vanilla.
		if (bActive) { EI->AddMappingContext(AdjustContext, 110); }
		else         { EI->RemoveMappingContext(AdjustContext); }
		bAdjustContextActive = bActive;
	}
}

// ---------------------------------------------------------------------------
// Input handlers
// ---------------------------------------------------------------------------
void UAirBuildSubsystem::OnToggleAirPlace(const FInputActionValue& /*Value*/)
{
	FAirBuildHologramHook::EnsureInstalled(); // install the placement hook on first real engage
	// One override key across all modes: flip the per-session manual toggle. The hook's ResolveShouldFloat
	// interprets it per mode (Off: turn float on; Always: suspend it; Smart: force float over the gap-fill).
	bManualToggle = !bManualToggle;
	UE_LOG(LogAirBuild, Display, TEXT("AirBuild: toggle pressed (mode %d, override %s)"),
		AirPlaceMode, bManualToggle ? TEXT("on") : TEXT("off"));
}

void UAirBuildSubsystem::OnAdjustHoldStarted(const FInputActionValue& /*Value*/)
{
	bAdjustHeld = true;
	SetAdjustContextActive(true);
}

void UAirBuildSubsystem::OnAdjustHoldCompleted(const FInputActionValue& /*Value*/)
{
	bAdjustHeld = false;
	SetAdjustContextActive(false);
}

void UAirBuildSubsystem::OnReachWheel(const FInputActionValue& Value)
{
	if (!bAdjustHeld)
	{
		return; // safety: vanilla rotation path
	}
	const float D = Value.Get<float>();
	if (D == 0.f)
	{
		return;
	}
	const float Dir = (D > 0.f) ? 1.f : -1.f;
	const int32 Notches = FMath::Max(1, FMath::RoundToInt(FMath::Abs(D))); // accumulate for fast wheels
	ReachCm = FMath::Clamp(ReachCm + Dir * ReachStepCm * Notches, MinReachCm, MaxReachCm);
}

// ---------------------------------------------------------------------------
// Hook -> subsystem writes
// ---------------------------------------------------------------------------
void UAirBuildSubsystem::NotifyHologramActive(AFGHologram* Holo)
{
	ActiveHologram = Holo;
	if (UWorld* World = GetWorld())
	{
		LastHologramSeenSeconds = World->GetTimeSeconds();
	}
}

void UAirBuildSubsystem::SetHudComputed(AFGHologram* Holo, const FVector& HoloLoc, float DistPlayerCm, float DistGroundCm)
{
	ActiveHologram = Holo;
	HologramLocation = HoloLoc;
	DistFromPlayerCm = DistPlayerCm;
	DistFromGroundCm = DistGroundCm;
	if (UWorld* World = GetWorld())
	{
		LastHologramSeenSeconds = World->GetTimeSeconds();
	}
}

// ---------------------------------------------------------------------------
// HUD indicator
// ---------------------------------------------------------------------------
void UAirBuildSubsystem::EnsureHudBinding()
{
	if (CachedHUD.IsValid())
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (AHUD* HUD = PC->GetHUD())
		{
			CachedHUD = HUD;
			HUD->OnHUDPostRender.AddUObject(this, &UAirBuildSubsystem::DrawIndicator);
		}
	}
}

void UAirBuildSubsystem::DrawIndicator(AHUD* /*HUD*/, UCanvas* /*Canvas*/)
{
	const bool bShouldShow = bShowIndicator && bFloatingNow && ActiveHologram.IsValid();

	if (!Widget)
	{
		UWorld* World = GetWorld();
		if (!World) { return; }
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			Widget = CreateWidget<UAirBuildIndicatorWidget>(PC);
			if (Widget)
			{
				Widget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
	if (!Widget)
	{
		return;
	}

	if (!bShouldShow)
	{
		if (Widget->IsInViewport())
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(0);
	}
	Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Widget->SetIndicatorScale(IndicatorScale);

	const float PX = FMath::Clamp(IndicatorPosX, 0.f, 1.f);
	const float PY = FMath::Clamp(IndicatorPosY, 0.f, 1.f);
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	Widget->SetPositionInViewport(FVector2D(ViewportSize.X * PX, ViewportSize.Y * PY));

	const float ReachM = ReachCm / 100.f;
	const float DistPlayerM = DistFromPlayerCm / 100.f;
	const float DistGroundM = (DistFromGroundCm < 0.f) ? -1.f : DistFromGroundCm / 100.f;
	const float WorldHeightM = HologramLocation.Z / 100.f;
	Widget->UpdateContent(DistPlayerM, WorldHeightM, DistGroundM, ReachM);
}

AFGHologram* UAirBuildSubsystem::GetLocalActiveBuildHologram() const
{
	// Not used in the freshness-driven design; retained for potential direct polling.
	return ActiveHologram.Get();
}
