#pragma once
#include <stdbool.h>
#include <Arena.h>
#include <World.h>
#include <Assets.h>

#define LevelPath "../levels/"
#define AssetPath "../assets/"

typedef enum {
    WindowWidth = 1280,
    WindowHeight = 720,
    MaxLineSize = 1024,
    ArenaSize = 33554432, // 32 Megabytes
    MaxEntityCount = 10000,
    PlayerSpeed = 8
} GameConfig;

typedef struct {
    bool Z;
    bool X;
    bool C;
    bool Up;
    bool Down;
    bool Left;
    bool Right;
} Input;

typedef struct {
    Texture TextureArray[TextureCount];
    Texture AnimationArray[AnimationCount];
    World GameWorld;
    Arena GameArena;
    uint64_t GameFrame;
    Input UserInput;
    uint16_t PlayerID;
} Game;

extern Game CurrentGame;

void GameInit(void);
void LoadAssets(void);
void GetUserInput(void);
void Update(void);
void Render(void);
void LoadLevel(const char* File);
