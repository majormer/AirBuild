# Changelog

All notable changes to Air Build will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> **Audience note:** This changelog is read by players, not developers. Entries should describe what the user experiences — what the feature does and what it feels like to use. Class names, internal APIs, and implementation details belong in code comments or design docs, not here. Unless an entry says otherwise, changes apply to both single-player and multiplayer.

> **Status:** Air Build is a small, single-mechanic mod — it does one thing. Feedback and bug reports are welcome.

> **AI Assisted Development Used.** Air Build was built with AI assistance (see AI_DISCLOSURE.md). The mod places standard Satisfactory buildings at their normal cost — it does **not** grant free buildings or free resources.

---

## [1.1.0] - 2026-06-30

> *Smarter air placement, a friendlier config menu, and a fix for the missing mod icon.*

### Added

- **Auto Air Placement modes** — A new **Auto Air Placement** setting controls how air-place engages, so you don't always have to reach for the toggle key:
  - **Off** (default) — unchanged from 1.0.0: press the toggle key to turn air-place on for a build.
  - **Always** — air-place is on automatically whenever you're holding a buildable; the toggle key suspends it for that build.
  - **Smart** — air-place engages only when there's nothing within reach to snap to: aim at the ground or an existing building and it places or snaps normally, aim at open sky and it floats out at your reach. The toggle key still overrides per build in any mode.

- **Config menu values can be typed in directly** — Air Placement and Indicator settings now use a slider you can also type an exact value into, instead of a plain number box. Drag for a quick adjustment, or type a precise number — either way works.

---

## [1.0.0] - 2026-06-29

> *First release. A Finalomega Labs mod, built from an idea suggested by **MovedToStoat** — a vanilla-friendly way to drop a building into mid-air without endless nudging. Hold a building on the build gun, flip on air-place, and float it out along your aim line at a distance you control — instead of spamming the vanilla nudge keys to lift it into place. Standalone and self-contained; it does **not** depend on any other mod.*

### Added

- **Air-placement mode for the build gun** — Hold a building on the build gun and place it floating in the air at a distance you set, rather than nudging it up one step at a time. When the mode is engaged, the held building rides along your aim line at your chosen **reach**; push it further out or pull it back in to land it exactly where you want it in open space. Normal Satisfactory placement rules still apply — clearance, overlap, hard conflicts, and material cost all behave as in the base game, so a piece that can't legally sit where it's floating still reads invalid. It places standard buildings at standard cost; nothing here is free.

- **Controls — toggle air-place, and adjust reach (all rebindable)** — Press **Minus ( - )** to toggle air-place mode on and off. It's sticky: once on, it stays on until you toggle it off, and it resets when you leave the build gun. To change how far the building floats, **hold Equals ( = )** and roll the mouse wheel — push it out or pull it in, then release and it stays put. The plain mouse wheel still **rotates** the hologram exactly like vanilla, and **Ctrl** still snaps to the world grid exactly like vanilla. Every binding can be remapped under **Options → Keybindings → "Air Build."**

- **On-screen indicator** — While air-place is engaged, a small on-screen readout shows the building's **Distance** from you, its **Ground** height above the surface below it, and the current **Reach**. Its position and size are adjustable in the config (see below), so you can tuck it wherever suits your HUD.

- **Resource-node buildings are excluded** — Miners, oil and water extractors, frackers, geothermal generators, and resource wells are left out on purpose. They have to snap to a resource node to work, so air-place simply doesn't engage for them — they behave 100% vanilla in every case, single-player and multiplayer alike.

- **Uneven-surface placement for floating factory buildings** — Vanilla normally blocks a factory building with "Surface is too uneven!" when the floor below it isn't flat enough. For an air-placed building, Air Build drops just that one check, so a Constructor (and the like) reads valid while floating in open air. This mirrors what vanilla already permits — vanilla allows the same mid-air placement when the hologram is locked — so it isn't a new freedom, just a smoother way to reach it. Every other placement rule still applies.

- **In-game config (Mod Settings → Air Build)** — Two sections you can tune to taste:
  - **Air Placement** — **Default Reach** (15 m), **Minimum Reach** (2 m), **Maximum Reach** (200 m), and **Reach Step** (0.5 m per wheel notch).
  - **Indicator** — **Show Indicator** (on by default), **Position X**, **Position Y**, and **Scale**.

### Multiplayer

- Works in **single-player and multiplayer, including dedicated servers** — verified with a factory building on a Windows dedicated server. Resource-node buildings are excluded in all cases by design.

---

*Report issues on [GitHub](https://github.com/majormer/AirBuild/issues). Community & support on [Discord](https://discord.gg/SgXY4CwXYw).*
