#!/usr/bin/env python3
"""
autoplay.py - plays the real game by itself, then tells you what happened.

It never touches your main.cpp. It makes a temporary copy in tools/build/ that
  * prints every game-state change (with the positions of Link and the enemies), and
  * checks a few rules on every frame (listed below),
links that copy with tools/autoplay_driver.cpp (which presses the keys), and runs it with
SDL's "dummy" video driver, so no window opens and a whole round takes about a second.

Usage:
    python3 tools/autoplay.py                          one round: weapon 1, enemy 1
    python3 tools/autoplay.py 11 12 22                 three rounds: (1,1) (1,2) (2,2)
    python3 tools/autoplay.py --start StartEnemyTurn   start straight in a state, press nothing
                                                       (handy for testing the enemy turn on its own)
    python3 tools/autoplay.py --root SOME/FOLDER       test another copy of the project

Rules checked on every frame (a broken rule prints [VIOLATION]):
    0. the turn counter never wraps around below 0, and in every enemy turn each living enemy attacks exactly once
    1. nobody is out of range (flew off the screen, or the position became NaN)
    2. during the player's turn (Link has a TargetEnemy), an enemy that is not the target stays on its base,
       and the target only leaves its base when it has been knocked
    3. outside the player's turn, Link only leaves his base when he has been knocked

Verdict: FAIL if a rule is broken, if the game loop ends by itself, if a scripted round never gets back
to the first state, or if the game hangs. Exit code 0 = pass, 1 = fail, 2 = could not build.

The inserted code expects main.cpp to have variables called currentState, currentEnemyIndex, turn, MainPlayer
and enemies (a vector of Enemy*). If you rename them, update INSTRUMENTATION below.
"""

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent

# Same flags as the build command in README.md.
CXXFLAGS = ["-std=c++17", "-g", "-O0", "-I/opt/homebrew/include/SDL2", "-D_THREAD_SAFE"]
LDFLAGS = ["-L/opt/homebrew/lib", "-lSDL2main", "-lSDL2", "-lSDL2_image", "-lSDL2_mixer", "-Wl,-framework,Cocoa"]

# Names of two states in your enum, used for the rule "every living enemy attacks exactly once per enemy turn".
# If you rename them, change them here (if they are not in the enum the rule is simply skipped).
ENEMY_TURN_START = "StartEnemyTurn"
ENEMY_ATTACKING = "EnemyAttacking"

MARKER = "SDL_RenderPresent(renderer);"          # the per-frame instrumentation goes right before this line
AFTER_LOOP_MARKER = "SDL_DestroyRenderer(renderer);"   # first line after the game loop: we log the final state here,
                                                       # because a `break;` skips the per-frame instrumentation
TERMINAL_STATES = {"AllEnemyDead", "PlayerDie"}  # states that are allowed to end the game

INSTRUMENTATION = r'''
    {   // ---- inserted by tools/autoplay.py (this copy only, never your real main.cpp) ----
        static long ap_frame = 0;
        ap_frame++;

        static int ap_lastState = -1;
        if ((int)currentState != ap_lastState) {
            std::ostringstream ap_line;   // build the whole line first, then write it in one go (the key-press thread also logs)
            ap_line << "[state] " << (int)currentState << " frame=" << ap_frame << " turn=" << (int)turn
                    << " link=(" << MainPlayer->posX << "," << MainPlayer->posY << ")";
            for (size_t i = 0; i < enemies.size(); i++)
                ap_line << " e" << i << "=(" << enemies[i]->posX << "," << enemies[i]->posY << ")";
            ap_line << " hp=" << MainPlayer->health << " idx=" << currentEnemyIndex << " alive=";
            for (Enemy* e : enemies) ap_line << (e->isAlive ? '1' : '0');
            ap_line << "\n";
            std::cerr << ap_line.str();
            ap_lastState = (int)currentState;
        }

        auto ap_home = [](Entity* e) { return e->posX == e->basePosX && e->posY == e->basePosY; };
        auto ap_inRange = [](Entity* e) { return e->posX > -2000 && e->posX < 2000 && e->posY > -2000 && e->posY < 2000; };
        static std::set<std::string> ap_seen;   // report each kind of violation once
        auto ap_violation = [&](const char* what) {
            if (ap_seen.insert(what).second)
                std::cerr << (std::string("[VIOLATION] frame=") + std::to_string(ap_frame) + " " + what + "\n");
        };

        if (turn > 100)
            ap_violation("the turn counter wrapped around (it went below 0 - turn-- when turn was already 0?)");
        if (!ap_inRange(MainPlayer.get()))
            ap_violation("Link is out of range (flew off the screen, or his position is NaN)");
        for (Enemy* e : enemies)
            if (!ap_inRange(e))
                ap_violation("an enemy is out of range (flew off the screen, or its position is NaN)");

        Enemy* ap_target = MainPlayer->TargetEnemy;
        if (ap_target != nullptr) {   // it is the player's turn
            for (Enemy* e : enemies) {
                if (e != ap_target && !ap_home(e))
                    ap_violation("an enemy that is NOT Link's target left its base during the player's turn");
                if (e == ap_target && !ap_home(e) && !e->isKnocked)
                    ap_violation("Link's target left its base without being knocked (is the enemy attacking during the player's turn?)");
            }
        } else if (!ap_home(MainPlayer.get()) && !MainPlayer->isKnocked) {
            ap_violation("Link left his base outside his own turn without being knocked");
        }
    }
    '''


def parse_enum(source):
    """Return the names inside `enum GameState { ... }`, in order (so we can print names instead of numbers)."""
    match = re.search(r"enum\s+GameState\s*\{(.*?)\}", source, re.S)
    if not match:
        sys.exit("could not find `enum GameState { ... }` in main.cpp")
    body = re.sub(r"//[^\n]*", "", match.group(1))   # drop comments
    return [name.strip() for name in body.split(",") if name.strip()]


def has_main(path):
    return re.search(r"\bint\s+main\s*\(", path.read_text()) is not None


def run(command, what):
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"BUILD FAILED while {what}:\n")
        print(result.stderr)
        print("(the inserted code expects main.cpp to have variables named currentState, currentEnemyIndex, turn, MainPlayer and enemies -"
              " if you renamed them, update INSTRUMENTATION in tools/autoplay.py)")
        sys.exit(2)


def build(root, build_dir, source, start_state, states):
    text = source
    for marker in (MARKER, AFTER_LOOP_MARKER):
        if text.count(marker) != 1:
            sys.exit(f"expected `{marker}` exactly once in main.cpp (the instrumentation goes right before it)")
    # Vsync would cap the game at 60 fps, so a round takes seconds instead of milliseconds. It does not matter for
    # the game logic, so the test copy runs without it.
    text = re.sub(r"\|\s*SDL_RENDERER_PRESENTVSYNC", "", text)                       # ACCELERATED | PRESENTVSYNC -> ACCELERATED
    text = text.replace("SDL_RENDERER_PRESENTVSYNC", "SDL_RENDERER_ACCELERATED")    # PRESENTVSYNC alone -> ACCELERATED
    text = text.replace(MARKER, INSTRUMENTATION + MARKER)
    text = text.replace(AFTER_LOOP_MARKER,
                        'std::cerr << (std::string("[final] ") + std::to_string((int)currentState) + "\\n");\n    ' + AFTER_LOOP_MARKER)
    if start_state:
        if start_state not in states:
            sys.exit(f"--start {start_state}: not a state. States are: {', '.join(states)}")
        text = re.sub(r"(GameState\s+currentState\s*=\s*)\w+(\s*;)", rf"\g<1>{start_state}\g<2>", text, count=1)
    text = "#include <set>\n#include <sstream>\n#include <string>\n" + text

    instrumented = build_dir / "main_instrumented.cpp"
    instrumented.write_text(text)

    main_object = build_dir / "main_instrumented.o"
    run(["clang++", *CXXFLAGS, "-I", str(root), "-Dmain=game_main", "-Wno-return-type",
         "-c", str(instrumented), "-o", str(main_object)], "compiling the instrumented main.cpp")

    others = [p for p in sorted(root.glob("*.cpp")) if not has_main(p)]   # every game .cpp except the ones with a main()
    exe = build_dir / "autoplay"
    run(["clang++", *CXXFLAGS, "-I", str(root), str(TOOLS_DIR / "autoplay_driver.cpp"), str(main_object),
         *map(str, others), *LDFLAGS, "-o", str(exe)], "linking")
    return exe


def main():
    parser = argparse.ArgumentParser(description="Play the game by itself and report what happened.",
                                     formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    parser.add_argument("rounds", nargs="*", help="rounds to play, each <weapon key><enemy key>, e.g. 11 12 22")
    parser.add_argument("--start", metavar="STATE", help="start the game in this state instead of the normal one")
    parser.add_argument("--root", default=str(TOOLS_DIR.parent), help="project folder to test (default: this repo)")
    parser.add_argument("--step", type=float, default=2.0, help="seconds to let each round play out (default 2)")
    parser.add_argument("--timeout", type=float, help="kill the game after this many seconds (default: worked out from the rounds)")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    source = (root / "main.cpp").read_text()
    states = parse_enum(source)
    initial = re.search(r"GameState\s+currentState\s*=\s*(\w+)\s*;", source)
    idle_state = initial.group(1) if initial else states[0]   # the menu state a finished round should come back to

    rounds = args.rounds
    if not rounds and not args.start:
        rounds = ["11"]

    build_dir = TOOLS_DIR / "build"
    build_dir.mkdir(exist_ok=True)
    print(f"building a test copy of {root / 'main.cpp'} ...")
    exe = build(root, build_dir, source, args.start, states)

    timeout = args.timeout or (len(rounds) * (args.step + 0.3) + 2 * args.step + 20)
    # dummy video = no window, dummy audio = the game's music does not play out loud during a test
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy", AUTOPLAY_STEP=str(args.step))
    out_file, err_file = build_dir / "run.out", build_dir / "run.err"
    print(f"playing: rounds={rounds or 'none'} start={args.start or idle_state} (timeout {timeout:.0f}s)\n")
    timed_out = False
    with open(out_file, "w") as out, open(err_file, "w") as err:
        # cwd=root so that files the game loads by relative path (assets/...) are found
        process = subprocess.Popen([str(exe), *rounds], stdout=out, stderr=err, env=env, cwd=str(root))
        try:
            process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            process.kill()   # SIGKILL: SDL turns a normal "please stop" signal into a quit event, which a hung game never reads
            process.wait()
            timed_out = True

    game_output = out_file.read_text(errors="replace")
    log_lines = err_file.read_text(errors="replace").splitlines()

    # ---- the timeline ----
    print("timeline (state changes, key presses, rule violations):")
    seen_states = []
    records = []   # (state name, currentEnemyIndex, which enemies are alive) for every state change
    violations = []
    ended_by_itself = False
    final_state = None
    shown = 0
    for line in log_lines:
        state_match = re.match(r"\[state\] (\d+) (.*)", line)
        final_match = re.match(r"\[final\] (\d+)", line)
        if state_match:
            name = states[int(state_match.group(1))] if int(state_match.group(1)) < len(states) else "?"
            seen_states.append(name)
            idx_match = re.search(r"idx=(\d+) alive=([01]+)", state_match.group(2))
            records.append((name, int(idx_match.group(1)) if idx_match else None, idx_match.group(2) if idx_match else ""))
            line = f"[state] {name:<16} {state_match.group(2)}"
        elif final_match:
            final_state = states[int(final_match.group(1))] if int(final_match.group(1)) < len(states) else "?"
            if not seen_states or seen_states[-1] != final_state:
                seen_states.append(final_state)   # a `break;` can end the loop in a state the per-frame log never saw
                records.append((final_state, None, ""))
            line = f"[final] {final_state} (the state when the game loop ended)"
        elif line.startswith("[VIOLATION]"):
            violations.append(line)
        elif "BY ITSELF" in line:
            ended_by_itself = True
        if shown < 80:
            print("  " + line)
        shown += 1
    if shown > 80:
        print(f"  ... {shown - 80} more lines (full log: {err_file})")

    damage = re.findall(r"Health -?\d+ left\.|DEAD", game_output)
    print("\ndamage events:", " | ".join(damage) if damage else "none")

    # ---- the verdict ----
    completed = sum(1 for previous, current in zip(seen_states, seen_states[1:]) if current == idle_state and previous != idle_state)
    expected = len(rounds) + (1 if args.start and args.start != idle_state else 0)
    game_over = next((s for s in seen_states if s in TERMINAL_STATES), None)

    problems = []
    if ENEMY_TURN_START in states and ENEMY_ATTACKING in states:
        # Every enemy turn: the enemies that were alive at the start must each attack exactly once.
        i = 0
        while i < len(records):
            if records[i][0] != ENEMY_TURN_START:
                i += 1
                continue
            alive_enemies = [n for n, flag in enumerate(records[i][2]) if flag == "1"]
            attackers = []
            j = i + 1
            while j < len(records) and records[j][0] != idle_state and records[j][0] not in TERMINAL_STATES:
                if records[j][0] == ENEMY_ATTACKING:
                    attackers.append(records[j][1])
                j += 1
            finished = j < len(records) and records[j][0] == idle_state
            if finished and sorted(attackers) != alive_enemies:
                problems.append(f"enemy turn: living enemies {alive_enemies} should each attack once, but the attackers were {attackers} "
                                "(is currentEnemyIndex advancing too far, or not at all?)")
            i = j
    if timed_out:
        problems.append("the game hung (had to be killed)")
    if violations:
        problems.append(f"{len(violations)} rule(s) broken (see [VIOLATION] lines above)")
    if ended_by_itself and not game_over:
        problems.append(f"the game loop ended by itself (final state {final_state}) although nobody quit and the game "
                        "was not won or lost - a `break;` inside the main loop?")
    if not game_over and completed < expected:
        problems.append(f"only {completed} of {expected} rounds got back to {idle_state}; "
                        f"the game finished in state {seen_states[-1] if seen_states else '?'}")

    print(f"\nrounds back at {idle_state}: {completed} of {expected}" + (f"   (game ended in {game_over})" if game_over else ""))
    if problems:
        print("\nFAIL")
        for problem in problems:
            print("  - " + problem)
        sys.exit(1)
    print("\nPASS")


if __name__ == "__main__":
    main()
