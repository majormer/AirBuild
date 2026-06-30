#include "AirBuildHologramHook.h"

#include "AirBuild.h"
#include "AirBuildSubsystem.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Patching/NativeHookManager.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"

#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGHologram.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGResourceExtractorHologram.h"
#include "FGConstructDisqualifier.h"

// ---------------------------------------------------------------------------
// Dev-driven state (temporary CVar driver; real Enhanced Input replaces it).
//   airbuild.Enabled 1        -> engage air-place mode
//   airbuild.Reach   1500     -> distance (cm) from the eye along the aim ray
// ---------------------------------------------------------------------------
static TAutoConsoleVariable<int32> CVarAirBuildEnabled(
	TEXT("airbuild.Enabled"), 0,
	TEXT("Engage Air Build placement mode (0 = off, 1 = on)."), ECVF_Default);

static TAutoConsoleVariable<float> CVarAirBuildReach(
	TEXT("airbuild.Reach"), 1500.0f,
	TEXT("Air Build reach: distance in cm from the camera along the aim ray."), ECVF_Default);

// Wide cm safety net. Must never be tighter than the configurable Min/Max reach, or it would clip
// user settings; the subsystem clamps to the config Min/Max, this is just a sane absolute backstop.
static constexpr float SAB_MinReach = 50.0f;      // 0.5 m
static constexpr float SAB_MaxReach = 50000.0f;   // 500 m absolute backstop (must exceed the config MaxReach)
static constexpr float SAB_GroundTraceMaxCm = 100000.0f; // 1 km down-trace for the ground readout

static bool GAirBuildHookInstalled = false;

// Shared placement rewrite: pushes the hit to a point floating along the SAME aim ray at the chosen
// reach, clearing the surface hit so it floats instead of snapping. Run BEFORE the original
// UpdateHologramPlacement, so the hologram's own IsValidHitResult sees the synthetic (valid) hit -- this
// is what lets you place into open sky where the raw trace would have missed. Vanilla validity
// downstream is otherwise untouched. Reads engaged/reach from the runtime subsystem (the CVars are a
// debug override). Also computes the HUD distances while it has the hologram. Returns true if it acted.
static bool RewriteHitForAirPlace(AFGHologram* Holo, FHitResult& Hit)
{
	if (!Holo)
	{
		return false;
	}
	if (FAirBuildHologramHook::IsExcludedHologram(Holo))
	{
		return false; // resource-node family -> vanilla snapping
	}

	// Resolve engaged + reach: CVar debug override first, else the runtime subsystem state.
	bool bEngaged = false;
	float ReachCm = 1500.0f;
	UAirBuildSubsystem* State = UAirBuildSubsystem::Get(Holo);
	if (CVarAirBuildEnabled.GetValueOnAnyThread() != 0)
	{
		bEngaged = true;
		ReachCm = CVarAirBuildReach.GetValueOnAnyThread();
	}
	else if (State)
	{
		bEngaged = State->IsEngaged();
		ReachCm = State->GetReachCm();
	}
	if (!bEngaged)
	{
		return false;
	}

	APawn* OwningPawn = Holo->GetConstructionInstigator();
	if (!OwningPawn)
	{
		static int32 NoPawnLog = 0;
		if (++NoPawnLog <= 8)
		{
			UE_LOG(LogAirBuild, Warning, TEXT("[AirBuild] holo %s has no construction instigator; cannot air-place."), *GetNameSafe(Holo));
		}
		return false;
	}

	FVector ViewLoc;
	FRotator ViewRot;
	OwningPawn->GetActorEyesViewPoint(ViewLoc, ViewRot);

	const FVector Dir = ViewRot.Vector();
	const float Reach = FMath::Clamp(ReachCm, SAB_MinReach, SAB_MaxReach);
	const FVector Target = ViewLoc + Dir * Reach;

	Hit.bBlockingHit = true;
	Hit.bStartPenetrating = false;
	Hit.Time = 1.0f;
	Hit.Distance = Reach;
	Hit.Location = Target;
	Hit.ImpactPoint = Target;
	Hit.Normal = -Dir;
	Hit.ImpactNormal = -Dir;
	Hit.TraceStart = ViewLoc;
	Hit.TraceEnd = ViewLoc + Dir * (Reach + 100.0f);
	Hit.HitObjectHandle = FActorInstanceHandle();
	Hit.Component = nullptr;
	Hit.PhysMaterial = nullptr;
	return true;
}

// Compute the HUD readout from the ACTUAL hologram transform. Must run AFTER the original
// UpdateHologramPlacement positions the hologram, so distances track the PLACED hologram rather than the
// raw aim-ray target -- e.g. when Ctrl snaps the building to the world grid, or ground adjustment shifts it.
static void ComputeAndReportHud(AFGHologram* Holo, UAirBuildSubsystem* State)
{
	if (!Holo || !State)
	{
		return;
	}
	const FVector ActualLoc = Holo->GetActorLocation();
	APawn* OwningPawn = Holo->GetConstructionInstigator();
	const float DistPlayerCm = OwningPawn ? FVector::Dist(OwningPawn->GetActorLocation(), ActualLoc) : 0.f;

	float DistGroundCm = -1.0f;
	if (UWorld* World = Holo->GetWorld())
	{
		FHitResult Ground;
		FCollisionQueryParams P(SCENE_QUERY_STAT(AirBuildGround), /*bTraceComplex*/ false);
		P.AddIgnoredActor(Holo);
		if (OwningPawn)
		{
			P.AddIgnoredActor(OwningPawn);
		}
		for (const AFGHologram* Child : Holo->GetHologramChildren())
		{
			if (Child)
			{
				P.AddIgnoredActor(Child);
			}
		}
		const FVector GroundEnd = ActualLoc - FVector(0.f, 0.f, SAB_GroundTraceMaxCm);
		if (World->LineTraceSingleByChannel(Ground, ActualLoc, GroundEnd, ECC_WorldStatic, P))
		{
			DistGroundCm = FMath::Max(0.f, ActualLoc.Z - Ground.ImpactPoint.Z);
		}
	}
	State->SetHudComputed(Holo, ActualLoc, DistPlayerCm, DistGroundCm);
}

// ---------------------------------------------------------------------------
// On-demand hook install (dev). Triggered by `airbuild.InstallHook`.
//
// Seam: AFGHologram::UpdateHologramPlacement(const FHitResult&) -- the per-frame
// placement orchestrator (substantial body -> hookable, unlike the unhookable
// AFGBuildGun::TraceForBuilding). Hooked as a virtual via the AFGHologram CDO so
// every building hologram that doesn't override it is covered; resource-node
// holograms (which DO override the placement steps) are excluded anyway.
//
// We rewrite the incoming hit BEFORE the original runs, so the hologram's own
// internal IsValidHitResult sees a valid synthetic hit (lets you place into sky).
// The param is const& but the referent is the build gun's mutable mHitResult, so
// a const_cast to mutate it in place is safe.
// ---------------------------------------------------------------------------
void FAirBuildHologramHook::InstallHook()
{
	if (GAirBuildHookInstalled)
	{
		UE_LOG(LogAirBuild, Display, TEXT("AirBuild hook already installed."));
		return;
	}

	AFGHologram* HologramCDO = GetMutableDefault<AFGHologram>();
	if (!HologramCDO)
	{
		UE_LOG(LogAirBuild, Error, TEXT("AirBuild: could not get AFGHologram CDO; hook not installed."));
		return;
	}

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::UpdateHologramPlacement, HologramCDO,
		[](auto& scope, AFGHologram* self, const FHitResult& hitResult)
		{
			if (self && !FAirBuildHologramHook::IsExcludedHologram(self))
			{
				// Freshness signal so the subsystem can scope the '-'/'=' keybind context to
				// hologram-active (even when not engaged). Excluded holograms don't count.
				if (UAirBuildSubsystem* State = UAirBuildSubsystem::Get(self))
				{
					State->NotifyHologramActive(self);
				}
			}

			const bool bActed = self ? RewriteHitForAirPlace(self, const_cast<FHitResult&>(hitResult)) : false;

			scope(self, hitResult); // positions the hologram (incl. Ctrl world-grid snap / ground adjust)

			// HUD distances are measured from the ACTUAL placed hologram, so they track the building (not
			// the raw aim ray) after snapping. Only when we actually air-placed it this frame.
			if (bActed)
			{
				if (UAirBuildSubsystem* State = UAirBuildSubsystem::Get(self))
				{
					ComputeAndReportHud(self, State);
				}
			}
		});

	// Rotation suppression while '=' (reach-adjust) is held.
	// The wheel still reaches vanilla rotation through the build-gun state's Scroll path; Enhanced Input
	// priority/consume does NOT block it. UFGBuildGunStateBuild::Scroll_Implementation
	// is the single chokepoint ABOVE both rotation paths -- the per-hologram ScrollRotate chain AND the
	// build-state's player-relative re-push (ApplyPlayerRelativeRotation/mPlayerRelativeScrollRotation) -- so
	// cancelling here suppresses rotation completely (and never fires Server_Scroll), while the reach wheel
	// handler runs independently. Gated to the local player + a non-excluded hologram + the '=' hold window;
	// otherwise the original runs and rotation is fully vanilla.
	UFGBuildGunStateBuild* BuildStateCDO = GetMutableDefault<UFGBuildGunStateBuild>();
	SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateBuild::Scroll_Implementation, BuildStateCDO,
		[](auto& scope, UFGBuildGunStateBuild* self, int32 delta)
		{
			if (!self)
			{
				return; // original runs
			}
			UAirBuildSubsystem* State = UAirBuildSubsystem::Get(self);
			if (!State || !State->IsAdjustHeld())
			{
				return; // not adjusting reach -> vanilla rotation
			}

			AFGHologram* Holo = self->GetHologram();
			if (Holo && FAirBuildHologramHook::IsExcludedHologram(Holo))
			{
				return; // resource-node family -> leave vanilla rotation/snapping
			}

			// Only suppress for the LOCAL player's build gun (never touch a remote player's in MP).
			if (AFGBuildGun* Gun = self->GetBuildGun())
			{
				UWorld* World = Gun->GetWorld();
				APlayerController* LocalPC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
				APawn* LocalPawn = LocalPC ? LocalPC->GetPawn() : nullptr;
				APawn* Instigator = Gun->GetInstigator();
				if (LocalPawn && Instigator && Instigator != LocalPawn)
				{
					return; // remote build gun -> vanilla
				}
			}

			scope.Cancel(); // swallow the rotation delta while reach is being adjusted
		});

	// Uneven-surface suppression while air-placing. Factory buildings floating in the air get the
	// UFGCDInvalidFloor disqualifier ("Surface is too uneven!"), which makes them invalid (red) - yet vanilla
	// itself ALLOWS the very same mid-air placement once the hologram is locked (the floor check is skipped for
	// deliberately-positioned holograms). Air-place is deliberate positioning, so we drop ONLY that one
	// disqualifier (keeping clearance/overlap/affordability) when air-place is engaged. Hooked on the
	// AFGBuildableHologram OVERRIDE (the base AFGHologram::CheckValidPlacement resolves to an unhookable import
	// thunk), AFTER vanilla fills the list, so the subsequent material/validity uses the trimmed set -> green.
	AFGBuildableHologram* BuildableCDO = GetMutableDefault<AFGBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::CheckValidPlacement, BuildableCDO,
		[](AFGBuildableHologram* self)
		{
			if (!self)
			{
				return;
			}
			UAirBuildSubsystem* State = UAirBuildSubsystem::Get(self);
			if (!State || !State->IsEngaged())
			{
				return; // not air-placing -> vanilla floor validity stands
			}
			if (FAirBuildHologramHook::IsExcludedHologram(self))
			{
				return; // resource-node family excluded anyway
			}

			TArray<TSubclassOf<UFGConstructDisqualifier>> Disqualifiers;
			self->GetConstructDisqualifiers(Disqualifiers);
			if (Disqualifiers.Remove(UFGCDInvalidFloor::StaticClass()) > 0)
			{
				// Surgical: clear all then re-add the rest, so only the uneven-floor disqualifier is dropped.
				self->ResetConstructDisqualifiers();
				for (const TSubclassOf<UFGConstructDisqualifier>& Disqualifier : Disqualifiers)
				{
					self->AddConstructDisqualifier(Disqualifier);
				}
			}
		});

	GAirBuildHookInstalled = true;
	UE_LOG(LogAirBuild, Display,
		TEXT("AirBuild: installed UpdateHologramPlacement hook. Set `airbuild.Enabled 1` and hold a building to test."));
}

static FAutoConsoleCommand GAirBuildInstallHookCmd(
	TEXT("airbuild.InstallHook"),
	TEXT("Install the Air Build placement hook on demand (dev)."),
	FConsoleCommandDelegate::CreateStatic(&FAirBuildHologramHook::EnsureInstalled));

// Auto-install the hook the first time the player opts in via `airbuild.Enabled 1`.
// Opt-in only (never at boot), so an unhookable seam can only crash on explicit
// enable, not on load -- and the player no longer has to remember InstallHook.
static void OnAirBuildEnabledChanged(IConsoleVariable* Var)
{
	if (Var && Var->GetInt() != 0 && !GAirBuildHookInstalled)
	{
		UE_LOG(LogAirBuild, Display, TEXT("[AirBuild] airbuild.Enabled set -> auto-installing placement hook."));
		FAirBuildHologramHook::EnsureInstalled();
	}
}

void FAirBuildHologramHook::Register()
{
	// Self-install on first enable; nothing installed at startup.
	CVarAirBuildEnabled.AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateStatic(&OnAirBuildEnabledChanged));

	UE_LOG(LogAirBuild, Display,
		TEXT("AirBuildHologramHook: ready. Set `airbuild.Enabled 1` to auto-install the hook and engage."));
}

void FAirBuildHologramHook::EnsureInstalled()
{
	// Idempotent: installs the native hooks once. Driven by the subsystem at world begin play so install
	// is decoupled from the temporary debug CVar.
	InstallHook();
}

void FAirBuildHologramHook::Unregister()
{
	// SML native hooks live for the process lifetime; nothing to tear down.
}

bool FAirBuildHologramHook::IsExcludedHologram(const AFGHologram* Hologram)
{
	if (!Hologram)
	{
		return false;
	}

	// AFGResourceExtractorHologram is the SDK base for "buildings that can only be
	// placed (snapped) on resource nodes" -- miners, oil/water extractors, the
	// water-pump (+child), and geothermal generators all derive from it.
	return Hologram->IsA(AFGResourceExtractorHologram::StaticClass());
}
