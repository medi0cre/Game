#include <raylib.h>

int main(void)
{
    const int Width = 1280;
    const int Height = 720;

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(Width, Height, "Game");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Game", 200, 200, 20, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
