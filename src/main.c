#include <raylib.h>
#include <Assets.h>
#include <Utils.h>

#define Width 1280
#define Height 720

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(Width, Height, "Game");

    LoadAssets();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(TextureArray[Tile60], 200, 200, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
