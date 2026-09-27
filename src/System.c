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

    if (CurrentGame.GamePlayer.Mask & HasChangedState)
    {
        W->Animations[PID].CurrentFrameInAnimation = 0;
        CurrentGame.GamePlayer.Mask &= ~HasChangedState;
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

void S_Collision(uint16_t LastCollisionBox)
{
    Enforce(LastCollisionBox < MaxEntityCount,
        "Precondition broken inside S_Collision()");

    uint16_t PID = CurrentGame.GamePlayer.ID;
    World* W = &CurrentGame.GameWorld;

    uint16_t PreviousState = CurrentGame.GamePlayer.State;
    bool IsStanding = false;

    for (uint16_t i = 0; i < LastCollisionBox; i++)
    {
        uint16_t ID = W->TempCollisionBoxArray[i];
        if (ID == PID) { continue; }

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

        // This triggered once, need to solve this later
        Enforce(Overlap.height != Overlap.width,
            "Same collision dimensions cannot be resolved, HAAALP!!");

        if (Overlap.width > Overlap.height) // Vertical Collision
        {
            float* PositionY = &W->Spatials[PID].Position.y;

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
                IsStanding = true;
            }

        }
        else if (Overlap.width < Overlap.height) // Horizontal Collision
        {
            float* PositionX = &W->Spatials[PID].Position.x;

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
    }

    uint16_t* State = &CurrentGame.GamePlayer.State;
    uint16_t* Mask = &CurrentGame.GamePlayer.Mask;
    Input UserInput = CurrentGame.GamePlayer.UserInput;

    if (!IsStanding) { *State = PlayerStateJumping; }
    else if (UserInput.Left != UserInput.Right) { *State = PlayerStateRunning; }
    else { *State = PlayerStateIdle; }

    if (PreviousState != *State) { *Mask |= HasChangedState; }
}

void S_Movement(void)
{
    uint16_t PID = CurrentGame.GamePlayer.ID;
    Input UserInput = CurrentGame.GamePlayer.UserInput;
    uint16_t PreviousState = CurrentGame.GamePlayer.State;

    World* W = &CurrentGame.GameWorld;
    uint16_t* State = &CurrentGame.GamePlayer.State;
    uint16_t* Mask = &CurrentGame.GamePlayer.Mask;

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

    float* MagnitudeY = &W->Movements[PID].Magnitude.y;
    float* DirectionY = &W->Movements[PID].Direction.y;

    // Player jumps
    if (UserInput.Up
        && *State != PlayerStateJumping
        && (*Mask & CanJump))
    {
        float Velocity = -CurrentGame.GamePlayer.Jump;
        Velocity = fmaxf(Velocity, -W->Gravities[PID].MaxVelocity);

        *MagnitudeY = fabsf(Velocity);
        *DirectionY = Velocity >= 0.0f ? 1.0f : -1.0f;

        *Mask &= ~CanJump;
        *State = PlayerStateJumping;
    }

    // Player lets go of jump halfway
    if (!UserInput.Up && *State == PlayerStateJumping
        && *MagnitudeY > 0.0f
        && *DirectionY == -1.0f)
    {
        *MagnitudeY = 0.0f;
        *DirectionY = 1.0f;
    }

    // Used to prevent bunny hops
    if (*State != PlayerStateJumping && !UserInput.Up) { *Mask |= CanJump; }
    if (PreviousState != *State) { *Mask |= HasChangedState; }

    W->Movements[PID].PreviousPosition = W->Spatials[PID].Position;
    W->Spatials[PID].Position.y += (*MagnitudeY) * (*DirectionY);
    W->Spatials[PID].Position.x += (*MagnitudeX) * (*DirectionX);
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

