#pragma once
#include <stdbool.h>
#include <Arena.h>

#define WindowWidth 1280
#define WindowHeight 720

#define LevelPath "../levels/"
#define AssetPath "../assets/"

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
void GameLoop(void);
Input GetUserInput(void);
void LoadLevel(const char* File);
