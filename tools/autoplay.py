#!/usr/bin/env python3
"""
autoplay.py - plays the real game by itself, then tells you what happened.

It never touches your main.cpp. It makes a temporary copy in tools/build/ that
  * prints every game-state change (with the positions of Link and the enemies), and
  * checks a few rules on every frame (listed below),
links that copy with tools/autoplay_driver.cpp (which presses the keys), and runs it with
SDL's "dummy" video driver, so no window opens. Movement is in real seconds now, so a round takes about 6 seconds.

Usage:
    python3 tools/autoplay.py                          one round: weapon 1, enemy 1
    python3 tools/autoplay.py 11 12 22                 three rounds: (1,1) (1,2) (2,2)
    python3 tools/autoplay.py --start StartEnemyTurn   start straight in a state, press nothing
                                                       (handy for testing the enemy turn on its own)
    python3 tools/autoplay.py --root SOME/FOLDER       test another copy of the project
    python3 tools/autoplay.py --intro                  play the intro too (presses Enter until the game starts)
    python3 tools/autoplay.py --clips --step 25        play the hit cutscenes too (about 4 s each, so give each round more time)
    python3 tools/autoplay.py --fps 30 --shots 500     save a picture of the window every 500 ms into tools/build/shots/
    python3 tools/autoplay.py --pad                    press gamepad buttons (X = key 1 / Enter, O = key 2) instead of keys

By default the test copy skips the intro and the hit cutscenes (it sets `showIntro` and `playAttackClips` to false), so a
round is just the physics. The game code is looked up in <root>/src/ and every folder inside it (or in <root> if there is no src folder).

Rules checked on every frame (a broken rule prints [VIOLATION]):
    0. the turn counter never wraps around below 0
       (the "each living enemy attacks exactly once" half of this rule is currently inactive, see
       ENEMY_ATTACKING below, since "attacking" moved from a top-level state to a per-enemy one)
    1. nobody is out of range (flew off the screen, or the position became NaN)
    2. during the player's turn (Link has a TargetEnemy), an enemy that is not the target stays on its base,
       and the target only leaves its base when it has been knocked
    3. outside the player's turn, Link only leaves his base when he has been knocked

Verdict: FAIL if a rule is broken, if the game loop ends by itself, if a scripted round never gets back
to the first state, or if the game hangs. Exit code 0 = pass, 1 = fail, 2 = could not build.

The inserted code expects main.cpp to have variables called currentState, currentEnemyIndex, turn, currentLevel,
MainPlayer, levelEnemies (a vector of vector of unique_ptr<Enemy>, indexed by currentLevel) and renderer. If you rename
them, update INSTRUMENTATION below.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent

# Same flags as the build command in README.md.
CXXFLAGS = ["-std=c++17", "-g", "-O0", "-I/opt/homebrew/include/SDL2", "-D_THREAD_SAFE"]
LDFLAGS = ["-L/opt/homebrew/lib", "-lSDL2main", "-lSDL2", "-lSDL2_image", "-lSDL2_mixer", "-Wl,-framework,Cocoa"]

# Names of two states in your enum, used for the rule "every living enemy attacks exactly once per enemy turn".
# If you rename them, change them here (if they are not in the enum the rule is simply skipped).
# ENEMY_ATTACKING used to be a top-level GameState; since the per-entity state machine refactor, "attacking"
# is Enemy::Attacking on each enemy instead, so this rule no longer has a match and is silently skipped.
ENEMY_TURN_START = "StartEnemyTurn"
ENEMY_ATTACKING = "EnemyAttacking"

# The state a finished round should come back to (the weapon-choice menu). Used instead of main.cpp's literal
# initial currentState value, since that is now StartLevel, a one-shot bootstrap the game passes through once
# and never returns to, not the actual place it waits for input between turns.
MENU_STATE = "ChooseWeapon"

MARKER = "SDL_RenderPresent(renderer);"          # the per-frame instrumentation goes right before this line
AFTER_LOOP_MARKER = "SDL_DestroyRenderer(renderer);"   # first line after the game loop: we log the final state here,
                                                       # because a `break;` skips the per-frame instrumentation
INTRO_SECONDS = 16.0   # how long --intro keeps pressing Enter: the video is 6 s, then the story types for about 4 s, plus some spare
TERMINAL_STATES = {"AllEnemyDead", "PlayerDie"}  # states that are allowed to end the game

INSTRUMENTATION = r'''
    {   // ---- inserted by tools/autoplay.py (this copy only, never your real main.cpp) ----
        static long ap_frame = 0;
        ap_frame++;

        bool ap_haveLevel = currentLevel < levelEnemies.size();   // levelEnemies[currentLevel] only exists once StartLevel has run for it

        // optional frame cap (autoplay.py --fps N): the test copy sleeps so each frame lasts about 1/N seconds.
        // 0 means no cap. This lets us check that physics and recall take the same real time at any fps.
        static const double ap_cap = @FPS@;
        if (ap_cap > 0) {
            static auto ap_prev = std::chrono::steady_clock::now();
            ap_prev += std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1.0 / ap_cap));
            auto ap_now = std::chrono::steady_clock::now();
            if (ap_prev < ap_now) ap_prev = ap_now;   // a frame that ran long: do not try to catch up
            std::this_thread::sleep_until(ap_prev);
        }

        // real seconds since the first frame, in milliseconds, so every log line shows how long things take
        static const auto ap_t0 = std::chrono::steady_clock::now();
        long ap_ms = (long)(std::chrono::duration<double>(std::chrono::steady_clock::now() - ap_t0).count() * 1000);

        // log every time Link or an enemy changes its own state (Attacking, Knocked, Recalling..., Idle) with the time
        static const char* ap_names[] = {"Idle","DealDamage","Attacking","Knocking","Knocked","Recalling_After_Attack","Recalling_After_Knocked"};
        static int ap_lastLink = -1;
        if ((int)MainPlayer->currentState != ap_lastLink) {
            std::cerr << (std::string("[link] ") + ap_names[(int)MainPlayer->currentState] + " t=" + std::to_string(ap_ms) + "ms\n");
            ap_lastLink = (int)MainPlayer->currentState;
        }
        static std::map<Enemy*, int> ap_lastEnemy;
        if (ap_haveLevel)
            for (size_t i = 0; i < levelEnemies[currentLevel].size(); i++) {
                Enemy* e = levelEnemies[currentLevel][i].get();
                auto found = ap_lastEnemy.find(e);
                if (found == ap_lastEnemy.end()) found = ap_lastEnemy.emplace(e, 0).first;   // new enemy starts as Idle
                if ((int)e->currentState != found->second) {
                    std::cerr << (std::string("[e") + std::to_string(i) + "] " + ap_names[(int)e->currentState] + " t=" + std::to_string(ap_ms) + "ms\n");
                    found->second = (int)e->currentState;
                }
            }

        static int ap_lastState = -1;
        if ((int)currentState != ap_lastState) {
            std::ostringstream ap_line;   // build the whole line first, then write it in one go (the key-press thread also logs)
            ap_line << "[state] " << (int)currentState << " t=" << ap_ms << "ms frame=" << ap_frame << " turn=" << (int)turn
                    << " link=(" << MainPlayer->posX << "," << MainPlayer->posY << ")";
            if (ap_haveLevel)
                for (size_t i = 0; i < levelEnemies[currentLevel].size(); i++)
                    ap_line << " e" << i << "=(" << levelEnemies[currentLevel][i]->posX << "," << levelEnemies[currentLevel][i]->posY << ")";
            ap_line << " hp=" << MainPlayer->health << " idx=" << currentEnemyIndex << " alive=";
            if (ap_haveLevel)
                for (const auto& e : levelEnemies[currentLevel]) ap_line << (e->isAlive ? '1' : '0');
            ap_line << "\n";
            std::cerr << ap_line.str();
            ap_lastState = (int)currentState;
        }

        auto ap_home = [](Entity* e) { return e->posX == e->basePosX && e->posY == e->basePosY; };
        auto ap_inRange = [](Entity* e) { return e->posX > -2000 && e->posX < 2000 && e->posY > -2000 && e->posY < 2000; };
        // "knocked" used to be a bool, then one Recalling state; now Recalling is split into
        // Recalling_After_Attack and Recalling_After_Knocked, this only cares about the latter.
        auto ap_knocked = [](Entity* e) { return e->currentState == Entity::Knocked || e->currentState == Entity::Recalling_After_Knocked; };
        static std::set<std::string> ap_seen;   // report each kind of violation once
        auto ap_violation = [&](const char* what) {
            if (ap_seen.insert(what).second)
                std::cerr << (std::string("[VIOLATION] frame=") + std::to_string(ap_frame) + " " + what + "\n");
        };

        if (turn > 100)
            ap_violation("the turn counter wrapped around (it went below 0 - turn-- when turn was already 0?)");
        if (!ap_inRange(MainPlayer.get()))
            ap_violation("Link is out of range (flew off the screen, or his position is NaN)");
        if (ap_haveLevel)
            for (const auto& e : levelEnemies[currentLevel])
                if (!ap_inRange(e.get()))
                    ap_violation("an enemy is out of range (flew off the screen, or its position is NaN)");

        Enemy* ap_target = MainPlayer->TargetEnemy;
        if (ap_target != nullptr && ap_haveLevel) {   // it is the player's turn
            for (const auto& e : levelEnemies[currentLevel]) {
                if (e.get() != ap_target && !ap_home(e.get()))
                    ap_violation("an enemy that is NOT Link's target left its base during the player's turn");
                if (e.get() == ap_target && !ap_home(e.get()) && !ap_knocked(e.get()))
                    ap_violation("Link's target left its base without being knocked (is the enemy attacking during the player's turn?)");
            }
        } else if (ap_target == nullptr && !ap_home(MainPlayer.get()) && !ap_knocked(MainPlayer.get())) {
            ap_violation("Link left his base outside his own turn without being knocked");
        }

        // optional pictures of the window (autoplay.py --shots MS): one every MS milliseconds, named with the time and the state.
        // This runs after everything has been drawn this frame, right before the frame is shown.
        static const char* ap_shotDir = std::getenv("AUTOPLAY_SHOTS_DIR");
        static const long ap_shotEvery = std::getenv("AUTOPLAY_SHOTS_MS") ? std::atol(std::getenv("AUTOPLAY_SHOTS_MS")) : 0;
        static long ap_lastShot = -1000000;
        if (ap_shotDir != nullptr && ap_shotEvery > 0 && ap_ms - ap_lastShot >= ap_shotEvery) {
            ap_lastShot = ap_ms;
            static const char* ap_stateNames[] = {@STATE_NAMES@};
            int ap_w = 0, ap_h = 0;
            SDL_GetRendererOutputSize(renderer, &ap_w, &ap_h);
            SDL_Surface* ap_pic = SDL_CreateRGBSurfaceWithFormat(0, ap_w, ap_h, 32, SDL_PIXELFORMAT_ARGB8888);
            if (ap_pic != nullptr) {
                SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ARGB8888, ap_pic->pixels, ap_pic->pitch);
                char ap_file[64];
                std::snprintf(ap_file, sizeof(ap_file), "/shot_%06ldms_%s.bmp", ap_ms, ap_stateNames[(int)currentState]);
                SDL_SaveBMP(ap_pic, (std::string(ap_shotDir) + ap_file).c_str());
                SDL_FreeSurface(ap_pic);
            }
        }
    }
    '''


def find_enum_text(src, main_text):
    """The text that defines the top-level `enum GameState`: main.cpp itself, or a header main.cpp includes directly
    (the enum may be moved out of main()). Entity.h also has an `enum GameState`, but main.cpp does not include it directly."""
    pattern = r"enum\s+GameState\s*\{"
    if re.search(pattern, main_text):
        return main_text
    for name in re.findall(r'#include\s+"([^"]+)"', main_text):
        header = next(src.rglob(name), None)   # the header may be in any folder under src/ (entities/, sdl/, ...)
        if header is not None and header.is_file() and re.search(pattern, header.read_text()):
            return header.read_text()
    return main_text


def parse_enum(source):
    """Return the names inside `enum GameState { ... }`, in order (so we can print names instead of numbers)."""
    match = re.search(r"enum\s+GameState\s*\{(.*?)\}", source, re.S)
    if not match:
        sys.exit("could not find `enum GameState { ... }` in main.cpp or in a header it includes")
    body = re.sub(r"//[^\n]*", "", match.group(1))   # drop comments
    return [name.strip() for name in body.split(",") if name.strip()]


def has_main(path):
    return re.search(r"\bint\s+main\s*\(", path.read_text()) is not None


def run(command, what):
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"BUILD FAILED while {what}:\n")
        print(result.stderr)
        print("(the inserted code expects main.cpp to have variables named currentState, currentEnemyIndex, turn, currentLevel,"
              " MainPlayer and levelEnemies - if you renamed them, update INSTRUMENTATION in tools/autoplay.py)")
        sys.exit(2)


def build(src, build_dir, source, start_state, states, fps=0.0, intro=False, clips=False):
    text = source
    for marker in (MARKER, AFTER_LOOP_MARKER):
        if text.count(marker) != 1:
            sys.exit(f"expected `{marker}` exactly once in main.cpp (the instrumentation goes right before it)")
    # Vsync would cap the game at 60 fps, so a round takes seconds instead of milliseconds. It does not matter for
    # the game logic, so the test copy runs without it.
    text = re.sub(r"\|\s*SDL_RENDERER_PRESENTVSYNC", "", text)                       # ACCELERATED | PRESENTVSYNC -> ACCELERATED
    text = text.replace("SDL_RENDERER_PRESENTVSYNC", "SDL_RENDERER_ACCELERATED")    # PRESENTVSYNC alone -> ACCELERATED
    # SDL_VIDEODRIVER=dummy has no real GPU surface to accelerate; asking for SDL_RENDERER_ACCELERATED under it
    # hangs on this machine instead of failing cleanly, so the test copy renders in software instead.
    text = text.replace("SDL_RENDERER_ACCELERATED", "SDL_RENDERER_SOFTWARE")
    # The test copy skips the intro and the hit cutscenes unless asked to play them, so a round takes about a second.
    # (If main.cpp has no such variable, nothing happens.)
    for flag, wanted in (("showIntro", intro), ("playAttackClips", clips)):
        text = re.sub(rf"(bool\s+{flag}\s*=\s*)(?:true|false)(\s*;)", rf"\g<1>{'true' if wanted else 'false'}\g<2>", text, count=1)
    instrumentation = (INSTRUMENTATION.replace("@FPS@", repr(float(fps)))
                       .replace("@STATE_NAMES@", ", ".join(f'"{name}"' for name in states)))
    text = text.replace(MARKER, instrumentation + MARKER)
    text = text.replace(AFTER_LOOP_MARKER,
                        'std::cerr << (std::string("[final] ") + std::to_string((int)currentState) + "\\n");\n    ' + AFTER_LOOP_MARKER)
    if start_state:
        if start_state not in states:
            sys.exit(f"--start {start_state}: not a state. States are: {', '.join(states)}")
        text = re.sub(r"(GameState\s+currentState\s*=\s*)\w+(\s*;)", rf"\g<1>{start_state}\g<2>", text, count=1)
    text = ("#include <chrono>\n#include <cstdio>\n#include <cstdlib>\n#include <map>\n#include <set>\n#include <sstream>\n"
            "#include <string>\n#include <thread>\n" + text)

    instrumented = build_dir / "main_instrumented.cpp"
    instrumented.write_text(text)

    main_object = build_dir / "main_instrumented.o"
    include_dirs = [src] + sorted(d for d in src.rglob("*") if d.is_dir())   # src/ and every folder in it, so "Enemy.h" is found wherever it is
    include_flags = [flag for d in include_dirs for flag in ("-I", str(d))]
    run(["clang++", *CXXFLAGS, *include_flags, "-Dmain=game_main", "-Wno-return-type",
         "-c", str(instrumented), "-o", str(main_object)], "compiling the instrumented main.cpp")

    others = [p for p in sorted(src.rglob("*.cpp")) if not has_main(p)]   # every game .cpp (in any folder under src/) except the ones with a main()
    exe = build_dir / "autoplay"
    run(["clang++", *CXXFLAGS, *include_flags, str(TOOLS_DIR / "autoplay_driver.cpp"), str(main_object),
         *map(str, others), *LDFLAGS, "-o", str(exe)], "linking")
    return exe


def main():
    parser = argparse.ArgumentParser(description="Play the game by itself and report what happened.",
                                     formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    parser.add_argument("rounds", nargs="*", help="rounds to play, each <weapon key><enemy key>, e.g. 11 12 22")
    parser.add_argument("--start", metavar="STATE", help="start the game in this state instead of the normal one")
    parser.add_argument("--root", default=str(TOOLS_DIR.parent), help="project folder to test (default: this repo)")
    parser.add_argument("--step", type=float, default=8.0, help="seconds to let each round play out (default 8; a round takes about 6)")
    parser.add_argument("--timeout", type=float, help="kill the game after this many seconds (default: worked out from the rounds)")
    parser.add_argument("--fps", type=float, default=0.0, help="cap the test copy at this many frames a second (default 0 = no cap, runs as fast as it can)")
    parser.add_argument("--intro", action="store_true", help=f"play the intro too (presses Enter for {INTRO_SECONDS:.0f} s before the first round)")
    parser.add_argument("--clips", action="store_true", help="play the hit cutscenes too (about 4 s each: give --step more seconds)")
    parser.add_argument("--pad", action="store_true", help="press gamepad buttons instead of keys (X = key 1 and Enter, O = key 2)")
    parser.add_argument("--shots", type=int, metavar="MS", help="save a picture of the window every MS milliseconds into tools/build/shots/")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    src = root / "src" if (root / "src" / "main.cpp").is_file() else root   # the game code lives in src/ (older layout: next to README)
    source = (src / "main.cpp").read_text()
    states = parse_enum(find_enum_text(src, source))
    if MENU_STATE in states:
        idle_state = MENU_STATE
    else:
        initial = re.search(r"GameState\s+currentState\s*=\s*(\w+)\s*;", source)
        idle_state = initial.group(1) if initial else states[0]   # fallback: main.cpp's literal starting state

    rounds = args.rounds
    if not rounds and not args.start:
        rounds = ["11"]

    build_dir = TOOLS_DIR / "build"
    build_dir.mkdir(exist_ok=True)
    print(f"building a test copy of {src / 'main.cpp'} ...")
    exe = build(src, build_dir, source, args.start, states, args.fps, args.intro, args.clips)

    timeout = args.timeout or (len(rounds) * (args.step + 0.3) + 2 * args.step + 20 + (INTRO_SECONDS if args.intro else 0))
    # dummy video = no window, dummy audio = the game's music does not play out loud during a test
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy", AUTOPLAY_STEP=str(args.step))
    if args.intro:
        env["AUTOPLAY_INTRO_SECONDS"] = str(INTRO_SECONDS)
    if args.pad:
        env["AUTOPLAY_PAD"] = "1"
    shots_dir = build_dir / "shots"
    if args.shots:
        shutil.rmtree(shots_dir, ignore_errors=True)
        shots_dir.mkdir()
        env["AUTOPLAY_SHOTS_DIR"] = str(shots_dir)
        env["AUTOPLAY_SHOTS_MS"] = str(args.shots)
    out_file, err_file = build_dir / "run.out", build_dir / "run.err"
    print(f"playing: rounds={rounds or 'none'} start={args.start or idle_state} fps={args.fps or 'no cap'} (timeout {timeout:.0f}s)\n")
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
    expected = len(rounds) + (1 if args.start and args.start != idle_state else 0) + (1 if args.intro else 0)   # the intro ends by entering the menu once
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
    elif process.returncode != 0:
        # a crash at the very end (for example while cleaning up after the game loop) would not show in the timeline above
        problems.append(f"the game exited with code {process.returncode}" + (f" (signal {-process.returncode}: it crashed)" if process.returncode < 0 else ""))
    if violations:
        problems.append(f"{len(violations)} rule(s) broken (see [VIOLATION] lines above)")
    if ended_by_itself and not game_over:
        problems.append(f"the game loop ended by itself (final state {final_state}) although nobody quit and the game "
                        "was not won or lost - a `break;` inside the main loop?")
    if not game_over and completed < expected:
        problems.append(f"only {completed} of {expected} rounds got back to {idle_state}; "
                        f"the game finished in state {seen_states[-1] if seen_states else '?'}")

    if args.shots:
        print(f"\n{len(list(shots_dir.glob('*.bmp')))} pictures saved in {shots_dir}")
    print(f"\nrounds back at {idle_state}: {completed} of {expected}" + (f"   (game ended in {game_over})" if game_over else ""))
    if problems:
        print("\nFAIL")
        for problem in problems:
            print("  - " + problem)
        sys.exit(1)
    print("\nPASS")


if __name__ == "__main__":
    main()
