#include "weapon.h"
#include "Player.h"
#include "ui.h"   // text, health bars, prompt bar (ui.cpp)
#include "clip.h" // the intro and the hit cutscenes (clip.cpp)
#include "GameState.h"
#include "levels.h"
#include "screens.h"
#include "input.h"

#include <memory> // Include the memory header for smart pointers
#include <iostream> // Include the iostream header for input/output operations
#include <vector>
#include <cmath>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

int main(int argc, char* argv[]){
    std::ios_base::sync_with_stdio(false); //Optimize text printing(cout) by disconnecting to old C safety checks
    

    GameState currentState = StartLevel; // seeds levelEnemies[0] before anything else runs, same path every later level uses
    int currentEnemyIndex = 0;
    unsigned int turn = 0; // can be 0 and (+)

    u_int8_t currentLevel = 0;
    u_int8_t MaxLevel = 3;
    

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER); // SDL_INIT_VIDEO for window, SDL_INIT_AUDIO for music, SDL_INIT_CONTROLLER for gamepad
    Input::Init(); // use gamepad if available
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

    // Intro + hit cutscenes. The .mp4 files in assets/ are turned into numbered pictures,
    // because SDL2 cannot play video. If assets/frames/ is missing the clips are empty and the game just skips them.
    bool showIntro = true;        // the intro plays once, before level 1
    bool playAttackClips = true;  // a cutscene every time a hit lands
    Clip introClip(renderer, "assets/frames/intro", 12.0f);
    Clip linkHitsBokobolin(renderer, "assets/frames/link-attacks-bokobolin", 12.0f);
    Clip linkHitsStalfos(renderer, "assets/frames/link-attacks-stalfos", 12.0f);
    Clip bokobolinHitsLink(renderer, "assets/frames/bokobolin-attacks-link", 12.0f);
    Clip stalfosHitsLink(renderer, "assets/frames/stalfos-attacks-link", 12.0f);
    Clip* playingClip = nullptr;  // the hit cutscene on screen right now (nullptr = none). The game stands still while it plays.
    std::string clipCaption;

    const float typingSpeed = 40.0f; // letters per second for the story text
    float storyTime = 0;             // seconds since the intro video finished: 1.5 s -> 1.5 * 40 = 60 letters typed so far
    const std::vector<std::string> storyLines = UI::WrapText(
        "YOUR NAME IS LINK. YOU CARRY THE MASTER SWORD AND THE OCARINA SWORD, BUT EACH ONE CAN ONLY STRIKE TWICE. "
        "DEFEAT THE BOKOBOLIN AND THE STALFOS BEFORE YOUR SWORDS BREAK.", 720, 2);

    // Game Start
    auto MainPlayer= std::make_unique<Player>();
    Enemy::MainPlayer = MainPlayer.get(); // set it once after we made player object so every enemy now shares same pointer instead of assign one by one
    // Remove future bugs by discarding the each enemy have their own line of code to player object.it is assigned only in one single code to prevent wrong assignment.
    // smart pointers
    MainPlayer -> WeaponList.push_back(std::make_unique<Weapon>("Master Sword",50,2));
    MainPlayer -> WeaponList.push_back(std::make_unique<Weapon>("Ocarina Sword",30,2));
    Weapon* MasterSword = MainPlayer -> WeaponList[0].get(); // raw pointer to the uniquq_ptr vector
    Weapon* OcarinaSword = MainPlayer -> WeaponList[1].get();
    std::vector<std::vector<std::unique_ptr<Enemy>>> levelEnemies; // store  unique_ptr in 2d vector (dynamic array)
    ScreenView view{renderer, link_texture, bokobolin_texture, stalfos_texture, spriteScale, currentState, currentLevel, MaxLevel, currentEnemyIndex, *MainPlayer, levelEnemies, *MasterSword, *OcarinaSword, playingClip, clipCaption, introClip, storyTime, typingSpeed, storyLines};
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    while(isRunning){
        float deltaTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - lastTime).count();
        lastTime = std::chrono::high_resolution_clock::now();
        if (currentState == AllEnemyDead){ // This condition will become true when one level completed
            if(currentLevel +1 < MaxLevel){
                currentLevel++;
                currentState = StartLevel;
                if(currentLevel ==1){
                    MainPlayer -> Next_Level_Weapon(currentLevel); 
                    MainPlayer -> maxHealth = 150;
                    MainPlayer -> health = MainPlayer -> maxHealth;
                }
                else{
                    MainPlayer -> Next_Level_Weapon(currentLevel); 
                    MainPlayer -> maxHealth = 200;
                    MainPlayer -> health = MainPlayer -> maxHealth;
                }
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
            if(showIntro){ // show intro only once
                showIntro = false;
                introClip.Start();
                currentState = Intro;
            }
        }

//-----------------------------------------------------------------------------------------------------------------
        // Part 1: Input Handling 
        while(SDL_PollEvent(&event)){
            Input::Translate(event, currentState); // convert event clicked by gamepad into keyboard press.
            if (event.type == SDL_QUIT){
                isRunning = false;
            }
            else if(event.type == SDL_KEYDOWN){ // when anything on keyboard is click
                if(currentState == Intro){
                    bool introDone = introClip.Finished() && storyTime * typingSpeed >= UI::CountLetters(storyLines);
                    if(introDone && (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)){
                        currentState = ChooseWeapon;
                    }
                }
                else if(currentState == ChooseWeapon){
                    if(event.key.keysym.sym == SDLK_1 && MasterSword -> durability > 0){
                        MainPlayer -> EquippedWeapon = MasterSword; //.get() is used to get the address of the object that the smart pointer is managing, so that it can be assigned to the raw pointer variable EquippedWeapon in the Player class.
                        currentState = ChooseEnemy;
                    }
                    else if(event.key.keysym.sym == SDLK_2 &&OcarinaSword -> durability > 0){
                        MainPlayer -> EquippedWeapon = OcarinaSword;
                        currentState = ChooseEnemy;
                    }
                    
                }
            
                else if(currentState == ChooseEnemy && MainPlayer -> TargetEnemy == nullptr && MainPlayer -> EquippedWeapon != nullptr){
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

        if(currentState == Intro){
            introClip.Update(deltaTime);
            if(introClip.Finished()){
                storyTime += deltaTime;
            }
        }
        else if(playingClip != nullptr){
            playingClip -> Update(deltaTime);
            if(playingClip -> Finished()){
                playingClip = nullptr;
            }
        }

//-----------------------------------------------------------------------------------------------------------------
        if(playingClip == nullptr){ 
            // Checks if player has choose .If not, then skip to render.
            if ((MainPlayer -> TargetEnemy != nullptr) && (MainPlayer -> EquippedWeapon != nullptr)){
                // Part 2 : Physics 

                if(MainPlayer -> currentState == Player:: DealDamage){ // seperate from attack physics because this one only run once since turn based game
                    MainPlayer -> EquippedWeapon -> Attack(); // to update durability of weapon
                    MainPlayer -> currentState = Player::Attacking;
                }
                if (MainPlayer -> currentState == Player::Attacking){
                    MainPlayer -> Attack(MainPlayer -> TargetEnemy -> posX, MainPlayer -> TargetEnemy -> posY,deltaTime); // Attack is a function that will move the player to the enemy position, and once it arrives, it will change the currentState to Knocking. So we need to check if it has arrived or not in the next frame.
                    // then dalam function attack entity, kalau dah sampai ke enmy,currentstate bertukar jadi knocking
                }
                if(MainPlayer -> currentState == Player::Knocking){
                    MainPlayer -> TargetEnemy -> TakeDamage(MainPlayer -> EquippedWeapon -> damage);
                    MainPlayer -> TargetEnemy -> KnockBack((MainPlayer -> vX / deltaTime), MainPlayer-> vY / deltaTime); // divide by deltaTime to get the original speed of player before it is multiplied by deltaTime in the previous frame
                    MainPlayer -> TargetEnemy -> currentState = Enemy::Knocked; // one-shot, same frame as KnockBack; was in the Recalling branch below, which re-ran every frame and kept stomping the target back to Knocked even after it finished its own recall
                    MainPlayer -> currentState = Player::Recalling_After_Attack; 
                    // hit cutscene
                    if(playAttackClips){
                        playingClip = (MainPlayer -> TargetEnemy == levelEnemies[currentLevel][0].get()) ? &linkHitsBokobolin : &linkHitsStalfos;
                        clipCaption = "LINK STRIKES " + MainPlayer -> TargetEnemy -> name + "!" ;
                        playingClip -> Start();
                    }
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
            if(playAttackClips){
                playingClip = (currentEnemyIndex == 0) ? &bokobolinHitsLink : &stalfosHitsLink;
                clipCaption = levelEnemies[currentLevel][currentEnemyIndex] -> name + " STRIKES LINK!";
                playingClip -> Start();
            }
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
        if(currentState == ChooseWeapon && (MasterSword -> durability ==0) && (OcarinaSword -> durability ==0)){
            currentState = PlayerDie;
        }
        if (!(MainPlayer -> isAlive)){
            currentState = PlayerDie;
        }
    }
        
//-----------------------------------------------------------------------------------------------------------------
        
        // Part 3 : Rendering
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderClear(renderer);
        
        DrawScreens(view); // render everything in one go in screens.cpp
        SDL_RenderPresent(renderer);

    }
    SDL_DestroyTexture(link_texture);
    SDL_DestroyTexture(bokobolin_texture);
    SDL_DestroyTexture(stalfos_texture);
    if(music != nullptr){
        Mix_FreeMusic(music); // freed the music obj
    }
    Mix_CloseAudio();
    introClip.Free();
    linkHitsBokobolin.Free();
    linkHitsStalfos.Free();
    bokobolinHitsLink.Free();
    stalfosHitsLink.Free();
    Input::Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


//When main fx ends, so smart pointers will automatically delete the objects they point to, so no need to manually delete them.
