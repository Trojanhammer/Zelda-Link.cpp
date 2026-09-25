# Worklist

Goal: finish this project in ~9-10 days. Focus is on mechanics, physics and
C++ fundamentals — not full content production, so asset breadth (more
enemies, more weapons) is deliberately deferred past this list.

## 1. Cleanup + enemy-data redesign (~1.5-2 days)
- [ ] Readability pass on `main.cpp` / `Player` / `Enemy` (naming, dead code, comments)
- [ ] Replace one-subclass-per-enemy (`Bokobolin`, `Stalfos`) with a data-driven
      approach — a list of `EnemyStats` that can be scaled per level, instead of
      a new C++ class for every enemy. This is what makes levels cheap later.

## 2. SDL2 rendering (~2-3 days)
- [ ] Draw sprites at `posX`/`posY` for Link + enemies
- [ ] Play intro video on launch
- [ ] Play hit-animation clips on attack

## 3. Level loop (~1-2 days)
- [ ] Advance level → same 2 enemies, scaled-up stats → repeat
- [ ] Win/lose screens actually rendered (currently empty `if` blocks in Part 3)
- [ ] Level count is just a loop bound now (no new art needed) — pick whatever number

## 4. Buffer (~1-2 days)
- [ ] Playtesting once it's visible + playable end-to-end, fix what breaks

## Explicitly not this week (phase 2)
- Save/load
- More weapons + weapon assets
- BOTW/TOTK sandbox project — **separate project**, real-time movement (not
  turn-based), SDL2 2D, camera/scrolling world, AABB collision, environment
  reactions (fire → updraft, rain/lightning, fuse) as trigger-volume + force
  systems, no physics engine needed. Starts after this project is done.
