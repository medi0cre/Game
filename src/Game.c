#include <Game.h>
#include <Assets.h>
#include <Component.h>
#include <Utils.h>
#include <Entity.h>
#include <System.h>
#include <raylib.h>

Game CurrentGame = { 0 };

void Update(void)
{
    World* W = &CurrentGame.GameWorld;

    uint16_t LastGravity = 0;
    uint16_t LastTile = 0;
    uint16_t LastEnemy = 0;

    // Declare these in the World struct since they are needed for rendering
    W->LastAnimation = 0;
    W->LastTexture = 0;

    for (uint16_t i = 0; i < MaxEntityCount; i++)
    {
        if (!W->Actives[i]) { continue; }
        uint64_t Component = W->Components[i];
        Enforce(Component != 0, "Useless entity");

        if (Component & CGravity) { W->TempGravityArray[LastGravity++] = i; }
        if (Component & CAnimation) { W->TempAnimationArray[W->LastAnimation++] = i; }
        if (Component & CTexture) { W->TempTextureArray[W->LastTexture++] = i; }

        if (Component == CTile) { W->TempTileArray[LastTile++] = i; }
        if (Component == CEnemy) { W->TempEnemyArray[LastEnemy++] = i; }
    }

    uint16_t PreviousState = CurrentGame.GamePlayer.State;
    uint16_t* State = &CurrentGame.GamePlayer.State;
    uint16_t* Mask = &CurrentGame.GamePlayer.Mask;

    S_Gravity(LastGravity);
    S_MovementX();
    S_CollisionX(LastTile, LastEnemy);
    S_MovementY(LastEnemy);
    S_CollisionY(LastTile, LastEnemy);
    S_Camera();

    if (*State != PreviousState) { *Mask |= HasChangedState; }
    S_Animation();

    CurrentGame.GameFrame++;
}

void Render(void)
{
    World* W = &CurrentGame.GameWorld;

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(CurrentGame.GameCamera);

    for (uint16_t i = 0; i < W->LastTexture; i++)
    {
        uint16_t ID = W->TempTextureArray[i];
        Enforce(ID < MaxEntityCount
            && W->Textures[ID] < TextureCount, "Invalid texture");

        DrawTextureEx(CurrentGame.TextureArray[W->Textures[ID]],
            W->Spatials[ID].Position, 0.0f, W->Spatials[ID].Scale, WHITE);

        // Debug Lines
        if (W->Components[ID] & CCollisionBox)
        {
            Rectangle TextureRectangle = (Rectangle) {
                .x = W->Spatials[ID].Position.x,
                .y = W->Spatials[ID].Position.y,
                .width = (float)W->CollisionBoxes[ID].Width,
                .height = (float)W->CollisionBoxes[ID].Height
            };

            DrawRectangleLinesEx(TextureRectangle, 1.0f, RED);
        }
    }

    for (uint16_t i = 0; i < W->LastAnimation; i++)
    {
        uint16_t ID = W->TempAnimationArray[i];
        Enforce(ID < MaxEntityCount, "Invalid animation");

        Animation A = W->Animations[ID];
        uint16_t AnimationWidth = CurrentGame.AnimationArray[A.AnimationID].width;
        uint16_t FrameWidth = AnimationWidth / A.FramesInAnimation;

        Rectangle Source = {
            .x = FrameWidth * A.CurrentFrameInAnimation,
            .y = 0.0f,
            .width = W->Movements[ID].Direction.x * FrameWidth,
            .height = CurrentGame.AnimationArray[A.AnimationID].height
        };

        Rectangle Destination = {
            .x = W->Spatials[ID].Position.x,
            .y = W->Spatials[ID].Position.y,
            .width = Source.width * W->Spatials[ID].Scale,
            .height = Source.height * W->Spatials[ID].Scale
        };

        DrawTexturePro(CurrentGame.AnimationArray[A.AnimationID],
            Source, Destination, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);

        // Debug Lines
        if (W->Components[ID] & CCollisionBox)
        {
            Rectangle AnimationRectangle = (Rectangle) {
                .x = W->Spatials[ID].Position.x,
                .y = W->Spatials[ID].Position.y,
                .width = (float)W->CollisionBoxes[ID].Width,
                .height = (float)W->CollisionBoxes[ID].Height
            };

            DrawRectangleLinesEx(AnimationRectangle, 1.0f, RED);
        }
    }

    EndMode2D();

    // Draw UI
    uint16_t PID = CurrentGame.GamePlayer.ID;
    float Current = (float)W->Healths[PID].Current;
    float Max = (float)W->Healths[PID].Max;

    Rectangle HealthBar = {
        .x = 45.0f,
        .y = 19.0f,
        .width = (400.0f * Current) / Max,
        .height = 10.0f
    };

    Rectangle HealthBarOutline = {
        .x = 45.0f,
        .y = 19.0f,
        .width = 400.0f,
        .height = 10.0f
    };

    DrawText("HP", 14.0f, 16.0f, 18, GREEN);
    DrawRectangleRounded(HealthBar, 0.5f, 4, GREEN);
    DrawRectangleRoundedLines(HealthBarOutline, 0.5f, 4, WHITE);

    EndDrawing();
}

void LoadLevel(const char* File)
{
    FILE* Level = fopen(File, "r");
    Enforce(Level, "Failed to load level for the game");

    ArenaSnapshot(&CurrentGame.GameArena);
    char* Line = ArenaAlloc(&CurrentGame.GameArena, MaxLineSize, _Alignof(char));
    Enforce(Line, "Failed to allocate line buffer");

    PlayerInit();
    World* W = &CurrentGame.GameWorld;

    // Read from file and load other entities
    while (fgets(Line, MaxLineSize, Level) != NULL)
    {
        // Skip whitespaces, ignore comments and read until null terminator
        while (*Line == ' ' || *Line == '\n'
            || *Line == '\t' || *Line == '\r') { Line++; }

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
                int TextureID, PositionX, PositionY;
                int ScaleNumerator, ScaleDenominator;

                if (sscanf(Line, "%d, %d, %d, %d, %d, %d",
                    &Type,
                    &TextureID,
                    &PositionX,
                    &PositionY,
                    &ScaleNumerator,
                    &ScaleDenominator) != 6)
                {
                    TraceLog(LOG_WARNING, "Failed to load decoration");
                    continue;
                }

                uint16_t Dec = CreateEntity();
                Enforce(Dec < MaxEntityCount, "Failed to create entity");

                W->Components[Dec] = CDecoration;

                W->Spatials[Dec] = (Spatial) {
                    .Position = { (float)PositionX, (float)PositionY },
                    .Scale = (float)ScaleNumerator / (float)ScaleDenominator
                };

                W->Textures[Dec] = (uint16_t)TextureID;

                break;
            }
            case Tile:
            {
                int TextureID, PositionX, PositionY;
                int ScaleNumerator, ScaleDenominator;
                int CollisionBoxX, CollisionBoxY;

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
                    TraceLog(LOG_WARNING, "Failed to load tile properly");
                    continue;
                }

                uint16_t Tile = CreateEntity();
                Enforce(Tile < MaxEntityCount, "Failed to create entity");

                W->Components[Tile] = CTile;

                W->Spatials[Tile] = (Spatial) {
                    .Position = { (float)PositionX, (float)PositionY },
                    .Scale = (float)ScaleNumerator / (float)ScaleDenominator
                };

                W->CollisionBoxes[Tile] = (CollisionBox) {
                    .Width = CollisionBoxX,
                    .Height = CollisionBoxY
                };

                W->Textures[Tile] = (uint16_t)TextureID;

                break;
            }
            case Enemy:
            {
                // TODO:(Anirban): Find some assets for enemies
                // Anirban: No.

                int AnimationID, PositionX, PositionY;
                int ScaleNumerator, ScaleDenominator;
                int CollisionBoxX, CollisionBoxY;

                if (sscanf(Line, "%d, %d, %d, %d, %d, %d, %d, %d",
                    &Type,
                    &AnimationID,
                    &PositionX,
                    &PositionY,
                    &ScaleNumerator,
                    &ScaleDenominator,
                    &CollisionBoxX,
                    &CollisionBoxY) != 8)
                {
                    TraceLog(LOG_WARNING, "Failed to load enemy properly");
                    continue;
                }

                uint16_t Enemy = CreateEntity();
                Enforce(Enemy < MaxEntityCount, "Failed to create entity");

                W->Components[Enemy] = CEnemy;

                W->Spatials[Enemy] = (Spatial) {
                    .Scale = (float)ScaleNumerator / (float)ScaleDenominator,
                    .Position = { (float)PositionX, (float)PositionY }
                };

                W->Movements[Enemy] = (Movement) {
                    .Magnitude = { 0.0f, 0.0f },
                    .Direction = { 1.0f, 1.0f },
                    .PreviousPosition = W->Spatials[Enemy].Position
                };

                W->CollisionBoxes[Enemy] = (CollisionBox) {
                    .Width = CollisionBoxX,
                    .Height = CollisionBoxY
                };

                W->Animations[Enemy] = (Animation) {
                    .AnimationID = AnimationID,
                    .CurrentFrameInAnimation = 0,
                    .FramesInAnimation = FrameCountShadowIdle,
                    .FramesPassed = 0,
                    .Duration = FrameDurationShadowIdle
                };

                W->Gravities[Enemy] = (Gravity) {
                    .Acceleration = 0.5f,
                    .MaxVelocity = 20.0f
                };

                W->Healths[Enemy] = (Health) {
                    .Max = 10000,
                    .Current = 10000
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

    ArenaResetToSnapshot(&CurrentGame.GameArena);
    fclose(Level);
}

void GameInit(void)
{
    CurrentGame.GameFrame = 0;
    World* W = &CurrentGame.GameWorld;

    LoadAssets();

    CurrentGame.GameCamera = (Camera2D) {
        .target = { 640.0f, 360.0f },
        .rotation = 0.0f,
        .zoom = 1.25f
    };

    Enforce(ArenaInit(&CurrentGame.GameArena, ArenaSize),
        "Failed to initialize arena");

    W->Components = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(uint64_t), _Alignof(uint64_t));
    W->Actives = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(bool), _Alignof(bool));
    W->Spatials = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(Spatial), _Alignof(Spatial));
    W->Movements = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(Movement), _Alignof(Movement));
    W->Gravities = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(Gravity), _Alignof(Gravity));
    W->Textures = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(uint16_t), _Alignof(uint16_t));
    W->Animations = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(Animation), _Alignof(Animation));
    W->CollisionBoxes = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(CollisionBox), _Alignof(CollisionBox));
    W->Healths = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(Health), _Alignof(Health));

    W->TempGravityArray = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(uint16_t), _Alignof(uint16_t));
    W->TempTextureArray = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(uint16_t), _Alignof(uint16_t));
    W->TempAnimationArray = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(uint16_t), _Alignof(uint16_t));
    W->TempTileArray = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(uint16_t), _Alignof(uint16_t));
    W->TempEnemyArray = ArenaAlloc(&CurrentGame.GameArena, MaxEntityCount * sizeof(uint16_t), _Alignof(uint16_t));

    Enforce(W->Components && W->Actives
        && W->Spatials && W->Movements
        && W->Gravities && W->Textures
        && W->Animations && W->CollisionBoxes
        && W->Healths

        && W->TempGravityArray && W->TempTextureArray
        && W->TempAnimationArray && W->TempTileArray
        && W->TempEnemyArray
        , "Failed to initialize component arrays");
}

void PlayerInit(void)
{
    uint16_t PID = CreateEntity();
    Enforce(PID == 0, "Player should always be loaded first");
    World* W = &CurrentGame.GameWorld;

    CurrentGame.GamePlayer = (Player) {
        .Character = Samurai,
        .ID = PID,
        .Jump = 16,
        .Mask = 0,
        .Speed = 8,
        .State = PlayerStateIdle,
        .UserInput = { 0 }
    };

    W->Components[PID] = CPlayer;

    W->Spatials[PID] = (Spatial) {
        .Position = { 640.0f, 360.0f },
        .Scale = 1.0f
    };

    W->CollisionBoxes[PID] = (CollisionBox) {
        .Width = 128.0f,
        .Height = 128.0f
    };

    W->Movements[PID] = (Movement) {
        .Magnitude = { 0.0f, 0.0f },
        .Direction = { 1.0f, 1.0f },
        .PreviousPosition = { 640.0f, 360.0f }
    };

    W->Gravities[PID] = (Gravity) {
        .Acceleration = 0.5f,
        .MaxVelocity = 20.0f
    };

    W->Animations[PID] = (Animation) {
        .AnimationID = SamuraiIdle,
        .CurrentFrameInAnimation = 0,
        .FramesInAnimation = FrameCountSamuraiIdle,
        .FramesPassed = 0,
        .Duration = FrameDurationSamuraiIdle
    };

    W->Healths[PID] = (Health) {
        .Max = 10000,
        .Current = 10000
    };
}

void GetUserInput(void)
{
    CurrentGame.GamePlayer.UserInput = (Input) {
        .Z = IsKeyDown(KEY_Z),
        .X = IsKeyDown(KEY_X),
        .C = IsKeyDown(KEY_C),
        .Up = IsKeyDown(KEY_UP),
        .Down = IsKeyDown(KEY_DOWN),
        .Left = IsKeyDown(KEY_LEFT),
        .Right = IsKeyDown(KEY_RIGHT)
    };
}

void LogMemoryUsed(void)
{
    Arena* A = &CurrentGame.GameArena;
    size_t Used = (size_t)(A->Current - A->Start);
    size_t Percentage = Used * 100 / A->Size;
    TraceLog(LOG_INFO, "Memory used: %zu out of %zu, %zu %", Used, A->Size, Percentage);
}

void LoadAssets(void)
{
    // Load Animations
    CurrentGame.AnimationArray[FighterAttack1] = LoadTexture(AssetPath "animations/fighter/Attack_1.png");
    CurrentGame.AnimationArray[FighterAttack2] = LoadTexture(AssetPath "animations/fighter/Attack_2.png");
    CurrentGame.AnimationArray[FighterAttack3] = LoadTexture(AssetPath "animations/fighter/Attack_3.png");
    CurrentGame.AnimationArray[FighterDead] = LoadTexture(AssetPath "animations/fighter/Dead.png");
    CurrentGame.AnimationArray[FighterHurt] = LoadTexture(AssetPath "animations/fighter/Hurt.png");
    CurrentGame.AnimationArray[FighterIdle] = LoadTexture(AssetPath "animations/fighter/Idle.png");
    CurrentGame.AnimationArray[FighterJump] = LoadTexture(AssetPath "animations/fighter/Jump.png");
    CurrentGame.AnimationArray[FighterRun] = LoadTexture(AssetPath "animations/fighter/Run.png");
    CurrentGame.AnimationArray[FighterShield] = LoadTexture(AssetPath "animations/fighter/Shield.png");
    CurrentGame.AnimationArray[FighterWalk] = LoadTexture(AssetPath "animations/fighter/Walk.png");
    CurrentGame.AnimationArray[Chest] = LoadTexture(AssetPath "animations/misc/Chest.png");
    CurrentGame.AnimationArray[Coin] = LoadTexture(AssetPath "animations/misc/Coin.png");
    CurrentGame.AnimationArray[Flag] = LoadTexture(AssetPath "animations/misc/Flag.png");
    CurrentGame.AnimationArray[Key] = LoadTexture(AssetPath "animations/misc/Key.png");
    CurrentGame.AnimationArray[Rune] = LoadTexture(AssetPath "animations/misc/Rune.png");
    CurrentGame.AnimationArray[SamuraiAttack1] = LoadTexture(AssetPath "animations/samurai/Attack_1.png");
    CurrentGame.AnimationArray[SamuraiAttack2] = LoadTexture(AssetPath "animations/samurai/Attack_2.png");
    CurrentGame.AnimationArray[SamuraiAttack3] = LoadTexture(AssetPath "animations/samurai/Attack_3.png");
    CurrentGame.AnimationArray[SamuraiDead] = LoadTexture(AssetPath "animations/samurai/Dead.png");
    CurrentGame.AnimationArray[SamuraiHurt] = LoadTexture(AssetPath "animations/samurai/Hurt.png");
    CurrentGame.AnimationArray[SamuraiIdle] = LoadTexture(AssetPath "animations/samurai/Idle.png");
    CurrentGame.AnimationArray[SamuraiJump] = LoadTexture(AssetPath "animations/samurai/Jump.png");
    CurrentGame.AnimationArray[SamuraiRun] = LoadTexture(AssetPath "animations/samurai/Run.png");
    CurrentGame.AnimationArray[SamuraiShield] = LoadTexture(AssetPath "animations/samurai/Shield.png");
    CurrentGame.AnimationArray[SamuraiWalk] = LoadTexture(AssetPath "animations/samurai/Walk.png");
    CurrentGame.AnimationArray[ShinobiAttack1] = LoadTexture(AssetPath "animations/shinobi/Attack_1.png");
    CurrentGame.AnimationArray[ShinobiAttack2] = LoadTexture(AssetPath "animations/shinobi/Attack_2.png");
    CurrentGame.AnimationArray[ShinobiAttack3] = LoadTexture(AssetPath "animations/shinobi/Attack_3.png");
    CurrentGame.AnimationArray[ShinobiDead] = LoadTexture(AssetPath "animations/shinobi/Dead.png");
    CurrentGame.AnimationArray[ShinobiHurt] = LoadTexture(AssetPath "animations/shinobi/Hurt.png");
    CurrentGame.AnimationArray[ShinobiIdle] = LoadTexture(AssetPath "animations/shinobi/Idle.png");
    CurrentGame.AnimationArray[ShinobiJump] = LoadTexture(AssetPath "animations/shinobi/Jump.png");
    CurrentGame.AnimationArray[ShinobiRun] = LoadTexture(AssetPath "animations/shinobi/Run.png");
    CurrentGame.AnimationArray[ShinobiShield] = LoadTexture(AssetPath "animations/shinobi/Shield.png");
    CurrentGame.AnimationArray[ShinobiWalk] = LoadTexture(AssetPath "animations/shinobi/Walk.png");
    CurrentGame.AnimationArray[ShadowAttack1] = LoadTexture(AssetPath "animations/bosses/shadow/Attack_1.png");
    CurrentGame.AnimationArray[ShadowAttack2] = LoadTexture(AssetPath "animations/bosses/shadow/Attack_2.png");
    CurrentGame.AnimationArray[ShadowAttack3] = LoadTexture(AssetPath "animations/bosses/shadow/Attack_3.png");
    CurrentGame.AnimationArray[ShadowDefend] = LoadTexture(AssetPath "animations/bosses/shadow/Defend.png");
    CurrentGame.AnimationArray[ShadowHurt] = LoadTexture(AssetPath "animations/bosses/shadow/Hurt.png");
    CurrentGame.AnimationArray[ShadowIdle] = LoadTexture(AssetPath "animations/bosses/shadow/Idle.png");
    CurrentGame.AnimationArray[ShadowInvisibility] = LoadTexture(AssetPath "animations/bosses/shadow/Invisibility.png");
    CurrentGame.AnimationArray[ShadowJump] = LoadTexture(AssetPath "animations/bosses/shadow/Jump.png");
    CurrentGame.AnimationArray[ShadowMove] = LoadTexture(AssetPath "animations/bosses/shadow/Move.png");
    CurrentGame.AnimationArray[ShadowTeleport1] = LoadTexture(AssetPath "animations/bosses/shadow/Teleport_1.png");
    CurrentGame.AnimationArray[ShadowTeleport2] = LoadTexture(AssetPath "animations/bosses/shadow/Teleport_2.png");

    // Load Textures
    CurrentGame.TextureArray[BGFull] = LoadTexture(AssetPath "background/Background.png");
    CurrentGame.TextureArray[BGLayer1] = LoadTexture(AssetPath "background/layer1.png");
    CurrentGame.TextureArray[BGLayer2] = LoadTexture(AssetPath "background/layer2.png");
    CurrentGame.TextureArray[BGLayer3] = LoadTexture(AssetPath "background/layer3.png");
    CurrentGame.TextureArray[BGLayer4] = LoadTexture(AssetPath "background/layer4.png");
    CurrentGame.TextureArray[BGLayer5] = LoadTexture(AssetPath "background/layer5.png");
    CurrentGame.TextureArray[Box1] = LoadTexture(AssetPath "objects/boxes/1.png");
    CurrentGame.TextureArray[Box2] = LoadTexture(AssetPath "objects/boxes/2.png");
    CurrentGame.TextureArray[Box3] = LoadTexture(AssetPath "objects/boxes/3.png");
    CurrentGame.TextureArray[Box4] = LoadTexture(AssetPath "objects/boxes/4.png");
    CurrentGame.TextureArray[Box5] = LoadTexture(AssetPath "objects/boxes/5.png");
    CurrentGame.TextureArray[Box6] = LoadTexture(AssetPath "objects/boxes/6.png");
    CurrentGame.TextureArray[Bush1] = LoadTexture(AssetPath "objects/bushes/1.png");
    CurrentGame.TextureArray[Bush2] = LoadTexture(AssetPath "objects/bushes/2.png");
    CurrentGame.TextureArray[Bush3] = LoadTexture(AssetPath "objects/bushes/3.png");
    CurrentGame.TextureArray[Bush4] = LoadTexture(AssetPath "objects/bushes/4.png");
    CurrentGame.TextureArray[Bush5] = LoadTexture(AssetPath "objects/bushes/5.png");
    CurrentGame.TextureArray[Bush6] = LoadTexture(AssetPath "objects/bushes/6.png");
    CurrentGame.TextureArray[Bush7] = LoadTexture(AssetPath "objects/bushes/7.png");
    CurrentGame.TextureArray[Bush8] = LoadTexture(AssetPath "objects/bushes/8.png");
    CurrentGame.TextureArray[Bush9] = LoadTexture(AssetPath "objects/bushes/9.png");
    CurrentGame.TextureArray[Fence1] = LoadTexture(AssetPath "objects/fence/1.png");
    CurrentGame.TextureArray[Fence2] = LoadTexture(AssetPath "objects/fence/2.png");
    CurrentGame.TextureArray[Fence3] = LoadTexture(AssetPath "objects/fence/3.png");
    CurrentGame.TextureArray[Grass1] = LoadTexture(AssetPath "objects/grass/01.png");
    CurrentGame.TextureArray[Grass2] = LoadTexture(AssetPath "objects/grass/02.png");
    CurrentGame.TextureArray[Grass3] = LoadTexture(AssetPath "objects/grass/03.png");
    CurrentGame.TextureArray[Grass4] = LoadTexture(AssetPath "objects/grass/04.png");
    CurrentGame.TextureArray[Grass5] = LoadTexture(AssetPath "objects/grass/05.png");
    CurrentGame.TextureArray[Grass6] = LoadTexture(AssetPath "objects/grass/06.png");
    CurrentGame.TextureArray[Grass7] = LoadTexture(AssetPath "objects/grass/07.png");
    CurrentGame.TextureArray[Grass8] = LoadTexture(AssetPath "objects/grass/08.png");
    CurrentGame.TextureArray[Grass9] = LoadTexture(AssetPath "objects/grass/09.png");
    CurrentGame.TextureArray[Grass10] = LoadTexture(AssetPath "objects/grass/10.png");
    CurrentGame.TextureArray[Ladder1] = LoadTexture(AssetPath "objects/ladders/1.png");
    CurrentGame.TextureArray[Ladder2] = LoadTexture(AssetPath "objects/ladders/2.png");
    CurrentGame.TextureArray[Ladder3] = LoadTexture(AssetPath "objects/ladders/3.png");
    CurrentGame.TextureArray[Ladder4] = LoadTexture(AssetPath "objects/ladders/4.png");
    CurrentGame.TextureArray[Ladder5] = LoadTexture(AssetPath "objects/ladders/5.png");
    CurrentGame.TextureArray[Ladder6] = LoadTexture(AssetPath "objects/ladders/6.png");
    CurrentGame.TextureArray[Pointer1] = LoadTexture(AssetPath "objects/pointers/1.png");
    CurrentGame.TextureArray[Pointer2] = LoadTexture(AssetPath "objects/pointers/2.png");
    CurrentGame.TextureArray[Pointer3] = LoadTexture(AssetPath "objects/pointers/3.png");
    CurrentGame.TextureArray[Pointer4] = LoadTexture(AssetPath "objects/pointers/4.png");
    CurrentGame.TextureArray[Pointer5] = LoadTexture(AssetPath "objects/pointers/5.png");
    CurrentGame.TextureArray[Pointer6] = LoadTexture(AssetPath "objects/pointers/6.png");
    CurrentGame.TextureArray[Pointer7] = LoadTexture(AssetPath "objects/pointers/7.png");
    CurrentGame.TextureArray[Pointer8] = LoadTexture(AssetPath "objects/pointers/8.png");
    CurrentGame.TextureArray[Ridge1] = LoadTexture(AssetPath "objects/ridges/1.png");
    CurrentGame.TextureArray[Ridge2] = LoadTexture(AssetPath "objects/ridges/2.png");
    CurrentGame.TextureArray[Ridge3] = LoadTexture(AssetPath "objects/ridges/3.png");
    CurrentGame.TextureArray[Ridge4] = LoadTexture(AssetPath "objects/ridges/4.png");
    CurrentGame.TextureArray[Ridge5] = LoadTexture(AssetPath "objects/ridges/5.png");
    CurrentGame.TextureArray[Ridge6] = LoadTexture(AssetPath "objects/ridges/6.png");
    CurrentGame.TextureArray[Stone1] = LoadTexture(AssetPath "objects/stones/1.png");
    CurrentGame.TextureArray[Stone2] = LoadTexture(AssetPath "objects/stones/2.png");
    CurrentGame.TextureArray[Stone3] = LoadTexture(AssetPath "objects/stones/3.png");
    CurrentGame.TextureArray[Stone4] = LoadTexture(AssetPath "objects/stones/4.png");
    CurrentGame.TextureArray[Stone5] = LoadTexture(AssetPath "objects/stones/5.png");
    CurrentGame.TextureArray[Tree1] = LoadTexture(AssetPath "objects/trees/1.png");
    CurrentGame.TextureArray[Tree2] = LoadTexture(AssetPath "objects/trees/2.png");
    CurrentGame.TextureArray[Tree3] = LoadTexture(AssetPath "objects/trees/3.png");
    CurrentGame.TextureArray[Willow1] = LoadTexture(AssetPath "objects/willows/1.png");
    CurrentGame.TextureArray[Willow2] = LoadTexture(AssetPath "objects/willows/2.png");
    CurrentGame.TextureArray[Willow3] = LoadTexture(AssetPath "objects/willows/3.png");
    CurrentGame.TextureArray[Tile1] = LoadTexture(AssetPath "tiles/Tile_01.png");
    CurrentGame.TextureArray[Tile2] = LoadTexture(AssetPath "tiles/Tile_02.png");
    CurrentGame.TextureArray[Tile3] = LoadTexture(AssetPath "tiles/Tile_03.png");
    CurrentGame.TextureArray[Tile4] = LoadTexture(AssetPath "tiles/Tile_04.png");
    CurrentGame.TextureArray[Tile5] = LoadTexture(AssetPath "tiles/Tile_05.png");
    CurrentGame.TextureArray[Tile6] = LoadTexture(AssetPath "tiles/Tile_06.png");
    CurrentGame.TextureArray[Tile7] = LoadTexture(AssetPath "tiles/Tile_07.png");
    CurrentGame.TextureArray[Tile8] = LoadTexture(AssetPath "tiles/Tile_08.png");
    CurrentGame.TextureArray[Tile9] = LoadTexture(AssetPath "tiles/Tile_09.png");
    CurrentGame.TextureArray[Tile10] = LoadTexture(AssetPath "tiles/Tile_10.png");
    CurrentGame.TextureArray[Tile11] = LoadTexture(AssetPath "tiles/Tile_11.png");
    CurrentGame.TextureArray[Tile12] = LoadTexture(AssetPath "tiles/Tile_12.png");
    CurrentGame.TextureArray[Tile13] = LoadTexture(AssetPath "tiles/Tile_13.png");
    CurrentGame.TextureArray[Tile14] = LoadTexture(AssetPath "tiles/Tile_14.png");
    CurrentGame.TextureArray[Tile15] = LoadTexture(AssetPath "tiles/Tile_15.png");
    CurrentGame.TextureArray[Tile16] = LoadTexture(AssetPath "tiles/Tile_16.png");
    CurrentGame.TextureArray[Tile17] = LoadTexture(AssetPath "tiles/Tile_17.png");
    CurrentGame.TextureArray[Tile18] = LoadTexture(AssetPath "tiles/Tile_18.png");
    CurrentGame.TextureArray[Tile19] = LoadTexture(AssetPath "tiles/Tile_19.png");
    CurrentGame.TextureArray[Tile20] = LoadTexture(AssetPath "tiles/Tile_20.png");
    CurrentGame.TextureArray[Tile21] = LoadTexture(AssetPath "tiles/Tile_21.png");
    CurrentGame.TextureArray[Tile22] = LoadTexture(AssetPath "tiles/Tile_22.png");
    CurrentGame.TextureArray[Tile23] = LoadTexture(AssetPath "tiles/Tile_23.png");
    CurrentGame.TextureArray[Tile24] = LoadTexture(AssetPath "tiles/Tile_24.png");
    CurrentGame.TextureArray[Tile25] = LoadTexture(AssetPath "tiles/Tile_25.png");
    CurrentGame.TextureArray[Tile26] = LoadTexture(AssetPath "tiles/Tile_26.png");
    CurrentGame.TextureArray[Tile27] = LoadTexture(AssetPath "tiles/Tile_27.png");
    CurrentGame.TextureArray[Tile28] = LoadTexture(AssetPath "tiles/Tile_28.png");
    CurrentGame.TextureArray[Tile29] = LoadTexture(AssetPath "tiles/Tile_29.png");
    CurrentGame.TextureArray[Tile30] = LoadTexture(AssetPath "tiles/Tile_30.png");
    CurrentGame.TextureArray[Tile31] = LoadTexture(AssetPath "tiles/Tile_31.png");
    CurrentGame.TextureArray[Tile32] = LoadTexture(AssetPath "tiles/Tile_32.png");
    CurrentGame.TextureArray[Tile33] = LoadTexture(AssetPath "tiles/Tile_33.png");
    CurrentGame.TextureArray[Tile34] = LoadTexture(AssetPath "tiles/Tile_34.png");
    CurrentGame.TextureArray[Tile35] = LoadTexture(AssetPath "tiles/Tile_35.png");
    CurrentGame.TextureArray[Tile36] = LoadTexture(AssetPath "tiles/Tile_36.png");
    CurrentGame.TextureArray[Tile37] = LoadTexture(AssetPath "tiles/Tile_37.png");
    CurrentGame.TextureArray[Tile38] = LoadTexture(AssetPath "tiles/Tile_38.png");
    CurrentGame.TextureArray[Tile39] = LoadTexture(AssetPath "tiles/Tile_39.png");
    CurrentGame.TextureArray[Tile40] = LoadTexture(AssetPath "tiles/Tile_40.png");
    CurrentGame.TextureArray[Tile41] = LoadTexture(AssetPath "tiles/Tile_41.png");
    CurrentGame.TextureArray[Tile42] = LoadTexture(AssetPath "tiles/Tile_42.png");
    CurrentGame.TextureArray[Tile43] = LoadTexture(AssetPath "tiles/Tile_43.png");
    CurrentGame.TextureArray[Tile44] = LoadTexture(AssetPath "tiles/Tile_44.png");
    CurrentGame.TextureArray[Tile45] = LoadTexture(AssetPath "tiles/Tile_45.png");
    CurrentGame.TextureArray[Tile46] = LoadTexture(AssetPath "tiles/Tile_46.png");
    CurrentGame.TextureArray[Tile47] = LoadTexture(AssetPath "tiles/Tile_47.png");
    CurrentGame.TextureArray[Tile48] = LoadTexture(AssetPath "tiles/Tile_48.png");
    CurrentGame.TextureArray[Tile49] = LoadTexture(AssetPath "tiles/Tile_49.png");
    CurrentGame.TextureArray[Tile50] = LoadTexture(AssetPath "tiles/Tile_50.png");
    CurrentGame.TextureArray[Tile51] = LoadTexture(AssetPath "tiles/Tile_51.png");
    CurrentGame.TextureArray[Tile52] = LoadTexture(AssetPath "tiles/Tile_52.png");
    CurrentGame.TextureArray[Tile53] = LoadTexture(AssetPath "tiles/Tile_53.png");
    CurrentGame.TextureArray[Tile54] = LoadTexture(AssetPath "tiles/Tile_54.png");
    CurrentGame.TextureArray[Tile55] = LoadTexture(AssetPath "tiles/Tile_55.png");
    CurrentGame.TextureArray[Tile56] = LoadTexture(AssetPath "tiles/Tile_56.png");
    CurrentGame.TextureArray[Tile57] = LoadTexture(AssetPath "tiles/Tile_57.png");
    CurrentGame.TextureArray[Tile58] = LoadTexture(AssetPath "tiles/Tile_58.png");
    CurrentGame.TextureArray[Tile59] = LoadTexture(AssetPath "tiles/Tile_59.png");
    CurrentGame.TextureArray[Tile60] = LoadTexture(AssetPath "tiles/Tile_60.png");
}
