#pragma once
#include <stdbool.h>
#include <Arena.h>

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

extern Arena GameArena;
extern uint16_t PlayerID;
extern uint64_t GameFrame;

void GameInit(void);
void LoadAssets(void);
void GameLoop(void);
Input GetUserInput(void);
void LoadLevel(const char* File);
