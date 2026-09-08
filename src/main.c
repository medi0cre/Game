#include <raylib.h>
#include <Assets.h>
#include <Utils.h>
#include <Game.h>

#define Width 1280
#define Height 720

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(Width, Height, "Game");

    GameInit();
    LoadLevel("../levels/level1.txt");

    while (!WindowShouldClose())
    {
        GameLoop();
    }

    CloseWindow();
    return 0;
}
