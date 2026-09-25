#pragma once
#include <stdbool.h>
#include <Arena.h>
#include <World.h>
#include <Assets.h>

#define LevelPath "../levels/"
#define AssetPath "../assets/"

// Player Bitmasks
#define CanJump UINT16_C(0x1)
#define HasChangedState UINT16_C(0x2)
//#define HasChangedState UINT16_C(0x4)
//#define CAnimation UINT16_C(0x8)
//#define CTexture UINT16_C(0x10)
//#define CCollisionBox UINT16_C(0x20)
//#define Component7 UINT16_C(0x40)
//#define Component8 UINT16_C(0x80)
//#define Component9 UINT16_C(0x100)
//#define Component10 UINT16_C(0x200)
//#define Component11 UINT16_C(0x400)
//#define Component12 UINT16_C(0x800)
//#define Component13 UINT16_C(0x1000)
//#define Component14 UINT16_C(0x2000)
//#define Component15 UINT16_C(0x4000)
//#define Component16 UINT16_C(0x8000)

typedef enum {
    WindowWidth = 1280,
    WindowHeight = 720,
    MaxLineSize = 1024,
    ArenaSize = 33554432, // 32 Megabytes
    MaxEntityCount = 10000,
} GameConfig;

typedef enum {
    PlayerStateIdle = 0,
    PlayerStateRunning = 1,
    PlayerStateJumping = 2
} PlayerState;

typedef enum {
    Fighter = 0,
    Samurai = 1,
    Shinobi = 2
} PlayerCharacter;

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
    Input UserInput;
    uint16_t ID;
    uint16_t State;
    uint16_t Mask;
    uint16_t Jump;
    uint16_t Speed;
    uint16_t Character;
} Player;

typedef struct {
    Texture TextureArray[TextureCount];
    Texture AnimationArray[AnimationCount];
    World GameWorld;
    Arena GameArena;
    uint64_t GameFrame;
    Player GamePlayer;
} Game;

extern Game CurrentGame;

void GameInit(void);
void LoadAssets(void);
void GetUserInput(void);
void Update(void);
void Render(void);
void LoadLevel(const char* File);
