#include <raylib.h>
#include <Assets.h>
#include <Utils.h>

#define Width 1280
#define Height 720

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(Width, Height, "Game");

    FilePathList FighterFiles = LoadDirectoryFiles("../assets/characters/fighter");
    FilePathList SamuraiFiles = LoadDirectoryFiles("../assets/characters/samurai");
    FilePathList ShinobiFiles = LoadDirectoryFiles("../assets/characters/shinobi");
    Enforce(FighterFiles.count == 10 && SamuraiFiles.count == 10 && ShinobiFiles.count == 10, "Incorrect animation file count");

    for (int i = 0; i < AnimationCount; i++)
    {
        if (i < 10) { AnimationArray[i] = LoadTexture(FighterFiles.paths[i]); }
        else if (i < 20) { AnimationArray[i] = LoadTexture(SamuraiFiles.paths[i - 10]); }
        else { AnimationArray[i] = LoadTexture(ShinobiFiles.paths[i - 20]); }
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(AnimationArray[SamuraiWalk], 200, 200, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
