# TFR Framework

TFR Framework is a modular framework for Arma Reforger designed to provide reusable gameplay, operations, persistence, interaction, placement and utility systems for communities, servers and future TFR addons.

TFR Framework, known internally in Workbench as TFR Operations, is the same addon.

It is intended as a shared foundation for TFR projects, with a focus on clean configuration, modular systems, dedicated server stability and long-term expandability.

## Publicable target

The current target is to make TFR Framework / TFR Operations publicable with:

- modular core;
- no hard dependencies on ACE, RHS or Parachute Framework;
- radial compatibility with ACE/RHS/vanilla;
- centralized Safe Position / safe placement;
- improved Spawn Director;
- configurable AmbientAI;
- clean dynamic missions;
- ordered persistence;
- **no loose GenericEntities as gameplay configuration**;
- clear components for mission makers;
- controlled logs;
- easy GameMode setup.

## Core rules

- Do not delete working code.
- Do not rewrite systems blindly.
- Do not add hard dependencies for optional integrations.
- Optional dependencies must only affect the systems they enable.
- Missing optional dependencies must not break compilation.
- Configuration errors should produce clear logs instead of hard failures where possible.
- Gameplay setup must use clear TFR components/prefabs, not loose GenericEntities with magic names.

## Documentation

- [`docs/roadmap.md`](docs/roadmap.md)
- [`docs/architecture.md`](docs/architecture.md)
- [`docs/components.md`](docs/components.md)
- [`docs/component-placement.md`](docs/component-placement.md)
- [`docs/setup.md`](docs/setup.md)
- [`docs/performance.md`](docs/performance.md)

## License

This project is licensed for non-commercial use only.

You may use, modify, and share this addon for personal, community and non-commercial purposes.

Commercial use is not allowed without prior written permission from the copyright holder.
