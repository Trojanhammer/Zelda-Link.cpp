#include "input.h"

#include <algorithm>
#include <vector>

namespace {
    std::vector<SDL_GameController*> controllers;   // the gamepads that are open right now
    bool usingGamepad = false;

    void Open(int deviceIndex) {
        if (!SDL_IsGameController(deviceIndex)) return;
        SDL_GameController* controller = SDL_GameControllerOpen(deviceIndex);   // the same device gives the same pointer back
        if (controller != nullptr && std::find(controllers.begin(), controllers.end(), controller) == controllers.end()) {
            controllers.push_back(controller);
            usingGamepad = true;   // a gamepad is plugged in, so the first screen already says "PRESS X" (a key press switches it back)
        }
    }

    // What a gamepad button does, written as the key that does the same thing.
    SDL_Keycode KeyForButton(Uint8 button, GameState state) {
        if (button == SDL_CONTROLLER_BUTTON_A) return state == Intro ? SDLK_RETURN : SDLK_1;   // X
        if (button == SDL_CONTROLLER_BUTTON_B) return state == Intro ? SDLK_UNKNOWN : SDLK_2;  // O
        return SDLK_UNKNOWN;
    }
}

namespace Input {

void Init() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) Open(i);
}

void Quit() {
    for (SDL_GameController* controller : controllers) SDL_GameControllerClose(controller);
    controllers.clear();
}

void Translate(SDL_Event& event, GameState state) {
    switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            Open(event.cdevice.which);   // for this event `which` is the device index
            break;
        case SDL_CONTROLLERDEVICEREMOVED: {
            SDL_GameController* gone = SDL_GameControllerFromInstanceID(event.cdevice.which);   // here it is the instance id
            if (gone != nullptr) {
                controllers.erase(std::remove(controllers.begin(), controllers.end(), gone), controllers.end());
                SDL_GameControllerClose(gone);
            }
            if (controllers.empty()) usingGamepad = false;
            break;
        }
        case SDL_CONTROLLERBUTTONDOWN: {
            usingGamepad = true;
            SDL_Keycode key = KeyForButton(event.cbutton.button, state);
            event.type = SDL_KEYDOWN;                  // from here on it is an ordinary key press
            event.key.type = SDL_KEYDOWN;
            event.key.state = SDL_PRESSED;
            event.key.repeat = 0;
            event.key.keysym.sym = key;
            break;
        }
        case SDL_KEYDOWN:
            usingGamepad = false;
            break;
        default:
            break;
    }
}

bool UsingGamepad() {
    return usingGamepad;
}

std::string Choice(int number) {
    if (usingGamepad) return number == 1 ? "X" : "O";
    return std::to_string(number);
}

std::string StartButton() {
    return usingGamepad ? "X" : "ENTER";
}

}
