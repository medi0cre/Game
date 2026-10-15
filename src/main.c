#include <raylib.h>
#include <Assets.h>
#include <Utils.h>
#include <Game.h>

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(WindowWidth, WindowHeight, "Game");

    GameInit();
    LoadLevel(LevelPath "level1.txt");

    while (!WindowShouldClose())
    {
        GetUserInput();
        Update();
        Render();
    }

    CloseWindow();
    return 0;
}
