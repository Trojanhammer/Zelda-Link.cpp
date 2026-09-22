#include "weapon.h"
#include "Bokobolin.h"
#include "Stalfos.h"
#include "Player.h"

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
        AttackingPhysics,
        PlayerTurn,
        EnemyTurn,
        AllEnemyDead,
        CalculateTurn,
        EnemyAttack,
        PlayerDie,
        FirstTurn,
        NextTurn

    };

    GameState currentState = ChooseWeapon;
    int currentEnemyIndex = 0;
    uint8_t turn = 0;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow
    ("Zelda-Link",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800,500,
    SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    bool isRunning = true;
    SDL_Event event;


    std::cout << "Game Start";
    auto MainPlayer= std::make_unique<Player>();
    // smart pointers
    auto MasterSword = std::make_unique<Weapon>("Master Sword",50,2);
    auto OcarinaSword = std::make_unique<Weapon>("Ocarina Sword",30,2);
    auto Enemy1 = std::make_unique<Bokobolin>();
    auto Enemy2 = std::make_unique<Stalfos>();
    Enemy1 -> MainPlayer = MainPlayer.get();
    std::vector<Enemy*> enemies; // make another raw pointers in vector that points towards each enemy pointers
    enemies.push_back(Enemy1.get());
    enemies.push_back(Enemy2.get());
    std::cout << "Your name is Link, you have " << MasterSword -> name << " and " << OcarinaSword -> name << " and you are going to fight against " << Enemy1 -> name << " & " << Enemy2 -> name << " , you have 4 chances to attack them (each weapon can be used 2 times), if you run out of durability, you will lose the game, if you kill both enemies, you will win the game";
    
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
                    if(event.key.keysym.sym == SDLK_1 && Enemy1 -> isAlive){
                        MainPlayer -> TargetEnemy = Enemy1.get();//.get() is used to get the address of the object that the smart pointer is managing, so that it can be assigned to the raw pointer variable EquippedWeapon in the Player class.
                        currentState = DealDamage;
                    }
                    else if(event.key.keysym.sym == SDLK_2 && Enemy2 -> isAlive){
                        MainPlayer -> TargetEnemy = Enemy2.get();
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
            currentState = AttackingPhysics;
            }
            if(currentState == AttackingPhysics){
            MainPlayer -> Attack(); // have both attacking and returning physics
            }
            if (MainPlayer->isKnocking){
            MainPlayer -> TargetEnemy -> KnockBack(MainPlayer -> vX, MainPlayer-> vY);
            MainPlayer -> isKnocking = false;
            MainPlayer -> TargetEnemy -> isKnocked = true;
            }
            if(MainPlayer -> TargetEnemy -> isKnocked)
            MainPlayer -> TargetEnemy -> UpdateKnock();
        
            if (MainPlayer -> posX == MainPlayer -> basePosX && MainPlayer -> posY == MainPlayer -> basePosY){ // When done attack & return
            MainPlayer -> TargetEnemy = nullptr; // ensure the next loop doesnt run Player turn
            currentState = EnemyTurn;
            }
        }
//-------------------------------------------------------------------------------
        // Enemy Turn
        else if (currentState == EnemyTurn){
            bool anyEnemyLive=false; // Check if any of enemy still alive
            
            for (Enemy* e : enemies){
                if(e->isAlive){
                    anyEnemyLive=true;
                    turn++; // How many turns enemy have depends on how many is alive right now
                }
            }

            if (!anyEnemyLive){
                currentState = AllEnemyDead;
                break;
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
                break; // verify this one is efficient or not
            }
        }
        if (currentState == DealDamage){ // Runs not on every frame
            MainPlayer -> TakeDamage(enemies[currentEnemyIndex] -> attackPower);
            currentState = AttackingPhysics;
        }

        if (currentState == AttackingPhysics){ // Runs mostly on every frame
            enemies[currentEnemyIndex] -> Attack();
            if (enemies[currentEnemyIndex] -> posX == MainPlayer -> basePosX && enemies[currentEnemyIndex] -> posY == MainPlayer -> posY){
                currentState=NextTurn;
                turn--;
            }
        }
        // else if (currentState == DecideTurn){
        //     currentEnemyIndex=(currentEnemyIndex+1) % enemies.size(); // revert back the turns
        //     if(turn == 0){
        //         currentState = ChooseWeapon; 
        //         break; // verify this one is efficient or not
        //     }
        // }
            // if (enemies[currentEnemyIndex] -> posX == MainPlayer -> basePosX && enemies[currentEnemyIndex] -> posY == MainPlayer -> posY){
            //     currentState=EnemyTurn;
            // }
        if (!(MainPlayer -> isAlive)){
            currentState = PlayerDie;
            break; // Break the loop when player died
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
