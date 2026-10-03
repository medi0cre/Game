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

typedef enum {
    CameraBoxWidth = 500,
    CameraBoxHeight = 432
} CameraBox;

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
    Camera2D GameCamera;
} Game;

extern Game CurrentGame;

void GameInit(void);
void PlayerInit(void);
void LogMemoryUsed(void);
void LoadAssets(void);
void GetUserInput(void);
void Update(void);
void Render(void);
void LoadLevel(const char* File);
