#include <raylib.h>
#include <Assets.h>
#include <Utils.h>
#include <Game.h>
#include <LevelEditor.h>

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(WindowWidth, WindowHeight, "Game");

    GameInit();
    if (EditorMode) { LevelEditorInit(); }
    LoadLevel(LevelPath "level1.txt");

    while (!WindowShouldClose())
    {
        if (EditorMode)
        {
            LevelEditorUpdate();
            LevelEditorRender();
        }
        else
        {
            GameInput();
            GameUpdate();
            GameRender();
        }
    }

    CloseWindow();
    return 0;
}
