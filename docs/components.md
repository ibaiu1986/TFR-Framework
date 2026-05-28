# TFR Framework Components

This document will describe the main components used by TFR Framework.

The goal is to keep component setup simple, clear and difficult to break.

---

## Component philosophy

TFR Framework should avoid forcing mission makers to manually guess where each component belongs.

Each component should have a clear purpose, safe default values and useful validation warnings when something is missing.

---

## Component categories

### GameMode components

Components that belong on the GameMode entity.

These should control global systems such as framework initialization, operation flow, persistence, mission logic or server-side managers.

### Character components

Components that belong on player characters or AI characters.

These should only be used when the system needs character-specific behavior.

### Optional components

Components that are only required when a specific system is enabled.

Optional systems should be disabled by default and should not create server load when unused.

---

## Rules for components

- Use clear names in Workbench.
- Avoid hidden dependencies.
- Avoid duplicated logic between components.
- Avoid required fields with no safe default.
- Log clear warnings when setup is incomplete.
- Do not run expensive logic if the system is disabled.
- Do not create repeated `CallLater` loops without limits.
- Do not scan the world unnecessarily.
- Keep debug output disabled unless explicitly enabled.

---

## Documentation requirement

Each important component should eventually document:

- where it must be placed;
- whether it is required or optional;
- what system it belongs to;
- which fields must be configured;
- what safe defaults it uses;
- what warnings it can generate.
