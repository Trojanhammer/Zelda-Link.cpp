#pragma once
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
        EnemyTurn,
        Intro // title video + story text, shown once before level 1
    };