# <img src="https://github.com/majormer/AirBuild/blob/main/Content/AirBuild.png?raw=true" width="150" alt="Air Build Logo"> Air Build

![Status](https://img.shields.io/badge/Status-Released-brightgreen) ![Version](https://img.shields.io/badge/Version-1.0.0-blue) ![Satisfactory](https://img.shields.io/badge/Satisfactory-1.2-blue) ![Engine](https://img.shields.io/badge/Engine-UE%205.6-blue) ![SML](https://img.shields.io/badge/SML-3.12-blue) ![Multiplayer](https://img.shields.io/badge/Multiplayer-Supported-brightgreen) ![AI Assisted Development Used](https://img.shields.io/badge/AI%20Assisted%20Development%20Used-Disclosure%20Below-blue)

> **Multiplayer:** Air Build works in single-player and multiplayer, including dedicated servers (verified with a factory building on a Windows dedicated server).

**Quick links:** [What is it?](#-what-is-air-build) • [Controls](#-controls) • [How it works](#-how-it-works) • [Should you install?](#-should-you-install-air-build) • [Config](#-config) • [Source](https://github.com/majormer/AirBuild) • [Issues](https://github.com/majormer/AirBuild/issues) • [Discord](https://discord.gg/SgXY4CwXYw)

---

## 🛩️ What is Air Build?

Air Build is a small, focused mod for Satisfactory. It adds an **air-placement mode** to the build gun: hold a building, toggle the mode on, and the hologram floats in the air at a camera-relative distance along your aim line — instead of spamming the vanilla nudge keys to lift a piece into place.

It's one mechanic, kept small:

- **Toggle air-place mode** while holding any supported building.
- **The hologram floats** along your view at a set **reach** (how far out it sits).
- **Push it out or pull it in** with a held modifier plus the mouse wheel.
- **Normal rotation and grid-snap still work** exactly like vanilla.

Air Build does **not** give you free buildings or free resources. It places standard Satisfactory buildings at the normal cost — it just lets you put them in the air more easily.

Air Build is a **Finalomega Labs** mod. It is **standalone and self-contained** — it does **not** depend on any other mod. The idea came from a player (handle **MovedToStoat**), who suggested a vanilla-friendly way to place buildings in mid-air without endless nudging.

<div align="center">

<img src="https://github.com/majormer/AirBuild/blob/main/images/FloatingFoundation.png?raw=true" width="430" alt="Air-placing a foundation in mid-air with the on-screen indicator">

*Air-placing a foundation mid-air — the indicator shows distance, world height, height above the ground below, and reach.*

</div>

---

## 🎮 Controls

All Air Build controls are **rebindable** in **Options → Keybindings → "Air Build"**.

| Action | Default Key | Description |
|---|---|---|
| Toggle air-place mode | `-` (Minus) | Turns air placement on/off. Sticky — it stays on until you toggle it off, and resets when you leave the build gun. |
| Adjust reach | `=` (Equals) + `Scroll Wheel` | **Hold** `=` and roll the mouse wheel to push the building further out or pull it in. Release and the reach stays where you set it. |
| Rotate hologram | `Scroll Wheel` (no modifier) | Rotates the hologram, exactly like vanilla. |
| Grid snap | `Ctrl` | Snaps to the world grid, exactly like vanilla. |

**How to use it:** Equip a building, tap `-` to engage air placement, then hold `=` and scroll to set how far out it floats. Scroll without `=` to rotate, hold `Ctrl` to grid-snap — both behave the same as the base game. Tap `-` again to turn the mode back off.

---

## 🔧 How It Works

When you engage air placement, the held building floats along your aim line at the current **reach**. Adjust the reach with `=` + wheel to slide it nearer or further along that line.

**Vanilla placement validity is preserved** — clearance, overlap, hard conflicts, and cost all behave exactly as in the base game. A piece that can't legally sit where it's floating reads invalid, just like nudging it there would.

There is **one deliberate exception**: the *"Surface is too uneven!"* floor check is dropped for air-placed **factory buildings**. Vanilla itself already allows that same mid-air placement when the hologram is locked, so Air Build lets a Constructor (for example) float freely and read valid.

**Resource-node buildings are excluded.** Miners, oil and water extractors, frackers, geothermal generators, and resource wells all have to snap to a resource node, so air placement simply doesn't engage for them — they behave 100% vanilla.

While air placement is engaged, an **on-screen indicator** shows:

- **Distance** — how far the building is from the player.
- **Ground** — its height above the ground directly below it.
- **Reach** — the current reach setting.

The indicator's position and size are configurable (see [Config](#-config)).

---

## 🧭 Should You Install Air Build?

Air Build is useful if you want to drop buildings into the air quickly without nudging them up one step at a time.

### Air Build is probably for you if you want to:

- Place factory buildings and structural pieces floating in the air with a single toggle.
- Slide a held building nearer or further along your aim line with the scroll wheel.
- Keep a vanilla-like save: Air Build places standard Satisfactory buildings at normal cost.
- Add one small, focused convenience without a large toolkit.

### What Air Build does **not** do:

- **No free buildings or free resources.** It places standard Satisfactory buildings at their normal material cost.
- **No air placement for resource-node buildings.** Miners, oil/water extractors, frackers, geothermal generators, and resource wells are intentionally excluded — they must snap to a node, and they stay fully vanilla.
- **No creative-mode replacement, scaling, or presets.** This is a single-mechanic mod, not a large building toolkit.

### Multiplayer

- Air Build works in **single-player and multiplayer, including dedicated servers** — verified with a factory building on a Windows dedicated server. Resource-node buildings are excluded in all cases by design.

---

## ⚙️ Config

Air Build settings live in-game under **Mod Settings → Air Build**, in two sections.

<div align="center">

<img src="https://github.com/majormer/AirBuild/blob/main/images/AirBuildSettings.png?raw=true" width="560" alt="Air Build mod settings page: Air Placement and Indicator sections">

</div>

### Air Placement

| Setting | Default | Description |
|---|---|---|
| Default Reach | 15 m | How far out the building floats when you first engage air placement. |
| Minimum Reach | 2 m | The closest you can pull a building in. |
| Maximum Reach | 200 m | The furthest you can push a building out. |
| Reach Step | 0.5 m | How far the reach changes per wheel notch while holding `=`. |

### Indicator

| Setting | Default | Description |
|---|---|---|
| Show Indicator | On | Shows the Distance / Ground / Reach readout while air placement is engaged. |
| Position X | — | Horizontal screen position of the indicator. |
| Position Y | — | Vertical screen position of the indicator. |
| Scale | — | Size of the indicator. |

---

## 🧪 Finalomega Labs

Air Build is a **Finalomega Labs** mod — a small, focused, standalone mod. It does not depend on any other mod. It targets **Satisfactory 1.2** on **Unreal Engine 5.6** with **SML 3.12**; its engine module is native C++.

---

## 🤖 AI Disclosure

Air Build uses AI-assisted development. AI tools help with investigation, drafting, refactoring, documentation, and debugging support. Final decisions, testing, release preparation, and maintenance remain the responsibility of the project maintainer.

AI assistance does not replace community testing. Air Build is validated through developer review, in-game testing, and bug reports.

---

## 💬 Getting Help

Air Build is **v1.0.0**. If something behaves unexpectedly, please report it on [GitHub Issues](https://github.com/majormer/AirBuild/issues) with your Satisfactory version, SML version, session type (single-player or multiplayer), and clear reproduction steps.

- **Report bugs / track issues:** https://github.com/majormer/AirBuild/issues
- **Source:** https://github.com/majormer/AirBuild
- **Discord (community & support):** https://discord.gg/SgXY4CwXYw

---

## ☕ Support

If Air Build is useful to you, you can support development on **[Ko-fi](https://ko-fi.com/finalomega)**. Thank you!
