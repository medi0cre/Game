#include <Game.h>
#include <Assets.h>
#include <Component.h>
#include <Utils.h>
#include <Entity.h>
#include <System.h>
#include <raylib.h>

#define MaxLineSize 1024
#define ArenaSize 33554432 // 32 Megabytes

Arena GameArena;
uint16_t PlayerID = UINT16_MAX;

void GameLoop(void)
{
    // Get User Input
    Input UserInput = GetUserInput();

    // Update
    ArenaSnapshot(&GameArena);

    uint16_t LastAnimation = 0;
    uint16_t LastTexture = 0;
    uint16_t LastGravity = 0;
    uint16_t LastMovement = 0;
    uint16_t LastSpatial = 0;

    uint16_t* TempSpatialArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempMovementArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempGravityArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempTextureArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempAnimationArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));

    Enforce(TempSpatialArray && TempMovementArray && TempGravityArray
        && TempTextureArray && TempAnimationArray, "Failed to allocate temporary memory for rendering");

    for (uint16_t i = 0; i < EntityMax; i++)
    {
        if (Entities[i].Active && (Entities[i].Components & CSpatial))
        {
            TempSpatialArray[LastSpatial] = i;
            LastSpatial++;
        }

        if (Entities[i].Active && (Entities[i].Components & CMovement))
        {
            TempMovementArray[LastMovement] = i;
            LastMovement++;
        }

        if (Entities[i].Active && (Entities[i].Components & CGravity))
        {
            TempGravityArray[LastGravity] = i;
            LastGravity++;
        }

        if (Entities[i].Active && (Entities[i].Components & CAnimation))
        {
            TempAnimationArray[LastAnimation] = i;
            LastAnimation++;
        }

        if (Entities[i].Active && (Entities[i].Components & CTexture))
        {
            TempTextureArray[LastTexture] = i;
            LastTexture++;
        }
    }

    S_Movement(UserInput);

    // Render
    BeginDrawing();
    ClearBackground(BLACK);

    for (uint16_t i = 0; i < LastTexture; i++)
    {
        uint16_t ID = TempTextureArray[i];
        Enforce(Entities[ID].Active, "Inactive entity cannot be rendered");
        DrawTextureEx(TextureArray[Textures[ID]], Spatials[ID].Position, 0.0f, Spatials[ID].Scale, WHITE);
    }

    for (uint16_t i = 0; i < LastAnimation; i++)
    {
        uint16_t ID = TempAnimationArray[i];
        Enforce(Entities[ID].Active, "Inactive entity cannot be rendered");
        DrawTextureEx(AnimationArray[Animations[ID]], Spatials[ID].Position, 0.0f, Spatials[ID].Scale, WHITE);
    }

    ArenaResetToSnapshot(&GameArena);
    EndDrawing();
}

void LoadLevel(const char* File)
{
    FILE* Level = fopen(File, "r");
    Enforce(Level, "Failed to load level for the game");

    ArenaSnapshot(&GameArena);
    char* Line = ArenaAlloc(&GameArena, MaxLineSize, _Alignof(char));
    Enforce(Line, "Failed to allocate line buffer");

    while (fgets(Line, MaxLineSize, Level) != NULL)
    {
        // Skip whitespaces, ignore comments and read until null terminator
        while (*Line == ' ' || *Line == '\n' || *Line == '\t' || *Line == '\r') { Line++; }
        if (*Line == '#' || *Line == '\0') { continue; }

        int Type = 0;
        if (sscanf(Line, "%d", &Type) != 1)
        {
            TraceLog(LOG_WARNING, "Failed to load line properly");
            continue;
        }

        switch (Type)
        {
            case Decoration:
            {
                int TextureID, PositionX, PositionY, ScaleNumerator, ScaleDenominator;

                if (sscanf(Line, "%d, %d, %d, %d, %d, %d",
                    &Type,
                    &TextureID,
                    &PositionX,
                    &PositionY,
                    &ScaleNumerator,
                    &ScaleDenominator) != 6)
                {
                    TraceLog(LOG_WARNING, "Failed to load decoration properly");
                    continue;
                }

                uint16_t Dec = CreateEntity();
                Enforce(Dec < EntityMax, "Failed to create entity");

                Entities[Dec].Components = CSpatial | CTexture;
                Spatials[Dec].Position = (Vector2) { .x = (float)PositionX, .y = (float)PositionY };
                Spatials[Dec].Scale = (float)ScaleNumerator / (float)ScaleDenominator;
                Textures[Dec] = (uint16_t)TextureID;

                break;
            }
            case Tile:
            {
                break;
            }
            case Player:
            {
                int AnimationID, PositionX, PositionY, Scale, Speed, Jump, Gravity;

                if (sscanf(Line, "%d, %d, %d, %d, %d, %d, %d, %d",
                    &Type,
                    &AnimationID,
                    &PositionX,
                    &PositionY,
                    &Scale,
                    &Speed,
                    &Jump,
                    &Gravity) == 8)
                {
                    PlayerID = CreateEntity();
                    Enforce(PlayerID < EntityMax, "Failed to create player");

                    Entities[PlayerID].Components = CSpatial | CMovement | CGravity | CAnimation;
                    Spatials[PlayerID].Position = (Vector2) { .x = (float)PositionX, .y = (float)PositionY };
                    Spatials[PlayerID].Scale = (float)Scale;
                    Movements[PlayerID].Velocity = (float)Speed;
                    Movements[PlayerID].Gravity = (float)Gravity;
                    Movements[PlayerID].Jump = (float)Jump;
                    Animations[PlayerID] = (uint16_t)AnimationID;
                }

                break;
            }
            default:
            {
                Enforce(false, "Unknown entity type encountered");
                return;
            }
        }
    }

    ArenaResetToSnapshot(&GameArena);
    fclose(Level);
}

void GameInit(void)
{
    // Allocate Resources
    Enforce(ArenaInit(&GameArena, ArenaSize), "Failed to initialize arena");
    Entities = ArenaAlloc(&GameArena, EntityMax * sizeof(Entity), _Alignof(Entity));
    Spatials = ArenaAlloc(&GameArena, EntityMax * sizeof(Spatial), _Alignof(Spatial));
    Movements = ArenaAlloc(&GameArena, EntityMax * sizeof(Movement), _Alignof(Movement));
    Gravities = ArenaAlloc(&GameArena, EntityMax * sizeof(float), _Alignof(float));
    Textures = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    Animations = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    Enforce(Entities && Spatials && Movements && Gravities && Textures && Animations, "Failed to initialize component arrays");

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

    for (uint16_t i = 0; i < AnimationCount; i++)
    {
        if (i < SamuraiAttack1) { AnimationArray[i] = LoadTexture(FighterFiles.paths[i]); }
        else if (i < ShinobiAttack1) { AnimationArray[i] = LoadTexture(SamuraiFiles.paths[i - SamuraiAttack1]); }
        else if (i < Chest) { AnimationArray[i] = LoadTexture(ShinobiFiles.paths[i - ShinobiAttack1]); }
        else { AnimationArray[i] = LoadTexture(MiscFiles.paths[i - Chest]); }
    }

    for (uint16_t i = 0; i < TextureCount; i++)
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
}

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
