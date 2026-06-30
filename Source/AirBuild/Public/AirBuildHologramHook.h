#pragma once

#include "CoreMinimal.h"

class AFGHologram;

/**
 * Installs and owns the build-gun hologram positioning hook that powers air-place mode.
 *
 * Design (locked for MVP):
 *  - Positioning ONLY. When air-place mode is active, the hologram is driven to
 *    (cameraLocation + cameraForward * Reach) instead of the build gun's world trace.
 *  - Validity is preserved except for ONE deliberate exception: the UFGCDInvalidFloor
 *    ("Surface is too uneven!") disqualifier is dropped while air-placing, because vanilla
 *    itself allows the same mid-air placement for a locked (deliberately-positioned) hologram.
 *    Everything else - clearance, overlap, hard conflicts, affordability - is left to vanilla.
 *  - Reach is player-controlled (scroll-based, rebindable via Enhanced Input) and clamped
 *    to [MinReach, MaxReach] from config.
 *
 * Exclusions (mode does NOT engage; falls back to vanilla snapping):
 *  - Resource-node hologram family — miners, oil/water/resource extractors, frackers,
 *    resource wells. Two reasons:
 *      1. They can't validly air-place anyway (require a snapped resource / deep water).
 *      2. MP safety — the water extractor spawns a child hologram and the authority can
 *         hit a null mSnappedExtractableResource assert on the authority.
 *         Air-place must stay clear of that path entirely.
 *    The exact class list is confirmed against the SDK hologram hierarchy before wiring
 *    the SUBSCRIBE_METHOD hook; IsExcludedHologram() is the single gate.
 */
class FAirBuildHologramHook
{
public:
	/** Installs the build-gun hologram hook. Safe to call once at module startup. */
	static void Register();

	/** Idempotently installs the UpdateHologramPlacement hook (called on the first real air-place engage). */
	static void EnsureInstalled();

	/** Removes the hook. Called at module shutdown. */
	static void Unregister();

	/** True if air-place mode must NOT engage for this hologram (resource-node family). */
	static bool IsExcludedHologram(const AFGHologram* Hologram);

private:
	/**
	 * Installs the native hooks (placement rewrite, rotation suppression, uneven-floor disqualifier drop).
	 * A member of this class so the AccessTransformers Friend grant on AFGBuildableHologram applies when
	 * taking &AFGBuildableHologram::CheckValidPlacement (protected). Idempotent.
	 */
	static void InstallHook();
};
