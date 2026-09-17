#include <Game.h>
#include <Assets.h>
#include <Component.h>
#include <Utils.h>
#include <Entity.h>
#include <System.h>
#include <raylib.h>

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
        Enforce(ID == PlayerID, "Not a player, need to handle Source.width differently now");

        uint16_t FrameWidth = AnimationArray[Animations[ID].AnimationID].width / Animations[ID].FramesInAnimation;

        Rectangle Source = {
            .x = FrameWidth * Animations[ID].CurrentFrameInAnimation,
            .y = 0.0f,
            .width = Movements[ID].Direction.x * FrameWidth,
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
                int PositionX, PositionY, Scale, MaxVelocity, Jump, Acceleration, CollisionBoxX, CollisionBoxY;

                if (sscanf(Line, "%d, %d, %d, %d, %d, %d, %d, %d, %d",
                    &Type,
                    &PositionX,
                    &PositionY,
                    &Scale,
                    &MaxVelocity,
                    &Jump,
                    &Acceleration,
                    &CollisionBoxX,
                    &CollisionBoxY) != 9)
                {
                    TraceLog(LOG_WARNING, "Failed to load player properly");
                    continue;
                }

                Enforce(PlayerID == UINT16_MAX, "There should never be 2 players, serious bug");
                PlayerID = CreateEntity();
                Enforce(PlayerID < EntityMax, "Failed to create player");

                Entities[PlayerID].ComponentMask = CSpatial | CMovement | CGravity | CAnimation;
                Spatials[PlayerID].Position = (Vector2) { .x = (float)PositionX, .y = (float)PositionY };
                Spatials[PlayerID].Scale = (float)Scale;
                CollisionBoxes[PlayerID] = (CollisionBox) { .Width = CollisionBoxX, .Height = CollisionBoxY };

                Movements[PlayerID] = (Movement) {
                    .Velocity = { 0.0f, 0.0f },
                    .Direction = { 1.0f, 1.0f }
                };

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
    LoadAssets();

    Enforce(ArenaInit(&GameArena, ArenaSize), "Failed to initialize arena");
    Entities = ArenaAlloc(&GameArena, EntityMax * sizeof(Entity), _Alignof(Entity));
    Spatials = ArenaAlloc(&GameArena, EntityMax * sizeof(Spatial), _Alignof(Spatial));
    Movements = ArenaAlloc(&GameArena, EntityMax * sizeof(Movement), _Alignof(Movement));
    Gravities = ArenaAlloc(&GameArena, EntityMax * sizeof(Gravity), _Alignof(Gravity));
    Textures = ArenaAlloc(&GameArena, EntityMax * sizeof(uint16_t), _Alignof(uint16_t));
    Animations = ArenaAlloc(&GameArena, EntityMax * sizeof(Animation), _Alignof(Animation));
    CollisionBoxes = ArenaAlloc(&GameArena, EntityMax * sizeof(CollisionBox), _Alignof(CollisionBox));

    Enforce(Entities && Spatials && Movements && Gravities
        && Textures && Animations && CollisionBoxes, "Failed to initialize component arrays");
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

void LoadAssets(void)
{
    // Load Animations
    AnimationArray[FighterAttack1] = LoadTexture(AssetPath "animations/fighter/Attack_1.png");
    AnimationArray[FighterAttack2] = LoadTexture(AssetPath "animations/fighter/Attack_2.png");
    AnimationArray[FighterAttack3] = LoadTexture(AssetPath "animations/fighter/Attack_3.png");
    AnimationArray[FighterDead] = LoadTexture(AssetPath "animations/fighter/Dead.png");
    AnimationArray[FighterHurt] = LoadTexture(AssetPath "animations/fighter/Hurt.png");
    AnimationArray[FighterIdle] = LoadTexture(AssetPath "animations/fighter/Idle.png");
    AnimationArray[FighterJump] = LoadTexture(AssetPath "animations/fighter/Jump.png");
    AnimationArray[FighterRun] = LoadTexture(AssetPath "animations/fighter/Run.png");
    AnimationArray[FighterShield] = LoadTexture(AssetPath "animations/fighter/Shield.png");
    AnimationArray[FighterWalk] = LoadTexture(AssetPath "animations/fighter/Walk.png");
    AnimationArray[Chest] = LoadTexture(AssetPath "animations/misc/Chest.png");
    AnimationArray[Coin] = LoadTexture(AssetPath "animations/misc/Coin.png");
    AnimationArray[Flag] = LoadTexture(AssetPath "animations/misc/Flag.png");
    AnimationArray[Key] = LoadTexture(AssetPath "animations/misc/Key.png");
    AnimationArray[Rune] = LoadTexture(AssetPath "animations/misc/Rune.png");
    AnimationArray[SamuraiAttack1] = LoadTexture(AssetPath "animations/samurai/Attack_1.png");
    AnimationArray[SamuraiAttack2] = LoadTexture(AssetPath "animations/samurai/Attack_2.png");
    AnimationArray[SamuraiAttack3] = LoadTexture(AssetPath "animations/samurai/Attack_3.png");
    AnimationArray[SamuraiDead] = LoadTexture(AssetPath "animations/samurai/Dead.png");
    AnimationArray[SamuraiHurt] = LoadTexture(AssetPath "animations/samurai/Hurt.png");
    AnimationArray[SamuraiIdle] = LoadTexture(AssetPath "animations/samurai/Idle.png");
    AnimationArray[SamuraiJump] = LoadTexture(AssetPath "animations/samurai/Jump.png");
    AnimationArray[SamuraiRun] = LoadTexture(AssetPath "animations/samurai/Run.png");
    AnimationArray[SamuraiShield] = LoadTexture(AssetPath "animations/samurai/Shield.png");
    AnimationArray[SamuraiWalk] = LoadTexture(AssetPath "animations/samurai/Walk.png");
    AnimationArray[ShinobiAttack1] = LoadTexture(AssetPath "animations/shinobi/Attack_1.png");
    AnimationArray[ShinobiAttack2] = LoadTexture(AssetPath "animations/shinobi/Attack_2.png");
    AnimationArray[ShinobiAttack3] = LoadTexture(AssetPath "animations/shinobi/Attack_3.png");
    AnimationArray[ShinobiDead] = LoadTexture(AssetPath "animations/shinobi/Dead.png");
    AnimationArray[ShinobiHurt] = LoadTexture(AssetPath "animations/shinobi/Hurt.png");
    AnimationArray[ShinobiIdle] = LoadTexture(AssetPath "animations/shinobi/Idle.png");
    AnimationArray[ShinobiJump] = LoadTexture(AssetPath "animations/shinobi/Jump.png");
    AnimationArray[ShinobiRun] = LoadTexture(AssetPath "animations/shinobi/Run.png");
    AnimationArray[ShinobiShield] = LoadTexture(AssetPath "animations/shinobi/Shield.png");
    AnimationArray[ShinobiWalk] = LoadTexture(AssetPath "animations/shinobi/Walk.png");

    // Load Textures
    TextureArray[BGFull] = LoadTexture(AssetPath "background/Background.png");
    TextureArray[BGLayer1] = LoadTexture(AssetPath "background/layer1.png");
    TextureArray[BGLayer2] = LoadTexture(AssetPath "background/layer2.png");
    TextureArray[BGLayer3] = LoadTexture(AssetPath "background/layer3.png");
    TextureArray[BGLayer4] = LoadTexture(AssetPath "background/layer4.png");
    TextureArray[BGLayer5] = LoadTexture(AssetPath "background/layer5.png");
    TextureArray[Box1] = LoadTexture(AssetPath "objects/boxes/1.png");
    TextureArray[Box2] = LoadTexture(AssetPath "objects/boxes/2.png");
    TextureArray[Box3] = LoadTexture(AssetPath "objects/boxes/3.png");
    TextureArray[Box4] = LoadTexture(AssetPath "objects/boxes/4.png");
    TextureArray[Box5] = LoadTexture(AssetPath "objects/boxes/5.png");
    TextureArray[Box6] = LoadTexture(AssetPath "objects/boxes/6.png");
    TextureArray[Bush1] = LoadTexture(AssetPath "objects/bushes/1.png");
    TextureArray[Bush2] = LoadTexture(AssetPath "objects/bushes/2.png");
    TextureArray[Bush3] = LoadTexture(AssetPath "objects/bushes/3.png");
    TextureArray[Bush4] = LoadTexture(AssetPath "objects/bushes/4.png");
    TextureArray[Bush5] = LoadTexture(AssetPath "objects/bushes/5.png");
    TextureArray[Bush6] = LoadTexture(AssetPath "objects/bushes/6.png");
    TextureArray[Bush7] = LoadTexture(AssetPath "objects/bushes/7.png");
    TextureArray[Bush8] = LoadTexture(AssetPath "objects/bushes/8.png");
    TextureArray[Bush9] = LoadTexture(AssetPath "objects/bushes/9.png");
    TextureArray[Fence1] = LoadTexture(AssetPath "objects/fence/1.png");
    TextureArray[Fence2] = LoadTexture(AssetPath "objects/fence/2.png");
    TextureArray[Fence3] = LoadTexture(AssetPath "objects/fence/3.png");
    TextureArray[Grass1] = LoadTexture(AssetPath "objects/grass/01.png");
    TextureArray[Grass2] = LoadTexture(AssetPath "objects/grass/02.png");
    TextureArray[Grass3] = LoadTexture(AssetPath "objects/grass/03.png");
    TextureArray[Grass4] = LoadTexture(AssetPath "objects/grass/04.png");
    TextureArray[Grass5] = LoadTexture(AssetPath "objects/grass/05.png");
    TextureArray[Grass6] = LoadTexture(AssetPath "objects/grass/06.png");
    TextureArray[Grass7] = LoadTexture(AssetPath "objects/grass/07.png");
    TextureArray[Grass8] = LoadTexture(AssetPath "objects/grass/08.png");
    TextureArray[Grass9] = LoadTexture(AssetPath "objects/grass/09.png");
    TextureArray[Grass10] = LoadTexture(AssetPath "objects/grass/10.png");
    TextureArray[Ladder1] = LoadTexture(AssetPath "objects/ladders/1.png");
    TextureArray[Ladder2] = LoadTexture(AssetPath "objects/ladders/2.png");
    TextureArray[Ladder3] = LoadTexture(AssetPath "objects/ladders/3.png");
    TextureArray[Ladder4] = LoadTexture(AssetPath "objects/ladders/4.png");
    TextureArray[Ladder5] = LoadTexture(AssetPath "objects/ladders/5.png");
    TextureArray[Ladder6] = LoadTexture(AssetPath "objects/ladders/6.png");
    TextureArray[Pointer1] = LoadTexture(AssetPath "objects/pointers/1.png");
    TextureArray[Pointer2] = LoadTexture(AssetPath "objects/pointers/2.png");
    TextureArray[Pointer3] = LoadTexture(AssetPath "objects/pointers/3.png");
    TextureArray[Pointer4] = LoadTexture(AssetPath "objects/pointers/4.png");
    TextureArray[Pointer5] = LoadTexture(AssetPath "objects/pointers/5.png");
    TextureArray[Pointer6] = LoadTexture(AssetPath "objects/pointers/6.png");
    TextureArray[Pointer7] = LoadTexture(AssetPath "objects/pointers/7.png");
    TextureArray[Pointer8] = LoadTexture(AssetPath "objects/pointers/8.png");
    TextureArray[Ridge1] = LoadTexture(AssetPath "objects/ridges/1.png");
    TextureArray[Ridge2] = LoadTexture(AssetPath "objects/ridges/2.png");
    TextureArray[Ridge3] = LoadTexture(AssetPath "objects/ridges/3.png");
    TextureArray[Ridge4] = LoadTexture(AssetPath "objects/ridges/4.png");
    TextureArray[Ridge5] = LoadTexture(AssetPath "objects/ridges/5.png");
    TextureArray[Ridge6] = LoadTexture(AssetPath "objects/ridges/6.png");
    TextureArray[Stone1] = LoadTexture(AssetPath "objects/stones/1.png");
    TextureArray[Stone2] = LoadTexture(AssetPath "objects/stones/2.png");
    TextureArray[Stone3] = LoadTexture(AssetPath "objects/stones/3.png");
    TextureArray[Stone4] = LoadTexture(AssetPath "objects/stones/4.png");
    TextureArray[Stone5] = LoadTexture(AssetPath "objects/stones/5.png");
    TextureArray[Tree1] = LoadTexture(AssetPath "objects/trees/1.png");
    TextureArray[Tree2] = LoadTexture(AssetPath "objects/trees/2.png");
    TextureArray[Tree3] = LoadTexture(AssetPath "objects/trees/3.png");
    TextureArray[Willow1] = LoadTexture(AssetPath "objects/willows/1.png");
    TextureArray[Willow2] = LoadTexture(AssetPath "objects/willows/2.png");
    TextureArray[Willow3] = LoadTexture(AssetPath "objects/willows/3.png");
    TextureArray[Tile1] = LoadTexture(AssetPath "tiles/Tile_01.png");
    TextureArray[Tile2] = LoadTexture(AssetPath "tiles/Tile_02.png");
    TextureArray[Tile3] = LoadTexture(AssetPath "tiles/Tile_03.png");
    TextureArray[Tile4] = LoadTexture(AssetPath "tiles/Tile_04.png");
    TextureArray[Tile5] = LoadTexture(AssetPath "tiles/Tile_05.png");
    TextureArray[Tile6] = LoadTexture(AssetPath "tiles/Tile_06.png");
    TextureArray[Tile7] = LoadTexture(AssetPath "tiles/Tile_07.png");
    TextureArray[Tile8] = LoadTexture(AssetPath "tiles/Tile_08.png");
    TextureArray[Tile9] = LoadTexture(AssetPath "tiles/Tile_09.png");
    TextureArray[Tile10] = LoadTexture(AssetPath "tiles/Tile_10.png");
    TextureArray[Tile11] = LoadTexture(AssetPath "tiles/Tile_11.png");
    TextureArray[Tile12] = LoadTexture(AssetPath "tiles/Tile_12.png");
    TextureArray[Tile13] = LoadTexture(AssetPath "tiles/Tile_13.png");
    TextureArray[Tile14] = LoadTexture(AssetPath "tiles/Tile_14.png");
    TextureArray[Tile15] = LoadTexture(AssetPath "tiles/Tile_15.png");
    TextureArray[Tile16] = LoadTexture(AssetPath "tiles/Tile_16.png");
    TextureArray[Tile17] = LoadTexture(AssetPath "tiles/Tile_17.png");
    TextureArray[Tile18] = LoadTexture(AssetPath "tiles/Tile_18.png");
    TextureArray[Tile19] = LoadTexture(AssetPath "tiles/Tile_19.png");
    TextureArray[Tile20] = LoadTexture(AssetPath "tiles/Tile_20.png");
    TextureArray[Tile21] = LoadTexture(AssetPath "tiles/Tile_21.png");
    TextureArray[Tile22] = LoadTexture(AssetPath "tiles/Tile_22.png");
    TextureArray[Tile23] = LoadTexture(AssetPath "tiles/Tile_23.png");
    TextureArray[Tile24] = LoadTexture(AssetPath "tiles/Tile_24.png");
    TextureArray[Tile25] = LoadTexture(AssetPath "tiles/Tile_25.png");
    TextureArray[Tile26] = LoadTexture(AssetPath "tiles/Tile_26.png");
    TextureArray[Tile27] = LoadTexture(AssetPath "tiles/Tile_27.png");
    TextureArray[Tile28] = LoadTexture(AssetPath "tiles/Tile_28.png");
    TextureArray[Tile29] = LoadTexture(AssetPath "tiles/Tile_29.png");
    TextureArray[Tile30] = LoadTexture(AssetPath "tiles/Tile_30.png");
    TextureArray[Tile31] = LoadTexture(AssetPath "tiles/Tile_31.png");
    TextureArray[Tile32] = LoadTexture(AssetPath "tiles/Tile_32.png");
    TextureArray[Tile33] = LoadTexture(AssetPath "tiles/Tile_33.png");
    TextureArray[Tile34] = LoadTexture(AssetPath "tiles/Tile_34.png");
    TextureArray[Tile35] = LoadTexture(AssetPath "tiles/Tile_35.png");
    TextureArray[Tile36] = LoadTexture(AssetPath "tiles/Tile_36.png");
    TextureArray[Tile37] = LoadTexture(AssetPath "tiles/Tile_37.png");
    TextureArray[Tile38] = LoadTexture(AssetPath "tiles/Tile_38.png");
    TextureArray[Tile39] = LoadTexture(AssetPath "tiles/Tile_39.png");
    TextureArray[Tile40] = LoadTexture(AssetPath "tiles/Tile_40.png");
    TextureArray[Tile41] = LoadTexture(AssetPath "tiles/Tile_41.png");
    TextureArray[Tile42] = LoadTexture(AssetPath "tiles/Tile_42.png");
    TextureArray[Tile43] = LoadTexture(AssetPath "tiles/Tile_43.png");
    TextureArray[Tile44] = LoadTexture(AssetPath "tiles/Tile_44.png");
    TextureArray[Tile45] = LoadTexture(AssetPath "tiles/Tile_45.png");
    TextureArray[Tile46] = LoadTexture(AssetPath "tiles/Tile_46.png");
    TextureArray[Tile47] = LoadTexture(AssetPath "tiles/Tile_47.png");
    TextureArray[Tile48] = LoadTexture(AssetPath "tiles/Tile_48.png");
    TextureArray[Tile49] = LoadTexture(AssetPath "tiles/Tile_49.png");
    TextureArray[Tile50] = LoadTexture(AssetPath "tiles/Tile_50.png");
    TextureArray[Tile51] = LoadTexture(AssetPath "tiles/Tile_51.png");
    TextureArray[Tile52] = LoadTexture(AssetPath "tiles/Tile_52.png");
    TextureArray[Tile53] = LoadTexture(AssetPath "tiles/Tile_53.png");
    TextureArray[Tile54] = LoadTexture(AssetPath "tiles/Tile_54.png");
    TextureArray[Tile55] = LoadTexture(AssetPath "tiles/Tile_55.png");
    TextureArray[Tile56] = LoadTexture(AssetPath "tiles/Tile_56.png");
    TextureArray[Tile57] = LoadTexture(AssetPath "tiles/Tile_57.png");
    TextureArray[Tile58] = LoadTexture(AssetPath "tiles/Tile_58.png");
    TextureArray[Tile59] = LoadTexture(AssetPath "tiles/Tile_59.png");
    TextureArray[Tile60] = LoadTexture(AssetPath "tiles/Tile_60.png");
}
