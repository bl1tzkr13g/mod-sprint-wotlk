# mod-sprint-wotlk

AzerothCore 3.3.5a stamina-scaled sprint module inspired by [NSCairns/mod-sprint-335a-AzerothCore](https://github.com/NSCairns/mod-sprint-335a-AzerothCore).

**Status: prototype; not yet compiled or tested in-game.**

## Features
- Uses the character's actual **Stamina attribute**, not health, to determine max sprint duration.
- Diminishing returns protect balance across levels and high-stamina tanks.
- Default sprint duration: `clamp(4 + 2.5 * log2(1 + Stamina / 100), 1, 17)` seconds.
- 8 seconds to regenerate a completely depleted sprint pool.
- Separate toggles for battlegrounds, arenas, combat and mounts.
- Sends `AC_SPRINT:<percent>` stamina UI messages compatible with the original addon.

## Installation
1. Clone into `azerothcore-wotlk/modules/mod-sprint-wotlk`.
2. Rerun CMake and rebuild AzerothCore worldserver.
3. Copy `mod-sprint.conf.dist` to your server config as required for your deployment.
4. Until a dedicated addon is included, install the companion **ModSprintUI** addon from the original NSCairns project into `Interface/AddOns/ModSprintUI`.

## Configuration
See `conf/mod-sprint.conf.dist`.

## Gameplay examples

| Character Stamina | Approx. maximum sprint |
| ---: | ---: |
| 20 | 4.66 seconds |
| 100 | 6.5 seconds |
| 300 | 9.0 seconds |
| 600 | 11.02 seconds |
| 1,000 | 12.65 seconds |
| 1,500 | 14.0 seconds |
| 3,000 | 16.39 seconds |
| 4,000 | 17.0 seconds (cap) |

## Limitations / planned hardening
This is an initial module implementation, **not a verified build**. It currently reuses the original client addon rather than packaging our own. The original addon is expected to send `.sprintstart` and `.sprintstop` on hold/release. State is in memory and resets on worldserver restart. Further work should add logout cleanup and use an aura-based movement modifier to better integrate with changing speed buffs.
