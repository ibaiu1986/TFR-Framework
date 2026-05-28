# TFR Framework Performance Guidelines

This document defines the performance rules for TFR Framework.

Performance is a core requirement of the addon, especially for dedicated servers and long-running operations.

---

## Main goal

TFR Framework must avoid heavy systems that cause FPS drops, server lag or unstable gameplay.

Systems should be modular, limited and disabled when not used.

---

## Performance rules

- Avoid unnecessary world scans.
- Avoid duplicated `CallLater` loops.
- Avoid unlimited timers.
- Avoid debug spam.
- Avoid heavy logic running every frame.
- Avoid persistence systems that save too often.
- Avoid communication systems with no limits.
- Keep optional systems disabled by default.
- Use clear update intervals.
- Use limits for entities, groups, stored data and active systems.
- Prefer event-driven logic when possible.
- Keep dedicated server performance as a priority.

---

## Persistence

Persistence must be stable, predictable and lightweight.

It should not constantly save everything without limits.

Persistence systems should define:

- what is saved;
- when it is saved;
- how often it is saved;
- what limits exist;
- what happens when data is missing or invalid.

---

## Enemy communications

Enemy communication systems must also have clear limits.

They should avoid uncontrolled scans, repeated calls or constant updates across the whole map.

Communication systems should define:

- update frequency;
- maximum active groups;
- maximum communication range;
- enabled and disabled states;
- debug settings;
- server-side limits.

---

## Debugging

Debug output should be useful but controlled.

Debug logs should never spam the server console during normal gameplay.

Debug mode should be disabled by default.
