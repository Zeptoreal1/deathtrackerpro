# Death Tracker Pro

Geode mod for Geometry Dash 2.2 that tracks attempts, deaths and progress for every level.

## Layout
- `src/data/`  `LevelStats` (per-level model, derived stats, JSON) and `StatsManager` (the single
  source of truth: sessions, recording, load/save to `levels.json` in the mod save dir)
- `src/ui/`    `Charts` (death map + progress graph), `StatsPopup` (tabbed stats menu),
  `SessionHud` (in-game panel), `Theme.hpp` (accent color / palette)
- `src/hooks/` thin hooks that only forward game events to the manager

## Adding features
- New stat: add the field to `LevelStats` (+ `toJson`/`fromJson`), record it in `StatsManager`.
- New tab: write a `buildXxx(CCNode*)` in `StatsPopup.cpp`, add its name to `kTabNames`.
- New setting: add it to `mod.json`, read it with `Mod::get()->getSettingValue<T>()`.

## Build
Needs the Geode SDK (`GEODE_SDK`). Or push to GitHub and run the "Build Geode Mod" action.
