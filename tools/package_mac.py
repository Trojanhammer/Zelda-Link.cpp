#!/usr/bin/env python3
"""
package_mac.py - builds the game and wraps it into dist/ZeldaLink.app, a normal Mac app you can double-click.

The game needs SDL2 (and friends), which Homebrew installed in /opt/homebrew. A Mac without Homebrew does not have them,
so this script copies them into the app, next to everything the game loads at run time:

    ZeldaLink.app/Contents/
        MacOS/ZeldaLink       a tiny launcher script (goes into Resources first, because the game loads "assets/..." by
                              a path that starts from the folder it was started in)
        MacOS/game-bin        the game itself, built with -O2
        Frameworks/           the copied libraries (SDL2, SDL3, SDL2_image, SDL2_mixer, ...). Homebrew's "SDL2" is really
                              sdl2-compat: a thin layer that looks for libSDL3.dylib NEXT TO ITSELF when the game starts,
                              so libSDL3 has to be in the same folder, under exactly that name.
        Resources/assets/     the pictures and the clips (assets/frames); the music only with --with-music

Usage:
    python3 tools/package_mac.py                 build dist/ZeldaLink.app (no music inside)
    python3 tools/package_mac.py --with-music    also copy assets/audio/ in (copyrighted: for your own Mac only!)
    open dist/ZeldaLink.app                      run it

The finished app is checked at the end: it is started once with a dummy screen and dummy sound for a few seconds. The check
fails unless (1) libSDL3 was loaded from the app, (2) nothing was loaded from Homebrew (if something was, it is copied in and the
check runs again), and (3) the game answers when it is asked to quit (a game that is stuck on an error window does not).
Only for Apple Silicon Macs like the one it was built on (arm64), and only for the same or a newer macOS than this one.

This is only for learning how packaging works. dist/ is in .gitignore: do not upload the app (it holds Nintendo's
characters and, with --with-music, a copyrighted song).
"""

import argparse
import os
import plistlib
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
DIST = ROOT / "dist"
APP = DIST / "ZeldaLink.app"
MACOS = APP / "Contents" / "MacOS"
FRAMEWORKS = APP / "Contents" / "Frameworks"
RESOURCES = APP / "Contents" / "Resources"

CXXFLAGS = ["-std=c++17", "-O2", "-I/opt/homebrew/include/SDL2", "-D_THREAD_SAFE"]
LDFLAGS = ["-L/opt/homebrew/lib", "-lSDL2main", "-lSDL2", "-lSDL2_image", "-lSDL2_mixer", "-Wl,-framework,Cocoa"]
HOMEBREW = ("/opt/homebrew/", "/usr/local/")
SDL3 = Path("/opt/homebrew/opt/sdl3/lib/libSDL3.0.dylib")   # sdl2-compat opens it by itself, so otool never lists it
SYSTEM = ("/usr/lib/", "/System/", "/Library/")
LAUNCHER = """#!/bin/bash
# The game loads "assets/..." from the folder it is started in, so start it inside Resources.
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE/../Resources" || exit 1
exec "$HERE/game-bin"
"""


def run(command, **kwargs):
    result = subprocess.run(command, capture_output=True, text=True, **kwargs)
    if result.returncode != 0:
        print("FAILED:", " ".join(map(str, command)))
        print(result.stdout + result.stderr)
        sys.exit(1)
    return result.stdout


def build(binary):
    sources = sorted(SRC.rglob("*.cpp"))
    include_dirs = [SRC] + sorted(d for d in SRC.rglob("*") if d.is_dir())
    include_flags = [flag for d in include_dirs for flag in ("-I", str(d))]
    print(f"compiling {len(sources)} files with -O2 ...")
    binary.parent.mkdir(parents=True, exist_ok=True)
    run(["clang++", *CXXFLAGS, *include_flags, *map(str, sources), *LDFLAGS, "-o", str(binary)])


def libraries_of(path):
    """The libraries a file asks for: the names exactly as written inside it (the first line of otool is the file itself)."""
    lines = run(["otool", "-L", str(path)]).splitlines()[1:]
    return [line.strip().split(" (")[0] for line in lines if line.strip()]


def rpaths_of(path):
    out = run(["otool", "-l", str(path)]).splitlines()
    return [out[i + 2].strip().split(" (")[0].replace("path ", "") for i, line in enumerate(out) if "LC_RPATH" in line]


def find_original(reference, owner_original):
    """Where a library really is on disk (None = not ours to copy: a system library)."""
    if reference.startswith(SYSTEM):
        return None
    candidates = []
    if reference.startswith("@rpath/"):
        name = reference[len("@rpath/"):]
        candidates += [Path(r.replace("@loader_path", str(owner_original.parent))) / name for r in rpaths_of(owner_original)]
        candidates += [Path("/opt/homebrew/lib") / name]
    elif reference.startswith("@loader_path/"):
        candidates.append(owner_original.parent / reference[len("@loader_path/"):])
    elif reference.startswith("@executable_path/"):
        return None
    else:
        candidates.append(Path(reference))
    for candidate in candidates:
        if candidate.exists():
            return candidate.resolve()
    sys.exit(f"cannot find {reference} (needed by {owner_original})")


def bundle_libraries(binary_in_app, extra=()):   # extra = [(file to copy, name it gets inside Frameworks/), ...]
    """Copy every non-system library the binary needs (and that those need, and so on) into Frameworks/ and
    point everything at the copies."""
    FRAMEWORKS.mkdir(parents=True, exist_ok=True)
    originals = {binary_in_app: binary_in_app}   # file in the app -> the file it was copied from (for finding its neighbours)
    queue = [binary_in_app]
    for extra_path, extra_name in extra:   # libraries the game opens by itself at run time
        target = FRAMEWORKS / extra_name
        if not target.exists():
            shutil.copy2(extra_path.resolve(), target)
            target.chmod(0o755)
            originals[target] = extra_path.resolve()
            queue.append(target)
    while queue:
        current = queue.pop()
        owner_original = originals[current]
        is_library = current.suffix == ".dylib"
        if is_library:
            run(["install_name_tool", "-id", f"@executable_path/../Frameworks/{current.name}", str(current)])
        for reference in libraries_of(current):
            if reference.startswith("@executable_path/../Frameworks/"):
                continue
            if is_library and Path(reference).name == current.name:
                continue   # a library lists itself first
            original = find_original(reference, owner_original)
            if original is None:
                continue
            name = Path(reference).name
            target = FRAMEWORKS / name
            if not target.exists():
                shutil.copy2(original, target)
                target.chmod(0o755)
                originals[target] = original
                queue.append(target)
            run(["install_name_tool", "-change", reference, f"@executable_path/../Frameworks/{name}", str(current)])


def sign_everything():
    """Apple Silicon refuses to run a file whose signature no longer matches (we just edited them), so sign them again
    with the 'ad hoc' signature (no developer account needed; it only works on this Mac and on Macs that trust local apps)."""
    for library in sorted(FRAMEWORKS.glob("*.dylib")):
        run(["codesign", "--force", "--sign", "-", str(library)])
    run(["codesign", "--force", "--sign", "-", str(MACOS / "game-bin")])
    run(["codesign", "--force", "--sign", "-", str(APP)])


def loaded_libraries(seconds):
    """Start the game inside Resources (as the launcher does), with no screen and no sound, let it run a few seconds,
    then ask it to quit (SIGTERM; SDL turns that into a normal quit request). Returns (every library it loaded,
    whether it quit by itself with exit code 0, everything it printed)."""
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy", DYLD_PRINT_LIBRARIES="1")
    # Into a file, not a pipe: dyld prints about 1400 lines, a full pipe would make the game wait before it even starts.
    log_path = DIST / "check.log"
    with open(log_path, "w") as log_file:
        process = subprocess.Popen([str(MACOS / "game-bin")], cwd=str(RESOURCES), env=env, stdout=log_file, stderr=log_file)
        time.sleep(seconds)
        answered = False
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
                answered = process.returncode == 0
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
    out, err = log_path.read_text(errors="replace"), ""
    log_path.unlink()
    libraries = re.findall(r"dyld\[\d+\]: <[0-9A-F-]+> (\S.*)", out + err)
    return libraries, answered, (out + err)


def main():
    parser = argparse.ArgumentParser(description="Build the game and wrap it into dist/ZeldaLink.app")
    parser.add_argument("--with-music", action="store_true", help="also copy assets/audio/ into the app (copyrighted music: only for your own Mac)")
    args = parser.parse_args()

    if sys.platform != "darwin":
        sys.exit("this script is for macOS")

    shutil.rmtree(APP, ignore_errors=True)
    MACOS.mkdir(parents=True)
    RESOURCES.mkdir(parents=True)

    build(MACOS / "game-bin")

    print("copying the pictures and clips ...")
    for folder in ("compressed", "frames") + (("audio",) if args.with_music else ()):
        source = ROOT / "assets" / folder
        if source.is_dir():
            shutil.copytree(source, RESOURCES / "assets" / folder)
        elif folder != "audio":
            print(f"  warning: assets/{folder} not found, the game will skip it")
    if not args.with_music:
        print("  (no music inside: the game runs silent. Use --with-music for your own copy)")

    (MACOS / "ZeldaLink").write_text(LAUNCHER)
    (MACOS / "ZeldaLink").chmod(0o755)
    (APP / "Contents" / "Info.plist").write_bytes(plistlib.dumps({
        "CFBundleName": "Zelda-Link",
        "CFBundleDisplayName": "Zelda-Link",
        "CFBundleIdentifier": "local.zelda-link",
        "CFBundleExecutable": "ZeldaLink",
        "CFBundlePackageType": "APPL",
        "CFBundleShortVersionString": "1.0",
        "CFBundleVersion": "1",
        "NSHighResolutionCapable": True,
    }))

    print("copying the libraries ...")
    extra = [(SDL3, "libSDL3.dylib")]
    bundle_libraries(MACOS / "game-bin", extra)

    for attempt in range(1, 4):
        sign_everything()
        print(f"check {attempt}: starting the game for 8 s and listing the libraries it loads ...")
        libraries, answered, log = loaded_libraries(8)
        strays = sorted({lib for lib in libraries if lib.startswith(HOMEBREW)})
        if not any("libSDL3" in lib and str(APP) in lib for lib in libraries):
            sys.exit("FAIL: libSDL3 was not loaded from the app, so SDL cannot start (the game would sit behind an error window)")
        if strays:
            print("  loaded from Homebrew instead of the app, copying these in:\n    " + "\n    ".join(strays))
            extra += [(Path(s), Path(s).name) for s in strays]
            bundle_libraries(MACOS / "game-bin", extra)
            continue
        if not answered:
            sys.exit("FAIL: the game did not quit when asked (it crashed, or it is stuck):\n" + "\n".join(l for l in log.splitlines() if not l.startswith("dyld"))[-800:])
        break
    else:
        sys.exit("still loading libraries from Homebrew after 3 tries")

    size = sum(f.stat().st_size for f in APP.rglob("*") if f.is_file()) / 1024 / 1024
    print(f"\nPASS: {APP.relative_to(ROOT)} ({size:.0f} MB, {len(list(FRAMEWORKS.glob('*.dylib')))} libraries inside). "
          f"SDL3 loaded from the app, nothing from Homebrew, and the game quit when asked.")
    print("Run it with:  open dist/ZeldaLink.app")


if __name__ == "__main__":
    main()
