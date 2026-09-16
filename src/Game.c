#include <Game.h>
#include <Assets.h>
#include <Component.h>
#include <Utils.h>
#include <Entity.h>
#include <System.h>
#include <raylib.h>

#define MaxLineSize 1024
#define ArenaSize 33554432 // 32 Megabytes

Arena GameArena = { 0 };
uint16_t PlayerID = UINT16_MAX;
uint64_t GameFrame = 0;

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
    uint16_t LastCollisionBox = 0;

    uint16_t* TempSpatialArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempMovementArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempGravityArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempTextureArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempAnimationArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    uint16_t* TempCollisionBoxArray = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));

    Enforce(TempSpatialArray && TempMovementArray && TempGravityArray
        && TempTextureArray && TempAnimationArray && TempCollisionBoxArray, "Failed to allocate temporary memory for rendering");

    for (uint16_t i = 0; i < EntityMax; i++)
    {
        if (!Entities[i].Active) { continue; }

        if (Entities[i].ComponentMask & CSpatial) { TempSpatialArray[LastSpatial++] = i; }
        if (Entities[i].ComponentMask & CCollisionBox) { TempCollisionBoxArray[LastCollisionBox++] = i; }
        if (Entities[i].ComponentMask & CMovement) { TempMovementArray[LastMovement++] = i; }
        if (Entities[i].ComponentMask & CGravity) { TempGravityArray[LastGravity++] = i; }
        if (Entities[i].ComponentMask & CAnimation) { TempAnimationArray[LastAnimation++] = i; }
        if (Entities[i].ComponentMask & CTexture) { TempTextureArray[LastTexture++] = i; }
    }

    S_Gravity(TempGravityArray, LastGravity);
    S_Movement(UserInput);
    S_Collision(TempCollisionBoxArray, LastCollisionBox);
    S_Animation(TempAnimationArray, LastAnimation);

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

        uint16_t FrameWidth = AnimationArray[Animations[ID].AnimationID].width / Animations[ID].FramesInAnimation;

        Rectangle Source = {
            .x = FrameWidth * Animations[ID].CurrentFrameInAnimation,
            .y = 0.0f,
            .width = FrameWidth,
            .height = AnimationArray[Animations[ID].AnimationID].height
        };

        Rectangle Destination = {
            .x = Spatials[ID].Position.x,
            .y = Spatials[ID].Position.y,
            .width = Source.width * Spatials[ID].Scale,
            .height = Source.height * Spatials[ID].Scale
        };

        DrawTexturePro(AnimationArray[Animations[ID].AnimationID], Source, Destination, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    ArenaResetToSnapshot(&GameArena);
    EndDrawing();

    GameFrame++;
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

                Entities[Dec].ComponentMask = CSpatial | CTexture;
                Spatials[Dec].Position = (Vector2) { .x = (float)PositionX, .y = (float)PositionY };
                Spatials[Dec].Scale = (float)ScaleNumerator / (float)ScaleDenominator;
                Textures[Dec] = (uint16_t)TextureID;

                break;
            }
            case Tile:
            {
                int TextureID, PositionX, PositionY, ScaleNumerator, ScaleDenominator, CollisionBoxX, CollisionBoxY;

                if (sscanf(Line, "%d, %d, %d, %d, %d, %d, %d, %d",
                    &Type,
                    &TextureID,
                    &PositionX,
                    &PositionY,
                    &ScaleNumerator,
                    &ScaleDenominator,
                    &CollisionBoxX,
                    &CollisionBoxY) != 8)
                {
                    TraceLog(LOG_WARNING, "Failed to load decoration properly");
                    continue;
                }

                uint16_t Tile = CreateEntity();
                Enforce(Tile < EntityMax, "Failed to create entity");

                Entities[Tile].ComponentMask = CSpatial | CTexture | CCollisionBox;
                Spatials[Tile].Position = (Vector2) { .x = (float)PositionX, .y = (float)PositionY };
                Spatials[Tile].Scale = (float)ScaleNumerator / (float)ScaleDenominator;
                Textures[Tile] = (uint16_t)TextureID;
                CollisionBoxes[Tile] = (CollisionBox) { .Width = CollisionBoxX, .Height = CollisionBoxY };

                break;
            }
            case Player:
            {
                int PositionX, PositionY, Scale, Speed, MaxVelocity, Jump, Acceleration, CollisionBoxX, CollisionBoxY;

                if (sscanf(Line, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
                    &Type,
                    &PositionX,
                    &PositionY,
                    &Scale,
                    &Speed,
                    &MaxVelocity,
                    &Jump,
                    &Acceleration,
                    &CollisionBoxX,
                    &CollisionBoxY) == 10)
                {
                    Enforce(PlayerID == UINT16_MAX, "There should never be 2 players, serious bug");
                    PlayerID = CreateEntity();
                    Enforce(PlayerID < EntityMax, "Failed to create player");

                    Entities[PlayerID].ComponentMask = CSpatial | CMovement | CGravity | CAnimation;
                    Spatials[PlayerID].Position = (Vector2) { .x = (float)PositionX, .y = (float)PositionY };
                    Spatials[PlayerID].Scale = (float)Scale;
                    CollisionBoxes[PlayerID] = (CollisionBox) { .Width = CollisionBoxX, .Height = CollisionBoxY };
                    Movements[PlayerID].x = (float)Speed;

                    Gravities[PlayerID] = (Gravity) {
                        .Acceleration = (float)Acceleration * 0.50f,
                        .MaxVelocity = (float)MaxVelocity,
                        .Jump = (float)Jump,
                        .Grounded = false
                    };

                    Animations[PlayerID] = (Animation) {
                        .AnimationID = SamuraiIdle,
                        .CurrentFrameInAnimation = 0,
                        .FramesInAnimation = FrameCountSamuraiIdle,
                        .FramesPassed = 0,
                        .Duration = FrameDurationSamuraiIdle
                    };
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
    Movements = ArenaAlloc(&GameArena, EntityMax * sizeof(Vector2), _Alignof(Vector2));
    Gravities = ArenaAlloc(&GameArena, EntityMax * sizeof(Gravity), _Alignof(Gravity));
    Textures = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    Animations = ArenaAlloc(&GameArena, EntityMax * sizeof(Animation), _Alignof(Animation));
    CollisionBoxes = ArenaAlloc(&GameArena, EntityMax * sizeof(CollisionBox), _Alignof(CollisionBox));

    Enforce(Entities && Spatials && Movements && Gravities
        && Textures && Animations && CollisionBoxes, "Failed to initialize component arrays");

    // Load Assets
    FilePathList FighterFiles = LoadDirectoryFiles(AssetPath "animations/fighter");
    FilePathList SamuraiFiles = LoadDirectoryFiles(AssetPath "animations/samurai");
    FilePathList ShinobiFiles = LoadDirectoryFiles(AssetPath "animations/shinobi");
    FilePathList MiscFiles = LoadDirectoryFiles(AssetPath "animations/misc");

    FilePathList BackgroundFiles = LoadDirectoryFiles(AssetPath "background");
    FilePathList BoxFiles = LoadDirectoryFiles(AssetPath "objects/boxes");
    FilePathList BushFiles = LoadDirectoryFiles(AssetPath "objects/bushes");
    FilePathList FenceFiles = LoadDirectoryFiles(AssetPath "objects/fence");
    FilePathList GrassFiles = LoadDirectoryFiles(AssetPath "objects/grass");
    FilePathList LadderFiles = LoadDirectoryFiles(AssetPath "objects/ladders");
    FilePathList PointerFiles = LoadDirectoryFiles(AssetPath "objects/pointers");
    FilePathList RidgeFiles = LoadDirectoryFiles(AssetPath "objects/ridges");
    FilePathList StoneFiles = LoadDirectoryFiles(AssetPath "objects/stones");
    FilePathList TreeFiles = LoadDirectoryFiles(AssetPath "objects/trees");
    FilePathList WillowFiles = LoadDirectoryFiles(AssetPath "objects/willows");
    FilePathList TileFiles = LoadDirectoryFiles(AssetPath "tiles");

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
