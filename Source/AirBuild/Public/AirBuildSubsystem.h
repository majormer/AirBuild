#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InputActionValue.h"
#include "AirBuildSubsystem.generated.h"

class AFGHologram;
class UInputAction;
class UFGInputMappingContext;
class UAirBuildIndicatorWidget;
class AHUD;
class UCanvas;

/**
 * Client-only per-world owner of Air Build's runtime state. Single source of truth shared by:
 *  - the Enhanced Input handlers (writers of bManualToggle / ReachCm / bAdjustHeld),
 *  - the hologram hook (reads the mode + reach, writes bFloatingNow + the HUD-computed distances),
 *  - the HUD indicator draw (reads everything).
 * A UWorldSubsystem input/state hub. Never replicated; the state is
 * a purely local placement/preview concern, so the subsystem is not created on a dedicated server.
 */
UCLASS()
class AIRBUILD_API UAirBuildSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// UWorldSubsystem
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** Resolve the subsystem from any world-context object (e.g. the active hologram). */
	static UAirBuildSubsystem* Get(const UObject* WorldContext);

	bool IsFloatingNow() const { return bFloatingNow; }
	float GetReachCm() const { return ReachCm; }
	bool IsAdjustHeld() const { return bAdjustHeld; }

	/** Per-frame float decision applied by the hook: mode + manual override + whether a surface is within reach. */
	bool ResolveShouldFloat(bool bSurfaceWithinReach) const;
	void SetFloatingNow(bool bFloating) { bFloatingNow = bFloating; }

	/** Called by the hook every frame a NON-EXCLUDED hologram is active (freshness signal that scopes the keybind context). */
	void NotifyHologramActive(AFGHologram* Holo);

	/** Written by the hologram hook each placement frame while engaged (drives the HUD readout). */
	void SetHudComputed(AFGHologram* Holo, const FVector& HoloLoc, float DistPlayerCm, float DistGroundCm);

	// Enhanced Input handlers (bound on the local PlayerController's input component).
	// UFUNCTION for Enhanced Input BindAction targets.
	UFUNCTION() void OnToggleAirPlace(const FInputActionValue& Value);
	UFUNCTION() void OnAdjustHoldStarted(const FInputActionValue& Value);
	UFUNCTION() void OnAdjustHoldCompleted(const FInputActionValue& Value);
	UFUNCTION() void OnReachWheel(const FInputActionValue& Value);

private:
	// --- live state ---
	bool bManualToggle = false;  // the toggle-key override; reset to the mode default on build entry, flipped by the key
	bool bFloatingNow = false;   // per-frame: did the hook air-place this frame (drives the HUD + uneven-floor exception)
	bool bAdjustHeld = false;
	float ReachCm = 1500.f;

	// --- config cache (converted to cm) ---
	float DefaultReachCm = 1500.f;
	float MinReachCm = 200.f;
	float MaxReachCm = 8000.f;
	float ReachStepCm = 200.f;
	int32 AirPlaceMode = 0;            // 0 = Off (manual toggle), 1 = Always, 2 = Smart (gap-fill)
	bool bShowIndicator = true;
	float IndicatorPosX = 0.5f;
	float IndicatorPosY = 0.45f;
	float IndicatorScale = 1.f;

	// --- HUD-computed (written by the hook) ---
	TWeakObjectPtr<AFGHologram> ActiveHologram;
	FVector HologramLocation = FVector::ZeroVector;
	float DistFromPlayerCm = 0.f;
	float DistFromGroundCm = -1.f; // < 0 => no ground within range ("-")
	double LastHologramSeenSeconds = -1000.0; // freshness; main context active while a hologram was seen recently

	// --- input + context ---
	bool bInputBound = false;
	bool bMainContextActive = false;
	bool bAdjustContextActive = false;
	UPROPERTY() TObjectPtr<UInputAction> IA_Toggle = nullptr;
	UPROPERTY() TObjectPtr<UInputAction> IA_AdjustHold = nullptr;
	UPROPERTY() TObjectPtr<UInputAction> IA_Wheel = nullptr;
	UPROPERTY() TObjectPtr<UFGInputMappingContext> MainContext = nullptr;   // toggle + adjust-hold (active while a hologram is up)
	UPROPERTY() TObjectPtr<UFGInputMappingContext> AdjustContext = nullptr; // wheel-only (active only during the '=' hold window)

	// --- HUD ---
	UPROPERTY() TObjectPtr<UAirBuildIndicatorWidget> Widget = nullptr;
	TWeakObjectPtr<AHUD> CachedHUD;

	FTimerHandle PollTimer;

	void PollTick();                 // binds input once PC exists; scopes the main context to hologram-active; resets on leave
	void ReadConfig();
	void TryBindInput();             // load assets + BindAction once
	void SetMainContextActive(bool bActive);
	void SetAdjustContextActive(bool bActive);
	void EnsureHudBinding();
	void DrawIndicator(AHUD* HUD, UCanvas* Canvas);
	AFGHologram* GetLocalActiveBuildHologram() const;
	class UEnhancedInputLocalPlayerSubsystem* GetEILP() const;
};
