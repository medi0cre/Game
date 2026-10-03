#include <System.h>
#include <Assets.h>
#include <Utils.h>
#include <Component.h>
#include <math.h>

void S_Gravity(uint16_t LastGravity)
{
    Enforce(LastGravity < MaxEntityCount,
        "Precondition broken inside S_Gravity()");

    World* W = &CurrentGame.GameWorld;

    for (uint16_t i = 0; i < LastGravity; i++)
    {
        uint16_t ID = W->TempGravityArray[i];
        float* MagnitudeY = &W->Movements[ID].Magnitude.y;
        float* DirectionY = &W->Movements[ID].Direction.y;

        Enforce(ID < MaxEntityCount
            && W->Gravities[ID].Acceleration > 0.0f
            && *MagnitudeY <= W->Gravities[ID].MaxVelocity
            && *MagnitudeY >= 0.0f
            && (*DirectionY == 1.0f || *DirectionY == -1.0f)
            , "Invalid values inside S_Gravity()");

        float Velocity = (*MagnitudeY) * (*DirectionY);
        Velocity += W->Gravities[ID].Acceleration;

        Velocity = fminf(Velocity, W->Gravities[ID].MaxVelocity);
        *MagnitudeY = fabsf(Velocity);
        *DirectionY = Velocity >= 0.0f ? 1.0f : -1.0f;
    }
}

void S_Animation(void)
{
    World* W = &CurrentGame.GameWorld;
    uint16_t PID = CurrentGame.GamePlayer.ID;

    Enforce(W->TempAnimationArray
        && W->LastAnimation < MaxEntityCount
        , "Precondition broken inside S_Animation()");

    switch (CurrentGame.GamePlayer.State)
    {
        case PlayerStateIdle:
        {
            W->Animations[PID].AnimationID = SamuraiIdle;
            W->Animations[PID].FramesInAnimation = FrameCountSamuraiIdle;
            W->Animations[PID].Duration = FrameDurationSamuraiIdle;
            break;
        }
        case PlayerStateRunning:
        {
            W->Animations[PID].AnimationID = SamuraiRun;
            W->Animations[PID].FramesInAnimation = FrameCountSamuraiRun;
            W->Animations[PID].Duration = FrameDurationSamuraiRun;
            break;
        }
        case PlayerStateJumping:
        {
            W->Animations[PID].AnimationID = SamuraiJump;
            W->Animations[PID].FramesInAnimation = FrameCountSamuraiJump;
            W->Animations[PID].Duration = FrameDurationSamuraiJump;
            break;
        }
        default:
        {
            Enforce(false, "Impossible player state encountered");
            return;
        }
    }

    for (uint16_t i = 0; i < W->LastAnimation; i++)
    {
        uint16_t ID = W->TempAnimationArray[i];
        Enforce(ID < MaxEntityCount, "Invalid ID inside S_Animation()");
        Animation* A = &CurrentGame.GameWorld.Animations[ID];

        if (A->FramesPassed > A->Duration)
        {
            uint16_t* Frame = &A->CurrentFrameInAnimation;
            *Frame = (*Frame + 1) % A->FramesInAnimation;
            A->FramesPassed = 0;
            continue;
        }

        A->FramesPassed++;
    }
}

void S_MovementX(void)
{
    // For now, only player entity has horizontal movement
    uint16_t PID = CurrentGame.GamePlayer.ID;
    Input UserInput = CurrentGame.GamePlayer.UserInput;

    World* W = &CurrentGame.GameWorld;
    uint16_t* State = &CurrentGame.GamePlayer.State;

    Enforce(PID < MaxEntityCount, "Invalid Player ID");

    float* MagnitudeX = &W->Movements[PID].Magnitude.x;
    float* DirectionX = &W->Movements[PID].Direction.x;

    if (UserInput.Right && !UserInput.Left)
    {
        *MagnitudeX = CurrentGame.GamePlayer.Speed;
        *DirectionX = 1.0f;

        if (*State != PlayerStateJumping) { *State = PlayerStateRunning; }
    }
    else if (!UserInput.Right && UserInput.Left)
    {
        *MagnitudeX = CurrentGame.GamePlayer.Speed;
        *DirectionX = -1.0f;

        if (*State != PlayerStateJumping) { *State = PlayerStateRunning; }
    }
    else
    {
        *MagnitudeX = 0.0f;
        if (*State != PlayerStateJumping) { *State = PlayerStateIdle; }
    }

    W->Movements[PID].PreviousPosition.x = W->Spatials[PID].Position.x;
    W->Spatials[PID].Position.x += (*MagnitudeX) * (*DirectionX);
}

void S_CollisionX(uint16_t LastTile, uint16_t LastEnemy)
{
    Enforce(LastEnemy < MaxEntityCount
        && LastTile < MaxEntityCount,
        "S_CollisionX: Invalid entity ID");

    uint16_t PID = CurrentGame.GamePlayer.ID;
    World* W = &CurrentGame.GameWorld;
    float* PositionX = &W->Spatials[PID].Position.x;

    // Player tile collisions
    for (uint16_t i = 0; i < LastTile; i++)
    {
        uint16_t ID = W->TempTileArray[i];
        Enforce(ID < MaxEntityCount, "Invalid ID inside S_Collision()");

        Rectangle PlayerBox = {
            .x = W->Spatials[PID].Position.x,
            .y = W->Spatials[PID].Position.y,
            .width = W->CollisionBoxes[PID].Width,
            .height = W->CollisionBoxes[PID].Height
        };

        Rectangle TileBox = {
            .x = W->Spatials[ID].Position.x,
            .y = W->Spatials[ID].Position.y,
            .width = W->CollisionBoxes[ID].Width,
            .height = W->CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(PlayerBox, TileBox);
        if (Overlap.width == 0.0f || Overlap.height == 0.0f) { continue; }

        // Player hits tile from the right
        if (*PositionX < W->Movements[PID].PreviousPosition.x)
        {
            *PositionX += Overlap.width;
            W->Movements[PID].Magnitude.x = 0.0f;
        }

        // Player hits tile from the left
        else if (*PositionX > W->Movements[PID].PreviousPosition.x)
        {
            W->Spatials[PID].Position.x -= Overlap.width;
            W->Movements[PID].Magnitude.x = 0.0f;
        }
    }

    // Player enemy collisions
    for (uint16_t i = 0; i < LastEnemy; i++)
    {
        uint16_t ID = W->TempEnemyArray[i];
        Enforce(ID < MaxEntityCount, "Invalid ID inside S_Collision()");

        Rectangle PlayerBox = {
            .x = W->Spatials[PID].Position.x,
            .y = W->Spatials[PID].Position.y,
            .width = W->CollisionBoxes[PID].Width,
            .height = W->CollisionBoxes[PID].Height
        };

        Rectangle EnemyBox = {
            .x = W->Spatials[ID].Position.x,
            .y = W->Spatials[ID].Position.y,
            .width = W->CollisionBoxes[ID].Width,
            .height = W->CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(PlayerBox, EnemyBox);
        if (Overlap.width == 0.0f || Overlap.height == 0.0f) { continue; }

        // Player hits enemy from the right
        if (*PositionX < W->Movements[PID].PreviousPosition.x)
        {
            *PositionX += Overlap.width;
            W->Movements[PID].Magnitude.x = 0.0f;
        }

        // Player hits enemy from the left
        else if (*PositionX > W->Movements[PID].PreviousPosition.x)
        {
            W->Spatials[PID].Position.x -= Overlap.width;
            W->Movements[PID].Magnitude.x = 0.0f;
        }
    }
}

void S_MovementY(uint16_t LastEnemy)
{
    uint16_t PID = CurrentGame.GamePlayer.ID;
    World* W = &CurrentGame.GameWorld;

    for (uint16_t i = 0; i < LastEnemy; i++)
    {
        uint16_t ID = W->TempEnemyArray[i];
        Enforce(ID < MaxEntityCount,
            "Invalid enemy ID in MovementY");

        float* MagnitudeY = &W->Movements[ID].Magnitude.y;
        float* DirectionY = &W->Movements[ID].Direction.y;

        W->Movements[ID].PreviousPosition.y = W->Spatials[ID].Position.y;
        W->Spatials[ID].Position.y += (*MagnitudeY) * (*DirectionY);
    }

    float* MagnitudeY = &W->Movements[PID].Magnitude.y;
    float* DirectionY = &W->Movements[PID].Direction.y;

    Input UserInput = CurrentGame.GamePlayer.UserInput;
    uint16_t* State = &CurrentGame.GamePlayer.State;

    // Player jumps
    if (UserInput.Up && W->Jumps[PID].CanJump
        && *State != PlayerStateJumping)
    {
        float Velocity = -W->Jumps[PID].InitialVelocity;
        Velocity = fmaxf(Velocity, -W->Gravities[PID].MaxVelocity);

        *MagnitudeY = fabsf(Velocity);
        *DirectionY = Velocity >= 0.0f ? 1.0f : -1.0f;

        W->Jumps[PID].CanJump = false;
        *State = PlayerStateJumping;
    }

    // Player lets go of jump halfway
    if (!UserInput.Up && *State == PlayerStateJumping
        && *MagnitudeY > 0.0f && *DirectionY == -1.0f)
    {
        *MagnitudeY = 0.0f;
        *DirectionY = 1.0f;
    }

    // Used to prevent bunny hops
    if (*State != PlayerStateJumping && !UserInput.Up) { W->Jumps[PID].CanJump = true; }

    W->Movements[PID].PreviousPosition.y = W->Spatials[PID].Position.y;
    W->Spatials[PID].Position.y += (*MagnitudeY) * (*DirectionY);
}

void S_CollisionY(uint16_t LastTile, uint16_t LastEnemy)
{
    Enforce(LastEnemy < MaxEntityCount
        && LastTile < MaxEntityCount,
        "S_CollisionY: Invalid entity ID");

    uint16_t PID = CurrentGame.GamePlayer.ID;
    World* W = &CurrentGame.GameWorld;

    // Ironically, the order is VERY important here
    // Enemy movement updates must be resolved before player
    // Otherwise, the enemies can get pushed into the player causing weird bugs

    // Enemy tile collisions
    for (uint16_t i = 0; i < LastEnemy; i++)
    {
        uint16_t EID = W->TempEnemyArray[i];
        Enforce(EID < MaxEntityCount,
            "Invalid enemy ID");

        for (uint16_t j = 0; j < LastTile; j++)
        {
            uint16_t TID = W->TempTileArray[j];
            Enforce(TID < MaxEntityCount,
                "Invalid tile ID");

            Rectangle TileBox = {
                .x = W->Spatials[TID].Position.x,
                .y = W->Spatials[TID].Position.y,
                .width = W->CollisionBoxes[TID].Width,
                .height = W->CollisionBoxes[TID].Height
            };

            Rectangle EnemyBox = {
                .x = W->Spatials[EID].Position.x,
                .y = W->Spatials[EID].Position.y,
                .width = W->CollisionBoxes[EID].Width,
                .height = W->CollisionBoxes[EID].Height
            };

            Rectangle Overlap = GetCollisionRec(TileBox, EnemyBox);
            if (Overlap.width == 0.0f || Overlap.height == 0.0f) { continue; }

            float* PositionY = &W->Spatials[EID].Position.y;

            // Enemy hits tile from below
            if (*PositionY < W->Movements[EID].PreviousPosition.y)
            {
                *PositionY += Overlap.height;
                W->Movements[EID].Magnitude.y = 0.0f;
                W->Movements[EID].Direction.y = 1.0f;
            }

            // Enemy hits tile from above
            else if (*PositionY > W->Movements[EID].PreviousPosition.y)
            {
                *PositionY -= Overlap.height;
                W->Movements[EID].Magnitude.y = 0.0f;
                W->Movements[EID].Direction.y = 1.0f;
            }
        }
    }

    float* PositionY = &W->Spatials[PID].Position.y;
    bool PlayerStand = false;

    // Player tile collisions
    for (uint16_t i = 0; i < LastTile; i++)
    {
        uint16_t ID = W->TempTileArray[i];
        Enforce(ID < MaxEntityCount, "Invalid ID inside S_CollisionY");

        Rectangle PlayerBox = {
            .x = W->Spatials[PID].Position.x,
            .y = W->Spatials[PID].Position.y,
            .width = W->CollisionBoxes[PID].Width,
            .height = W->CollisionBoxes[PID].Height
        };

        Rectangle TileBox = {
            .x = W->Spatials[ID].Position.x,
            .y = W->Spatials[ID].Position.y,
            .width = W->CollisionBoxes[ID].Width,
            .height = W->CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(PlayerBox, TileBox);
        if (Overlap.width == 0.0f || Overlap.height == 0.0f) { continue; }

        // Player hits tile from below
        if (*PositionY < W->Movements[PID].PreviousPosition.y)
        {
            *PositionY += Overlap.height;
            W->Movements[PID].Magnitude.y = 0.0f;
            W->Movements[PID].Direction.y = 1.0f;
        }

        // Player hits tile from above
        else if (*PositionY > W->Movements[PID].PreviousPosition.y)
        {
            *PositionY -= Overlap.height;
            W->Movements[PID].Magnitude.y = 0.0f;
            W->Movements[PID].Direction.y = 1.0f;
            PlayerStand = true;
        }
    }

    // Player enemy collisions
    for (uint16_t i = 0; i < LastEnemy; i++)
    {
        uint16_t ID = W->TempEnemyArray[i];
        Enforce(ID < MaxEntityCount, "Invalid ID inside S_Collision()");

        Rectangle PlayerBox = {
            .x = W->Spatials[PID].Position.x,
            .y = W->Spatials[PID].Position.y,
            .width = W->CollisionBoxes[PID].Width,
            .height = W->CollisionBoxes[PID].Height
        };

        Rectangle EnemyBox = {
            .x = W->Spatials[ID].Position.x,
            .y = W->Spatials[ID].Position.y,
            .width = W->CollisionBoxes[ID].Width,
            .height = W->CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(PlayerBox, EnemyBox);
        if (Overlap.width == 0.0f || Overlap.height == 0.0f) { continue; }

        // Player hits enemy from below
        if (*PositionY < W->Movements[PID].PreviousPosition.y)
        {
            *PositionY += Overlap.height;
            W->Movements[PID].Magnitude.y = 0.0f;
            W->Movements[PID].Direction.y = 1.0f;
        }

        // Player hits enemy from above
        else if (*PositionY > W->Movements[PID].PreviousPosition.y)
        {
            *PositionY -= Overlap.height;
            W->Movements[PID].Magnitude.y = 0.0f;
            W->Movements[PID].Direction.y = 1.0f;
            PlayerStand = true;
        }
    }

    uint16_t* State = &CurrentGame.GamePlayer.State;
    Input UserInput = CurrentGame.GamePlayer.UserInput;

    if (!PlayerStand) { *State = PlayerStateJumping; }
    else if (UserInput.Left != UserInput.Right) { *State = PlayerStateRunning; }
    else { *State = PlayerStateIdle; }
}

void S_Camera(void)
{
    World* W = &CurrentGame.GameWorld;
    uint16_t PID = CurrentGame.GamePlayer.ID;

    Enforce(PID < MaxEntityCount,
        "Invalid Player ID inside S_Camera()");

    Camera2D* Camera = &CurrentGame.GameCamera;
    Camera->offset = (Vector2) {
        GetRenderWidth() * 0.5f,
        GetRenderHeight() * 0.5f
    };

    float PlayerX = W->Spatials[PID].Position.x + W->CollisionBoxes[PID].Width * 0.5f;
    float PlayerY = W->Spatials[PID].Position.y + W->CollisionBoxes[PID].Height * 0.5f;

    float Left = Camera->target.x - CameraBoxWidth * 0.5f;
    float Right = Camera->target.x + CameraBoxWidth * 0.5f;
    float Top = Camera->target.y - CameraBoxHeight * 0.5f;
    float Bottom = Camera->target.y + CameraBoxHeight * 0.5f;

    if (PlayerX < Left) { Camera->target.x = PlayerX + CameraBoxWidth * 0.5f; }
    if (PlayerX > Right) { Camera->target.x = PlayerX - CameraBoxWidth * 0.5f; }
    if (PlayerY < Top) { Camera->target.y = PlayerY + CameraBoxHeight * 0.5f; }
    if (PlayerY > Bottom) { Camera->target.y = PlayerY - CameraBoxHeight * 0.5f; }
}

