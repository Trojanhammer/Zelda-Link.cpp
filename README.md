# Zelda-Link.cpp

<img src="assets/compressed/link-2x.png" alt="Pixel art of Link raising the Master Sword" width="130"> <img src="assets/compressed/bokobolin-2x.png" alt="Pixel art of Bokobolin wielding a wooden club" width="346"> <img src="assets/compressed/stalfos-2x.png" alt="Pixel art of Stalfos skeleton warrior" width="234">

> **How this was built (AI use):** all the C++ game code is written by me, by hand. This is not vibe coding. I discuss ideas with Claude, write the code myself, then ask Claude to review it and point out mistakes so I can learn from them. The exception is the tooling: `tools/autoplay.py`, `tools/autoplay_driver.cpp` (an automated test that plays the game and checks rules) and `check-memory.sh` were written with AI, to find bugs faster.

> **Disclaimer:** This is an unofficial fan project with no affiliation to Nintendo. *The Legend of Zelda* and its characters belong to Nintendo — this project is just inspired by them, built purely for personal learning and hobby purposes. No plan to publicly release or distribute this as a playable game — any cross-platform builds are purely to learn the packaging process itself.

A game-physics exploration in C++ and SDL2, built **from scratch with no game engine and no physics library**. The goal is to understand how movement, knockback and a Tears-of-the-Kingdom-style *recall* (replaying past positions in reverse) actually work under the hood, instead of relying on an engine that hides them. It doubles as a way to learn core C++: classes, inheritance, pointers and memory management.

Renders with SDL2: sprites, background music, vsync.

## How I work on this

Every mechanic goes through the same loop:

1. **Try my own way.** Write it by hand, with no engine to lean on.
2. **Find the bugs.** Play it, and let `tools/autoplay.py` play it too: it presses the keys, prints every state change and checks rules on every frame.
3. **Remove them.** Fix the cause, not the symptom.
4. **Ask if there is a simpler way**, one with fewer places to go wrong.

Example: the turn flow started as `currentState` plus many bools (`isKnocked`, `isKnocking`, `isReturn`, ...). It worked, but the bugs kept coming from those two disagreeing, for instance one shared state that both the player and the enemy set and that only the enemy's check should have reacted to. The fix was one state per actor (`Entity::GameState`) instead of many bools shared across sides, so each entity's state can only ever be one thing at a time.

The core loop: a `Player` (Link) selects one of two `Weapon` objects and attacks one of two `Enemy` objects (`Bokobolin`, `Stalfos`) each turn, and enemies take their own turn back, cycling through living enemies and skipping dead ones, until either both enemies are defeated or Link runs out of health/durability. There are 3 levels; the same two enemy types return each level with higher stats, built from one data table instead of separate hand-written variables per level.

**Current focus:** levels and code cleanup — the mechanics have to satisfy me before animation, attack sounds or other polish get added. Next physics step once that's done: a fixed timestep (movement by speed × time instead of a fixed amount per frame) and real overlap-based collision.

## Concepts practiced

- **Classes & inheritance** — `Entity` as a base class, `Player`/`Enemy` as child classes.
- **Structs vs classes** — `EnemyStats` used to bundle constructor data instead of long parameter lists
- **Pointers & smart pointers** — `std::unique_ptr` for automatic memory cleanup, raw pointers for non-owning references between objects
- **Move semantics & ownership** — `levelEnemies` owns each enemy directly as a `unique_ptr`; learned the hard way that brace-initializing a container of a move-only type tries to copy it (`std::initializer_list` only ever gives const access), so it has to be built with `push_back` and moved in with `std::move`
- **Stack vs heap** — explored directly with a debugger (breakpoints + memory inspector) to inspect actual object layout in memory; also where an object actually lives when it's owned by a `unique_ptr` versus a variable's own scope
- **Basic 2D physics** — direction vectors, normalization, distance checks for "has this entity arrived at its target"
- **Knockback and recall** — velocity with friction, and replaying stored positions in reverse to walk back to base
- **State machines** — one `currentState` per entity instead of scattered bools; each side (player, enemy) checks and transitions its own state independently
- **Data-driven design** — enemy stats per level live in one `LevelStats` table instead of a hand-written variable per level/enemy pair
- **Automated testing** — a script that plays the game headless and checks rules every frame
- **SDL2 fundamentals** — window/renderer setup, sprite drawing, background music, the event loop and why blocking it (e.g. `SDL_Delay`) breaks rendering on macOS

## Build

```bash
clang++ -std=c++17 main.cpp weapon.cpp Enemy.cpp Entity.cpp Player.cpp \
  -I/opt/homebrew/include/SDL2 -D_THREAD_SAFE -L/opt/homebrew/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -Wl,-framework,Cocoa \
  -o game
```

The game runs silently if `assets/audio/` isn't present locally — the background music file is gitignored (copyrighted).

## Files

| File | Purpose |
|---|---|
| `main.cpp` | Game loop, SDL2 window/sprite/music setup, input handling, turn logic, level progression |
| `weapon.h/.cpp` | `Weapon` class — damage, durability, attack |
| `Entity.h/.cpp` | Base class of `Player` and `Enemy`: health, position/velocity, per-entity state, damage, attack movement, and the knockback / recall physics (written once, shared by both) |
| `Enemy.h/.cpp` | `Enemy`, built from `EnemyStats` |
| `Player.h/.cpp` | Player-only state: equipped weapon, target enemy |
| `CombatUtils.h` | Shared `ApplyDamage()` used by both `Enemy` and `Player` |
| `main-sdl.cpp` | Standalone SDL2 exploration file (window/renderer/event loop basics), separate from the actual game |
| `notes.txt` | Personal study notes (English/Malay mixed) — memory layout, pointers vs references, stack vs heap |
| `sdl-notes.txt` | Notes specifically on SDL2 fundamentals and V-Sync |
| `tools/autoplay.py`, `tools/autoplay_driver.cpp` | Plays the real game headless by injecting key presses, prints every state change and checks rules. This is how the bugs get found (written with AI) |
| `check-memory.sh` | Samples the running game's RAM once per second into `memory-log.csv` (written with AI) |
| `TODO.md` | Worklist |
| `assets/compressed/` | Cropped, palette-reduced character sprites (a few KB each) — the `*-2x.png` copies are what the README shows |
| `assets/*.mp4` | Intro and hit-animation clips (not rendered in-game yet) |
| `assets/*.jpeg` | Original 2048×2048 character art, kept as source for the compressed sprites |
| `assets/audio/` | Background music, local-only, gitignored (copyrighted) |

## Limitations

- No win/lose screen text yet, and no sound effects on attacks — intentionally on hold until the mechanics themselves feel solid
- `tools/autoplay.py` is stale against the level system (it still expects a flat enemy list); hasn't been updated yet

## Future work

- Sound effects for attacks
- An intro screen before the game starts
- Package builds for Windows, macOS, and Linux
- Win/lose text on screen (SDL_ttf)
- Update `tools/autoplay.py` for the level system
- Fixed timestep and real overlap-based collision
- Long-shot goal: get this running on a jailbroken Wii

## Status

Active learning project — the commit history intentionally shows real bugs found and fixed along the way rather than a single polished snapshot. Early ones: an infinite loop from unhandled `cin` failure, double-counted durability, missing damage guard on defeated enemies. Since then: the shared-state bug that let the enemy's turn start early, a state that never transitioned away so a physics call fired every frame instead of once, a null pointer dereference from checking the wrong side's target during the enemy's turn, and a couple of off-by-one bugs (an array index, a level-count comparison). Each fix is written up with its cause and location in the commit message.
