#include <iostream>
#include <SDL.h>

int main(int argc , char* argv[]){
    // Call SDL2
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL fail to launch";
        return -1; //means something wrong and int main() will not continue
    }

    // Create Game Window
    SDL_Window* window = SDL_CreateWindow
    ("ZELDA - LINK", // Title
    SDL_WINDOWPOS_CENTERED, // X-pos
    SDL_WINDOWPOS_CENTERED, // Y-pos
    800,500, // width & height
    SDL_WINDOW_SHOWN);

    if (window==nullptr){
        std::cout << "Window failed to create . Error  " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1; //something wrong
    }

    // Launch the renderer(one who draws)
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    bool isRunning = true;
    SDL_Event event; // Initialize event
    while (isRunning) {
        // while (SDL_PollEvent(&event)) { // Get the event from RAM. Give address of event(which stores event obviously) to that fx
        //     if (event.type == SDL_QUIT){ // Check
        //         isRunning = false; // Close 
        //     }
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT){
            isRunning = false;
        }
    SDL_SetRenderDrawColor(renderer, 0,0,230,100);
    SDL_RenderClear(renderer); // Put color to render
    SDL_RenderPresent(renderer); // Show the result
    }

    // Clear memory in RAM before end the game
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();

    return 0; // means successful

}
