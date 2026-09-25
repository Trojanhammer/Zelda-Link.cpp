// autoplay_driver.cpp - "presses the keys" for the game, so a whole round can be played
// without a window and without a human. It is only used by tools/autoplay.py, it is not part of the game.
//
// How it works:
//   - tools/autoplay.py compiles a copy of main.cpp with `main` renamed to `game_main`
//   - this file has the real main(). It starts a second thread that pushes key-down events into
//     SDL's event queue (the same thing a real key press does), then runs game_main() on this thread
//
// Usage (normally you run it through tools/autoplay.py instead of by hand):
//   ./autoplay 11 12     each argument is one round: <weapon key><enemy key>
//                        "11" = press 1 (Master Sword) then 1 (first enemy), "22" = 2 then 2, ...

#include <SDL.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int game_main(int argc, char* argv[]);   // main() from main.cpp, renamed at compile time

static const auto startTime = std::chrono::steady_clock::now();
static std::atomic<bool> gameFinished{false};
static std::atomic<bool> quitSent{false};

static double secondsSinceStart() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
}

// Sleep, but wake up early if the game loop has already ended.
static void sleepSeconds(double seconds) {
    const auto end = std::chrono::steady_clock::now() + std::chrono::duration<double>(seconds);
    while (std::chrono::steady_clock::now() < end && !gameFinished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Two threads write to the log (this one and the game). Building the whole line first and writing it in
// one go keeps their lines from getting mixed together.
static void logLine(const std::string& text) {
    std::cerr << (text + "\n");
}

static void pressKey(SDL_Keycode key) {
    SDL_Event event{};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = key;   // SDLK_1 is just the character '1'
    event.key.state = SDL_PRESSED;
    SDL_PushEvent(&event);
    logLine("[driver] t=" + std::to_string(secondsSinceStart()) + " pressed key " + SDL_GetKeyName(key));
}

static void playRounds(std::vector<std::string> rounds, double step) {
    sleepSeconds(step);   // give the game a moment to start up and reach its first menu
    for (const std::string& round : rounds) {
        if (gameFinished) return;
        pressKey(static_cast<SDL_Keycode>(round[0]));   // choose weapon
        sleepSeconds(0.3);
        pressKey(static_cast<SDL_Keycode>(round[1]));   // choose enemy
        sleepSeconds(step);                             // let the whole round play out
    }
    if (gameFinished) return;
    SDL_Event quit{};
    quit.type = SDL_QUIT;
    quitSent = true;
    SDL_PushEvent(&quit);
    logLine("[driver] t=" + std::to_string(secondsSinceStart()) + " sent QUIT");
}

int main(int argc, char* argv[]) {
    double step = 2.0;   // seconds to wait for each round to finish (AUTOPLAY_STEP overrides it)
    if (const char* fromEnv = std::getenv("AUTOPLAY_STEP")) step = std::atof(fromEnv);

    std::vector<std::string> rounds;
    for (int i = 1; i < argc; i++) {
        const std::string round = argv[i];
        if (round.size() != 2 || round[0] < '1' || round[0] > '9' || round[1] < '1' || round[1] > '9') {
            std::cerr << "bad round '" << round << "': expected two digits like 11 (weapon key, enemy key)\n";
            return 2;
        }
        rounds.push_back(round);
    }

    std::cout.setf(std::ios::unitbuf);   // flush the game's output at once, so nothing is lost if it gets killed

    std::thread player(playRounds, rounds, step);
    game_main(1, argv);
    gameFinished = true;
    logLine("[driver] t=" + std::to_string(secondsSinceStart()) + " game loop ended " +
            (quitSent ? "after QUIT" : "BY ITSELF (nobody asked it to stop)"));
    player.join();
    return 0;
}
