#include "weapon.h"
#include "Player.h"
#include "Enemy.h"

#include <memory> // Include the memory header for smart pointers
#include <iostream> // Include the iostream header for input/output operations
#include <vector>
#include <cmath>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

void DrawSprite(SDL_Renderer* renderer,SDL_Texture* texture, float centerX, float centerY ,float scale){
    if (texture == nullptr) return; // check image is exist, use return so dont crash the game
    int w = 0;
    int h = 0;

    // Ask texture(image) about its size
    SDL_QueryTexture(texture,nullptr,nullptr, &w, &h);
    SDL_Rect dst; // destination
    dst.w = (int)(w * scale);
    dst.h = (int)(h * scale);
    dst.x = (int)(centerX - dst.w /2); // Change value of X .Ke kiri sikit
    dst.y = (int)(centerY - dst.h /2); // Ke atas sikit

    SDL_RenderCopy(renderer, texture,nullptr, &dst);
}

// main fx is the entry point of the program
int main(int argc, char* argv[]){
    std::ios_base::sync_with_stdio(false); //Optimize text printing(cout) by disconnecting to old C safety checks
    enum GameState {// Enumuration is for readability for users like chooseweapon is read as "0" to computer and "1" for chooseenemy and goes on
        ChooseWeapon, // GameState is datatype
        ChooseEnemy,
        StartEnemyTurn,
        AllEnemyDead,
        PlayerDie,
        FirstTurn,
        NextTurn,
        CompleteGame,
        StartLevel,
        EnemyTurn
    };

    GameState currentState = StartLevel; // seeds levelEnemies[0] before anything else runs, same path every later level uses
    int currentEnemyIndex = 0;
    unsigned int turn = 0; // can be 0 and (+)

    u_int8_t currentLevel = 0;
    u_int8_t MaxLevel = 3;
    

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* window = SDL_CreateWindow
    ("Zelda-Link",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800,500,
    SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // "|" means use both
    // accelerated tells the computer to use its gpu while presentvsync helps to maintain 60 fps since the cpu will wait before gpu sends the frame.
    
    // Put image to texture
    SDL_Texture* link_texture = IMG_LoadTexture(renderer, "assets/compressed/link.png");
    SDL_Texture* bokobolin_texture = IMG_LoadTexture(renderer, "assets/compressed/bokobolin.png");
    SDL_Texture* stalfos_texture = IMG_LoadTexture(renderer, "assets/compressed/stalfos.png");
    const float spriteScale[3] = {1.0f,1.25f,1.5f};

    // Put and play music
    Mix_Music* music = nullptr;
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT,2,2048)==0){
        music = Mix_LoadMUS("assets/audio/wiiu-shop.mp3");
        if(music != nullptr){
            Mix_PlayMusic(music,-1); // -1 to play infinitely
        }
    }

    bool isRunning = true;
    SDL_Event event;

    // Game Start
    auto MainPlayer= std::make_unique<Player>();
    Enemy::MainPlayer = MainPlayer.get(); // set it once after we made player object so every enemy now shares same pointer instead of assign one by one
    // Remove future bugs by discarding the each enemy have their own line of code to player object.it is assigned only in one single code to prevent wrong assignment.
    // smart pointers
    auto MasterSword = std::make_unique<Weapon>("Master Sword",50,2);
    auto OcarinaSword = std::make_unique<Weapon>("Ocarina Sword",30,2);
    std::vector<std::vector<std::unique_ptr<Enemy>>> levelEnemies; // store  unique_ptr in 2d vector (dynamic array)

    struct BlueprintLevel {
            Enemy::EnemyStats BokoData;
            Enemy::EnemyStats StalfosData;
        };

    const std::vector<BlueprintLevel> LevelStats {
        {   // level_0
            {.name = "Bokobolin", .health = 80, .maxHealth = 80, .attackPower = 10, .posX = 150, .posY = 100, .basePosX = 150, .basePosY = 100, .vX = 0, .vY = 0},
            {.name = "Stalfos", .health = 80, .maxHealth = 80, .attackPower = 20, .posX = 550, .posY = 160, .basePosX = 550, .basePosY = 160, .vX = 0, .vY = 0}
        },
        {   // level_1
            {.name = "Bokobolin", .health = 80, .maxHealth = 80, .attackPower = 10, .posX = 250, .posY = 200, .basePosX = 250, .basePosY = 200, .vX = 0, .vY = 0},
            {.name = "Stalfos", .health = 80, .maxHealth = 80, .attackPower = 20, .posX = 600, .posY = 240, .basePosX = 600, .basePosY = 240, .vX = 0, .vY = 0},
        },
        {   // level_2
            {.name = "Bokobolin", .health = 110, .maxHealth = 110, .attackPower = 25, .posX = 300, .posY = 200, .basePosX = 300, .basePosY = 200, .vX = 0, .vY = 0},
            {.name = "Stalfos", .health = 120, .maxHealth = 120, .attackPower = 35, .posX = 350, .posY = 240, .basePosX = 350, .basePosY = 240, .vX = 0, .vY = 0}
        }
    };

    //std::cout << "\nYour name is Link, you have " << MasterSword -> name << " and " << OcarinaSword -> name << " and you are going to fight against " << Bokobolin_0 -> name << " & " << Stalfos_0 -> name << " , you have 4 chances to attack them (each weapon can be used 2 times), if you run out of durability, you will lose the game, if you kill both enemies, you will win the game" << std::endl;
    auto lastTime = std::chrono::high_resolution_clock::now();
    while(isRunning){
        float deltaTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - lastTime).count();
        lastTime = std::chrono::high_resolution_clock::now();
        if (currentState == AllEnemyDead){ // This condition will become true when one level completed
            if(currentLevel +1 < MaxLevel){
                currentLevel++;
                currentState = StartLevel;
            }
            else {
                currentState = CompleteGame;
            }
        }
        if (currentState == StartLevel){ // To prevent game from loading the all enemy object all at once.
            std::vector<std::unique_ptr<Enemy>> e; // create vector that has unique_ptr
            e.push_back(std::make_unique<Enemy>(LevelStats[currentLevel].BokoData));
            e.push_back(std::make_unique<Enemy>(LevelStats[currentLevel].StalfosData));

            levelEnemies.push_back(std::move(e)); // transfer the unique pointer task to levelEnemies;
            currentState = ChooseWeapon;
        }

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
                        if(event.key.keysym.sym == SDLK_1 && levelEnemies[currentLevel][0] -> isAlive){
                            MainPlayer -> TargetEnemy = levelEnemies[currentLevel][0].get();//.get() is used to get the address of the object that the smart pointer is managing, so that it can be assigned to the raw pointer variable EquippedWeapon in the Player class.
                            MainPlayer-> currentState = Player::DealDamage;
                        }
                        else if(event.key.keysym.sym == SDLK_2 && levelEnemies[currentLevel][1] -> isAlive){
                            MainPlayer -> TargetEnemy = levelEnemies[currentLevel][1].get();
                            MainPlayer -> currentState = Player::DealDamage;
                        }
                    }
                }
        }

//-----------------------------------------------------------------------------------------------------------------

        // Checks if player has choose .If not, then skip to render.
        if ((MainPlayer -> TargetEnemy != nullptr) && (MainPlayer -> EquippedWeapon != nullptr)){
            // Part 2 : Physics 

            if(MainPlayer -> currentState == Player:: DealDamage){ // seperate from attack physics because this one only run once since turn based game
                MainPlayer -> EquippedWeapon -> Attack(); // to update durability of weapon
                MainPlayer -> TargetEnemy -> TakeDamage(MainPlayer -> EquippedWeapon -> damage);
                MainPlayer -> currentState = Player::Attacking;
            }
            if (MainPlayer -> currentState == Player::Attacking){
                MainPlayer -> Attack(MainPlayer -> TargetEnemy -> posX, MainPlayer -> TargetEnemy -> posY,deltaTime); // Attack is a function that will move the player to the enemy position, and once it arrives, it will change the currentState to Knocking. So we need to check if it has arrived or not in the next frame.
                // then dalam function attack entity, kalau dah sampai ke enmy,currentstate bertukar jadi knocking
            }
            if(MainPlayer -> currentState == Player::Knocking){
                MainPlayer -> TargetEnemy -> KnockBack((MainPlayer -> vX / deltaTime), MainPlayer-> vY / deltaTime); // divide by deltaTime to get the original speed of player before it is multiplied by deltaTime in the previous frame
                MainPlayer -> TargetEnemy -> currentState = Enemy::Knocked; // one-shot, same frame as KnockBack; was in the Recalling branch below, which re-ran every frame and kept stomping the target back to Knocked even after it finished its own recall
                MainPlayer -> currentState = Player::Recalling_After_Attack; 
            }
            else if(MainPlayer -> currentState == Player::Recalling_After_Attack){
                MainPlayer-> Return(MainPlayer -> Recall_After_Attack,deltaTime);
            }
            if (MainPlayer -> TargetEnemy -> currentState == Enemy::Knocked){
                MainPlayer -> TargetEnemy -> UpdateKnock(deltaTime);
            }
            else if(MainPlayer -> TargetEnemy -> currentState == Enemy::Recalling_After_Knocked){
                MainPlayer -> TargetEnemy -> Return(MainPlayer -> TargetEnemy -> Recall_After_Knocked,deltaTime);
            }
            if ((MainPlayer -> currentState == Player::Idle) && (MainPlayer -> TargetEnemy -> currentState == Enemy::Idle)){
                MainPlayer -> TargetEnemy = nullptr; // ensure the next loop doesnt run Player turn
                currentState = StartEnemyTurn;      
            }
        }
    // Use "if" for event that happen not on every frame or situations.    
    // Enemy Turn
        else if (currentState == StartEnemyTurn){
            bool anyEnemyLive=false; // Check if any of enemy still alive
            for (const auto& e : levelEnemies[currentLevel]){ // make a reference(store address) for that unique pointer
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
        if (currentState == FirstTurn){ // Runs not on every frame
            while(!(levelEnemies[currentLevel][currentEnemyIndex] -> isAlive)){ // Determine which enemy first turn to attack.Search for enemy that still alive
                currentEnemyIndex=(currentEnemyIndex+1) % levelEnemies[currentLevel].size();
            }
            levelEnemies[currentLevel][currentEnemyIndex] -> currentState = Enemy::DealDamage;
            currentState = EnemyTurn;
            
        }
        if (currentState == NextTurn){ // Runs not on every frame
            if(turn == 0){
                currentState = ChooseWeapon;
            }
            else{
                currentEnemyIndex=(currentEnemyIndex+1) % levelEnemies[currentLevel].size(); // revert back the turns
                levelEnemies[currentLevel][currentEnemyIndex] -> currentState = Enemy::DealDamage;
                currentState = EnemyTurn;
            }
        }
        if (levelEnemies[currentLevel][currentEnemyIndex]-> currentState == Enemy::DealDamage){ // Runs not on every frame
            MainPlayer -> TakeDamage(levelEnemies[currentLevel][currentEnemyIndex] -> attackPower);
            levelEnemies[currentLevel][currentEnemyIndex] -> currentState = Enemy::Attacking;
        }

        if (levelEnemies[currentLevel][currentEnemyIndex] -> currentState == Enemy::Attacking){ // Runs mostly on every frame
            levelEnemies[currentLevel][currentEnemyIndex] -> Attack(MainPlayer -> posX , MainPlayer -> posY,deltaTime);
        }
        if (levelEnemies[currentLevel][currentEnemyIndex] -> currentState == Enemy::Knocking){
            levelEnemies[currentLevel][currentEnemyIndex] -> MainPlayer -> KnockBack(levelEnemies[currentLevel][currentEnemyIndex] -> vX / deltaTime, levelEnemies[currentLevel][currentEnemyIndex] -> vY / deltaTime); // same recovery as the player's side: vX/vY are this frame's displacement, divide by deltaTime to get the real speed
            MainPlayer -> currentState = Player::Knocked; // one-shot, same frame as KnockBack; was in the Recalling branch below
            levelEnemies[currentLevel][currentEnemyIndex] -> currentState = Enemy::Recalling_After_Attack;
        }
        else if(levelEnemies[currentLevel][currentEnemyIndex] -> currentState == Enemy::Recalling_After_Attack){ // prevent this condition to run when player hits enemy
            levelEnemies[currentLevel][currentEnemyIndex] -> Return(levelEnemies[currentLevel][currentEnemyIndex] -> Recall_After_Attack,deltaTime);
        }
        if(MainPlayer -> currentState == Player::Knocked){
            MainPlayer -> UpdateKnock(deltaTime);
        }
        else if(MainPlayer -> currentState == Player::Recalling_After_Knocked){
            MainPlayer -> Return(MainPlayer -> Recall_After_Knocked,deltaTime);
        }
        if ((currentState ==EnemyTurn) && (MainPlayer -> currentState == Player::Idle) && (levelEnemies[currentLevel][currentEnemyIndex] -> currentState == Enemy::Idle)){
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
        
        // SDL2 lukis X coord and Y lain.tapi physics in game tetap attack ke posX,posY asal.
        DrawSprite(renderer,link_texture,MainPlayer -> posX, MainPlayer -> posY,spriteScale[currentLevel]);
        DrawSprite(renderer,bokobolin_texture,levelEnemies[currentLevel][0] -> posX, levelEnemies[currentLevel][0] -> posY,spriteScale[currentLevel]);
        DrawSprite(renderer,stalfos_texture,levelEnemies[currentLevel][1] -> posX, levelEnemies[currentLevel][1] -> posY,spriteScale[currentLevel]);

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
        else if (currentState == CompleteGame){

        }
        SDL_RenderPresent(renderer);

    }
    SDL_DestroyTexture(link_texture);
    SDL_DestroyTexture(bokobolin_texture);
    SDL_DestroyTexture(stalfos_texture);
    if(music != nullptr){
        Mix_FreeMusic(music); // freed the music obj
    }
    Mix_CloseAudio();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


//When main fx ends, so smart pointers will automatically delete the objects they point to, so no need to manually delete them.
