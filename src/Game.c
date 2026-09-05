#include <Game.h>
#include <Assets.h>
#include <Utils.h>
#include <Entity.h>
#include <raylib.h>

Arena GameArena;

Input GetUserInput(void)
{
    return (Input) {
        .Z = IsKeyDown(KEY_Z),
        .X = IsKeyDown(KEY_X),
        .C = IsKeyDown(KEY_C),
        .Up = IsKeyDown(KEY_UP),
        .Down = IsKeyDown(KEY_DOWN),
        .Left = IsKeyDown(KEY_LEFT),
        .Right = IsKeyDown(KEY_RIGHT)
    };
}

void GameInit(void)
{
    // Allocate 32 Megabytes in Arena
    ArenaInit(&GameArena, ArenaSize);

    // Load Assets
    FilePathList FighterFiles = LoadDirectoryFiles("../assets/animations/fighter");
    FilePathList SamuraiFiles = LoadDirectoryFiles("../assets/animations/samurai");
    FilePathList ShinobiFiles = LoadDirectoryFiles("../assets/animations/shinobi");
    FilePathList MiscFiles = LoadDirectoryFiles("../assets/animations/misc");

    FilePathList BackgroundFiles = LoadDirectoryFiles("../assets/background");
    FilePathList BoxFiles = LoadDirectoryFiles("../assets/objects/boxes");
    FilePathList BushFiles = LoadDirectoryFiles("../assets/objects/bushes");
    FilePathList FenceFiles = LoadDirectoryFiles("../assets/objects/fence");
    FilePathList GrassFiles = LoadDirectoryFiles("../assets/objects/grass");
    FilePathList LadderFiles = LoadDirectoryFiles("../assets/objects/ladders");
    FilePathList PointerFiles = LoadDirectoryFiles("../assets/objects/pointers");
    FilePathList RidgeFiles = LoadDirectoryFiles("../assets/objects/ridges");
    FilePathList StoneFiles = LoadDirectoryFiles("../assets/objects/stones");
    FilePathList TreeFiles = LoadDirectoryFiles("../assets/objects/trees");
    FilePathList WillowFiles = LoadDirectoryFiles("../assets/objects/willows");
    FilePathList TileFiles = LoadDirectoryFiles("../assets/tiles");

    Enforce(FighterFiles.count == 10 && SamuraiFiles.count == 10 && ShinobiFiles.count == 10
        && MiscFiles.count == 5 && BackgroundFiles.count == 6 && BoxFiles.count == 6
        && BushFiles.count == 9 && FenceFiles.count == 3 && GrassFiles.count == 10
        && LadderFiles.count == 6 && PointerFiles.count == 8 && RidgeFiles.count == 6
        && StoneFiles.count == 5 && TreeFiles.count == 3 && WillowFiles.count == 3 && TileFiles.count == 60,
        "Incorrect asset file count");

    for (int i = 0; i < AnimationCount; i++)
    {
        if (i < SamuraiAttack1) { AnimationArray[i] = LoadTexture(FighterFiles.paths[i]); }
        else if (i < ShinobiAttack1) { AnimationArray[i] = LoadTexture(SamuraiFiles.paths[i - SamuraiAttack1]); }
        else if (i < Chest) { AnimationArray[i] = LoadTexture(ShinobiFiles.paths[i - ShinobiAttack1]); }
        else { AnimationArray[i] = LoadTexture(MiscFiles.paths[i - Chest]); }
    }

    for (int i = 0; i < TextureCount; i++)
    {
        if (i < Box1) { TextureArray[i] = LoadTexture(BackgroundFiles.paths[i]); }
        else if (i < Bush1) { TextureArray[i] = LoadTexture(BoxFiles.paths[i - Box1]); }
        else if (i < Fence1) { TextureArray[i] = LoadTexture(BushFiles.paths[i - Bush1]); }
        else if (i < Grass1) { TextureArray[i] = LoadTexture(FenceFiles.paths[i - Fence1]); }
        else if (i < Ladder1) { TextureArray[i] = LoadTexture(GrassFiles.paths[i - Grass1]); }
        else if (i < Pointer1) { TextureArray[i] = LoadTexture(LadderFiles.paths[i - Ladder1]); }
        else if (i < Ridge1) { TextureArray[i] = LoadTexture(PointerFiles.paths[i - Pointer1]); }
        else if (i < Stone1) { TextureArray[i] = LoadTexture(RidgeFiles.paths[i - Ridge1]); }
        else if (i < Tree1) { TextureArray[i] = LoadTexture(StoneFiles.paths[i - Stone1]); }
        else if (i < Willow1) { TextureArray[i] = LoadTexture(TreeFiles.paths[i - Tree1]); }
        else if (i < Tile1) { TextureArray[i] = LoadTexture(WillowFiles.paths[i - Willow1]); }
        else { TextureArray[i] = LoadTexture(TileFiles.paths[i - Tile1]); }
    }

    // Initialize Entities
    for (uint16_t i = 0; i < EntityMax; i++)
    {
        Entities[i].Active = false;
        Entities[i].Components = 0;
    }
}
