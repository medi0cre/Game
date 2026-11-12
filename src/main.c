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
    SetTargetFPS(FPS);

    GameInit();
    if (EditorMode) { LevelEditorInit(); }

    LoadLevel(LevelPath "level1.txt");
    SetTraceLogLevel(LOG_INFO);

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

    SetTraceLogLevel(LOG_WARNING);
    CloseWindow();

    return 0;
}
