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
        Attack,
        AttackingPhysics,
        ReturningPhysics
    };

    GameState currentState = ChooseWeapon;
    int choice;
    int loop=4;
    int currentEnemyIndex = 0;

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
                        currentState = Attack;
                    }
                    else if(event.key.keysym.sym == SDLK_2 && Enemy2 -> isAlive){
                        MainPlayer -> TargetEnemy = Enemy2.get();
                        currentState = Attack;
                    }
                   
                }
            }

        }

        // Part 2 : Physics 

        if(currentState == Attack){
            MainPlayer -> EquippedWeapon -> Attack();
            MainPlayer -> TargetEnemy -> TakeDamage(MainPlayer -> EquippedWeapon -> damage);
            currentState = AttackingPhysics;
        }
        if(currentState == AttackingPhysics){
            MainPlayer -> Attack();
        }
        
        
        // Part 3 : Rendering
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderClear(renderer);

        if(currentState ==ChooseWeapon){
            // print ayat 
        }
        else if(currentState == ChooseEnemy){
            // print instruction to window
        }
        SDL_RenderPresent(renderer);
    
        









        std::cout << "Choose Weapon to Equip: 1." << MasterSword -> name << ((MasterSword-> durability > 0) ? " " : "BROKEN")
        << " 2. " << OcarinaSword -> name << ((OcarinaSword -> durability > 0) ? " " : "BROKEN");
        // << precedence has higher priority as default so need to have () in ternary operator so this one is evaluted first before the << operator
        
        // Check if the choice gets int
        if (!(std::cin >> choice)){
            std::cin.clear(); // Uncrash the cin operator
            std::cin.ignore(10000,'\n'); // Remove false value
            continue;
        }
        
        if (choice == 1 && MasterSword -> durability > 0){
            MainPlayer -> EquippedWeapon = MasterSword.get(); //.get() is used to get the address of the object that the smart pointer is managing, so that it can be assigned to the raw pointer variable EquippedWeapon in the Player class.
        }
        else if (choice == 2 && OcarinaSword -> durability > 0){
            MainPlayer -> EquippedWeapon = OcarinaSword.get();
        }
        else{
            std::cout << "Invalid choice, choose again";
            continue; //restart the while loop again
        }

        std::cout << "Choose Monster to Attack: 1." << Enemy1 -> name << " " << Enemy1 -> health << " left ." << " 2. " << Enemy2 -> name << " " << Enemy2 -> health << " left .";
        int enemyChoice;
        if(!(std::cin >> enemyChoice)){
            std::cin.clear();
            std::cin.ignore(10000,'\n');
            continue;
        }
        if (enemyChoice==1 && Enemy1 -> isAlive){
            MainPlayer -> TargetEnemy = Enemy1.get();
        }
        else if (enemyChoice==2 && Enemy2 -> isAlive){
            MainPlayer -> TargetEnemy = Enemy2.get();
        }
        else{
            std::cout << "Invalid";
            continue;
        }
        MainPlayer -> EquippedWeapon -> Attack();
        MainPlayer -> TargetEnemy -> TakeDamage(MainPlayer -> EquippedWeapon -> damage);
        MainPlayer -> Attack();
        
        //Enemy Turn

        bool anyEnemyLive=false; // Check if any of enemy still alive
        for (Enemy* e : enemies){
            if(e->isAlive){
                anyEnemyLive=true;
                break;
            }
        }

        if (!anyEnemyLive){
            break; // No living enemy left so break from looping
        }

        while(!(enemies[currentEnemyIndex] -> isAlive)){ // Search for enemy that still alive
            currentEnemyIndex=(currentEnemyIndex+1) % enemies.size();
        }
        MainPlayer -> TakeDamage(enemies[currentEnemyIndex] -> attackPower);
        enemies[currentEnemyIndex] -> Attack();
        currentEnemyIndex=(currentEnemyIndex+1) % enemies.size(); // revert back the turns

        if (!(MainPlayer -> isAlive)){
            break; // Break the loop when player died
        }


    }

    if(!Enemy1->isAlive && !Enemy2 -> isAlive && MainPlayer ->isAlive){
        std::cout << "You WIN";
    }
    else {
        std::cout << "You LOSE";
    }

    std::cout << "Game Over";
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
//When main fx ends, so smart pointers will automatically delete the objects they point to, so no need to manually delete them.
