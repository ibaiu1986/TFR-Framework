# TFR Framework Setup

This document will describe how to set up TFR Framework in Arma Reforger Workbench.

The goal of the setup process is to make the framework easy to configure, hard to break and clear for mission makers, server owners and TFR addon developers.

---

## Setup philosophy

TFR Framework should avoid fragile setups where users must manually guess which components are required.

The framework should provide prepared GameMode presets, safe defaults and clear validation warnings when something is missing.

---

## Planned GameMode presets

### TFR_GameMode_Core.et

Minimal framework setup.

This preset should include only the basic required systems needed by the framework.

### TFR_GameMode_Operations.et

Standard operations setup.

This preset should include the systems required for dynamic operations, objectives, intel chains and common gameplay flow.

### TFR_GameMode_FullPersistent.et

Full persistent operations setup.

This preset should include persistence and extended operation systems, with clear performance limits for dedicated servers.

---

## Setup rules

- Required components must be reduced as much as possible.
- Critical fields should not be spread across too many places.
- Optional systems should be disabled by default.
- Defaults should be safe.
- Logs should clearly explain what is missing.
- Workbench component names should be clear.
- Example prefabs should work out of the box.
- Dedicated server performance must always be considered.
