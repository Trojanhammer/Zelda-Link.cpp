# Zelda-Link.cpp

A terminal-based C++ program built for learning core language concepts — classes, inheritance, polymorphism, pointers, and memory management — through a working example instead of just theory.

The program simulates a turn-based combat loop: a `Player` (Link) selects one of two `Weapon` objects and attacks one of two `Enemy` objects (`Bokobolin`, `Stalfos`) each turn, until either both enemies are defeated or both weapons run out of durability.

## Concepts practiced

- **Classes & inheritance** — `Enemy` as a base class, `Bokobolin`/`Stalfos` as derived classes overriding behavior with `virtual`/`override`
- **Structs vs classes** — `EnemyStats` used to bundle constructor data instead of long parameter lists
- **Pointers & smart pointers** — `std::unique_ptr` for automatic memory cleanup, raw pointers (`Weapon*`, `Enemy*`) for non-owning references between objects
- **Stack vs heap** — explored directly with a debugger (breakpoints + memory inspector) to inspect actual object layout in memory
- **Input validation** — handling invalid choices, already-broken weapons, already-defeated enemies, and `std::cin` failure states

## Build

```bash
clang++ -std=c++17 main.cpp weapon.cpp Enemy.cpp Bokobolin.cpp Stalfos.cpp Player.cpp -o game
```

## Files

| File | Purpose |
|---|---|
| `main.cpp` | Game loop, input handling, turn logic |
| `weapon.h/.cpp` | `Weapon` class — damage, durability, attack |
| `Enemy.h/.cpp` | Base `Enemy` class — health, damage, `TakeAction()` |
| `Bokobolin.h/.cpp`, `Stalfos.h/.cpp` | Enemy subclasses with their own stats and `TakeAction()` override |
| `Player.h/.cpp` | Tracks the player's equipped weapon and current target |
| `notes.txt` | Personal study notes (English/Malay mixed) — memory layout, pointers vs references, stack vs heap |

## Limitations

- Enemies don't attack back — only the player currently deals damage
- No enemy AI or turn variation — `TakeAction()` exists on each enemy but isn't tied into the combat loop yet
- Stats (health, damage, durability) are hardcoded in each class's constructor rather than data-driven
- `Enemy`'s base class has a `virtual` function but no virtual destructor (known gap, kept as-is intentionally while still learning the implications)

## Future work

- Have enemies actually act on their turn instead of only receiving damage
- Move enemy/weapon stats into a data table instead of hardcoded constructor values, to scale past two enemy types without writing a new class per enemy
- Add a proper virtual destructor once polymorphic ownership (e.g. a shared `Enemy*` container) is introduced
- Eventually move from a text-only loop to a simple 2D visual layer (SDL2) once the underlying game logic is solid

## Status

Active learning project — the commit history intentionally shows real bugs found and fixed along the way (an infinite loop from unhandled `cin` failure, double-counted durability, missing damage guard on defeated enemies) rather than a single polished snapshot.
