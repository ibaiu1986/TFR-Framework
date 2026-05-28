# TFR Framework Roadmap

This document defines the four main development fronts of the TFR Framework mod.

---

## Front 1 — Safe Position / Safe Placement

The first front focuses on safe and reliable entity placement.

The objective is to ensure valid and safe positions for spawning, deploying or placing entities, avoiding broken, dangerous or invalid locations.

---

## Front 2 — Mission System for Mission Makers

The second front focuses on providing clean and reusable tools for creating operations and missions.

The objective is to give mission makers prepared logic and systems that make it easier to build scenarios without recreating the same mission flow every time.

---

## Front 3 — Stable Persistence

The third front focuses on clean, stable and efficient persistence.

Persistence must avoid lag, FPS drops and heavy systems. Configuration must be clear and robust, because if even the addon creator has problems setting up the required components correctly, the system needs simplification and documentation.

---

## Front 4 — Addon Simplification, Cleanup and Optimization

The fourth front focuses on making TFR Framework / TFR Operations easy to configure, difficult to break and lightweight on dedicated servers.

The setup must be clear enough that a mission maker can use it without knowing the internal code.

If even the addon creator cannot set the components correctly on the first try, the system is too fragmented or poorly documented.

This front includes:

- fewer mandatory components;
- fewer critical fields spread across different places;
- prepared presets and GameModes;
- functional example prefabs;
- separated minimal and full configuration modes;
- clear names in Workbench;
- safe default values;
- useful validation warnings;
- logs that explain exactly what is missing;
- short documentation for each component;
- a setup checklist;
- avoiding strong FPS drops;
- no unnecessary scans;
- no duplicated `CallLater` loops;
- no debug spam;
- optional systems disabled when not used;
- clear limits for persistence and enemy communications.

The final goal of Front 4 is that a mission maker should be able to use:

1. `TFR_GameMode_Core.et`
2. `TFR_GameMode_Operations.et`
3. `TFR_GameMode_FullPersistent.et`

without having to guess which components belong on the GameMode, which ones belong on the character and which ones are optional.
