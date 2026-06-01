# Changelog

All notable changes to TFR Framework will be documented in this file.

---

## Unreleased

### Added

- Initial README description.
- Non-commercial license information.
- Roadmap documentation.
- Setup documentation.
- Components documentation.
- Performance guidelines.
- Internal `TFR_SurrenderBridge` fallback for future optional ACE compatibility.
- Internal `TFR_ParachuteBridge` fallback for future optional Parachute Framework compatibility.

### Changed

- Removed the direct ACE surrender call from the core arrest action.
- Removed direct Parachute Framework references from the core HALO component.
- Updated HALO core to use `TFR_ParachuteBridge` fallback for external parachute checks and auto deploy.
- Updated TFR radial controller compatibility for the current Workbench callback/API behaviour.
- Updated TFR radial component controller-changed callback compatibility.

### Fixed

- Fixed core arrest action dependency on `RPC_ACE_ForceSurrender`.
- Fixed core HALO dependency on `ParachuteComponent` and `ParachuteItemComponent`.
- Fixed radial compilation errors caused by changed radial callback signatures.

### Planned

- Modular dependency compatibility layer for ACE systems.
- Modular dependency compatibility layer for Parachute Framework systems.
- Safe position / safe placement systems.
- Mission systems for TFR Operations.
- Stable persistence systems.
- Simplified GameMode presets.
- Dedicated server optimization pass.
