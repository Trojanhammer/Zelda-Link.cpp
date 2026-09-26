#include "weapon.h"
#include "Player.h"
#include "Enemy.h"

#include <memory> // Include the memory header for smart pointers
#include <iostream> // Include the iostream header for input/output operations
#include <vector>
#include <cmath>

#include <SDL.h>

// main fx is the entry point of the program
int main(int argc, char* argv[]){
    std::ios_base::sync_with_stdio(false); //Optimize text printing(cout) by disconnecting to old C safety checks
 
    enum GameState {// Enumuration is for readability for users like chooseweapon is read as "0" to computer and "1" for chooseenemy and goes on
        ChooseWeapon, // GameState is datatype
        ChooseEnemy,
        DealDamage,
        PlayerAttacking,
        EnemyAttacking,
        StartEnemyTurn,
        AllEnemyDead,
        PlayerDie,
        FirstTurn,
        NextTurn,
        PlayerRecovered,
        EnemyRecovered
    };

    GameState currentState = ChooseWeapon;
    int currentEnemyIndex = 0;
    unsigned int turn = 0; // can be 0 and (+)

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow
    ("Zelda-Link",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800,500,
    SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // "|" means use both
    // accelerated tells the computer to use its gpu while presentvsync helps to maintain 60 fps since the cpu will wait before gpu sends the frame.

    bool isRunning = true;
    SDL_Event event;

    // Game Start
    auto MainPlayer= std::make_unique<Player>();
    Enemy::MainPlayer = MainPlayer.get(); // set it once after we made player object so every enemy now shares same pointer instead of assign one by one
    // Remove future bugs by discarding the each enemy have their own line of code to player object.it is assigned only in one single code to prevent wrong assignment.
    // smart pointers
    auto MasterSword = std::make_unique<Weapon>("Master Sword",50,2);
    auto OcarinaSword = std::make_unique<Weapon>("Ocarina Sword",30,2);
    auto Bokobolin = std::make_unique<Enemy>(Enemy::EnemyStats{.name = "Bokobolin", .health = 80, .maxHealth = 80, .attackPower = 10, .posX = 100, .posY = 50, .basePosX = 100, .basePosY = 50, .vX = 0, .vY = 0});
    auto Stalfos = std::make_unique<Enemy>(Enemy::EnemyStats{.name = "Stalfos", .health = 80, .maxHealth = 80, .attackPower = 20, .posX = 250, .posY = 160, .basePosX = 250, .basePosY = 160, .vX = 0, .vY = 0});
    
    std::vector<Enemy*> enemies; // make another raw pointers in vector that points towards each enemy pointers
    enemies.push_back(Bokobolin.get());
    enemies.push_back(Stalfos.get());
    std::cout << "Your name is Link, you have " << MasterSword -> name << " and " << OcarinaSword -> name << " and you are going to fight against " << Bokobolin -> name << " & " << Stalfos -> name << " , you have 4 chances to attack them (each weapon can be used 2 times), if you run out of durability, you will lose the game, if you kill both enemies, you will win the game";
    
    while(isRunning){
//-----------------------------------------------------------------------------------------------------------------
        // Part 1: Input Handling 
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                isRunning = false;
            }
            else if(event.type == SDL_KEYDOWN){ // when anything on keyboard is click
                if(currentState == ChooseWeapon){
                    if(event.key.keysym.sym == SDLK_1 && MasterSword -> durability > 0){
                        MainPlayer -> EquippedWeapon = MasterSword.get(); //.get() is used to get the address of the object that the smart pointer is managing, so that it can be assigned to the raw pointer variable EquippedWeapon in the Player class.
                        currentState = ChooseEnemy;
                    }
                    else if(event.key.keysym.sym == SDLK_2 &&OcarinaSword -> durability > 0){
                        MainPlayer -> EquippedWeapon = OcarinaSword.get();
                        currentState = ChooseEnemy;
                    }
                    
                }
                else if(currentState == ChooseEnemy){
                    if(event.key.keysym.sym == SDLK_1 && Bokobolin -> isAlive){
                        MainPlayer -> TargetEnemy = Bokobolin.get();//.get() is used to get the address of the object that the smart pointer is managing, so that it can be assigned to the raw pointer variable EquippedWeapon in the Player class.
                        currentState = DealDamage;
                    }
                    else if(event.key.keysym.sym == SDLK_2 && Stalfos -> isAlive){
                        MainPlayer -> TargetEnemy = Stalfos.get();
                        currentState = DealDamage;
                    }
                   
                }
            }

        }
//-----------------------------------------------------------------------------------------------------------------

        // Checks if player has choose .If not, then skip to render.
        if ((MainPlayer -> TargetEnemy != nullptr) && (MainPlayer -> EquippedWeapon != nullptr)){
            // Part 2 : Physics 

            if(currentState == DealDamage){ // seperate from attack physics because this one only run once since turn based game
            MainPlayer -> EquippedWeapon -> Attack();
            MainPlayer -> TargetEnemy -> TakeDamage(MainPlayer -> EquippedWeapon -> damage);
            currentState = PlayerAttacking;
            }
            if(currentState == PlayerAttacking){
            MainPlayer -> Attack(); // have both attacking and returning physics
            }
            if (MainPlayer->isKnocking){
            MainPlayer -> TargetEnemy -> KnockBack(MainPlayer -> vX, MainPlayer-> vY);
            }
            if(MainPlayer -> TargetEnemy -> isKnocked){
            MainPlayer -> TargetEnemy -> UpdateKnock();
            MainPlayer -> isKnocking = false;
            }
            if (MainPlayer -> posX == MainPlayer -> basePosX && MainPlayer -> posY == MainPlayer -> basePosY){
            currentState = PlayerRecovered;
            }
            
            if ((MainPlayer -> posX == MainPlayer -> basePosX && MainPlayer -> posY == MainPlayer -> basePosY) && (MainPlayer -> TargetEnemy -> posX == MainPlayer -> TargetEnemy -> basePosX && MainPlayer -> TargetEnemy -> posY == MainPlayer -> TargetEnemy -> basePosY)){ // When done attack & return
            MainPlayer -> TargetEnemy = nullptr; // ensure the next loop doesnt run Player turn
            currentState = StartEnemyTurn;
            }
        }
//-------------------------------------------------------------------------------
    // Use "if" for event that happen not on every frame or situations.    
    // Enemy Turn
        else if (currentState == StartEnemyTurn){
            bool anyEnemyLive=false; // Check if any of enemy still alive
            
            for (Enemy* e : enemies){
                if(e->isAlive){
                    anyEnemyLive=true;
                    turn++; // How many turns enemy have depends on how many is alive right now
                }
            }

            if (!anyEnemyLive){
                currentState = AllEnemyDead;
            }
            else{
                currentState = FirstTurn;
            }
        }
        if (currentState == FirstTurn){
            while(!(enemies[currentEnemyIndex] -> isAlive)){ // Determine which enemy first turn to attack.Search for enemy that still alive
                currentEnemyIndex=(currentEnemyIndex+1) % enemies.size();
            }
            currentState = DealDamage;
        }
        if (currentState == NextTurn){
            currentEnemyIndex=(currentEnemyIndex+1) % enemies.size(); // revert back the turns
            currentState = DealDamage;
            if(turn == 0){
                currentState = ChooseWeapon;
            }
        }
        if (currentState == DealDamage){ // Runs not on every frame
            MainPlayer -> TakeDamage(enemies[currentEnemyIndex] -> attackPower);
            currentState = EnemyAttacking;
        }

        if (currentState == EnemyAttacking){ // Runs mostly on every frame
            enemies[currentEnemyIndex] -> Attack();
        }
        if (enemies[currentEnemyIndex] -> isKnocking){
            enemies[currentEnemyIndex] -> MainPlayer -> KnockBack(enemies[currentEnemyIndex] -> vX, enemies[currentEnemyIndex] -> vY);
            enemies[currentEnemyIndex] -> isKnocking = false;
        }

        if(enemies[currentEnemyIndex] -> MainPlayer -> isKnocked){
            enemies[currentEnemyIndex] -> MainPlayer -> UpdateKnock();
        }
        if ((currentState ==EnemyAttacking) && (enemies[currentEnemyIndex] -> posX == enemies[currentEnemyIndex] -> basePosX && enemies[currentEnemyIndex] -> posY == enemies[currentEnemyIndex] -> basePosY)){
            currentState = EnemyRecovered; // ensure enemy dont attack again IF arrived earlier & prevent this condition to run if no buttons is clicked.
        }

        if ((currentState==EnemyRecovered) && (enemies[currentEnemyIndex] -> posX == enemies[currentEnemyIndex] -> basePosX && enemies[currentEnemyIndex] -> posY == enemies[currentEnemyIndex] -> basePosY) && (enemies[currentEnemyIndex] -> MainPlayer -> posX == enemies[currentEnemyIndex] -> MainPlayer-> basePosX && enemies[currentEnemyIndex] -> MainPlayer -> posY == enemies[currentEnemyIndex] -> MainPlayer -> basePosY)){ // When done attack & return
            currentState = NextTurn;
            turn--;
            }

        if (!(MainPlayer -> isAlive)){
            currentState = PlayerDie;
            //break; // Break the loop when player died
        }

    
        
//-----------------------------------------------------------------------------------------------------------------

        // Part 3 : Rendering
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderClear(renderer);

        if(currentState ==ChooseWeapon){
            // print ayat 
        }
        else if(currentState == ChooseEnemy){
            // print instruction to window
        }
        else if (currentState == AllEnemyDead){
            /// print instruction of you win this game
        }
        else if (currentState == PlayerDie){
            /// print instruction of you lose this game
        }
        SDL_RenderPresent(renderer);

    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


//When main fx ends, so smart pointers will automatically delete the objects they point to, so no need to manually delete them.
