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

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureEx(TextureArray[BGFull], (Vector2) { 0.0f, 0.0f }, 0.0f, 80.0f / 36.0f, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
