#pragma once
#include <SDL.h>
#include <string>

#include "GameState.h"

// DualSense (and other gamepad) input. The game's keyboard code is left exactly as it is: a gamepad button is
// turned into the key press the game already understands, so there is only one place that decides what a choice does.
//
//     X (cross)      ->  Enter in the intro, key 1 everywhere else  (confirm / first choice)
//     O (circle)     ->  key 2                                       (second choice)
//     any other button -> a key press that means nothing
//
// SDL calls the cross button "A" and the circle button "B" (it names buttons by their Xbox position).
namespace Input {
    // Opens the gamepads that are already plugged in. Call once after SDL_Init(... | SDL_INIT_GAMECONTROLLER).
    void Init();
    void Quit();

    // Call for every event right after SDL_PollEvent. If the event is a gamepad button it is rewritten, in place,
    // into an SDL_KEYDOWN event. It also opens a gamepad that is plugged in later and closes one that is unplugged.
    // `state` is the current GameState, because X means Enter in the intro and 1 everywhere else.
    void Translate(SDL_Event& event, GameState state);

    // True after the last input was a gamepad button, false after a key press. The screens use it to say
    // "PRESS X" when you play with the gamepad and "PRESS 1" when you play with the keyboard.
    bool UsingGamepad();

    // Names for the on-screen prompts: Choice(1) is "X" or "1", Choice(2) is "O" or "2".
    std::string Choice(int number);
    std::string StartButton();   // "X" or "ENTER"
}
