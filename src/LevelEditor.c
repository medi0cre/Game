#include <LevelEditor.h>
#include <Game.h>
#include <Utils.h>
#include <raylib.h>

// Raygui has many unused variables and functions in its implementation
// This is just a way to remove warnings caused by those variables and functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#pragma GCC diagnostic pop

#define GridSize 80.0f
#define CameraSpeed 16.0f

static LevelEditor Editor = { 0 };

void LevelEditorInit(void)
{
    Editor = (LevelEditor) {
        .SelectedTexture = Tile1,
        .LastNewEntity = 0,
        .LevelFile = LevelPath "level1.txt",
        .NewEntities = ArenaAlloc(&CurrentGame.GameArena, sizeof(uint16_t) * MaxEntityCount, _Alignof(uint16_t))
    };

    Enforce(Editor.NewEntities, "Failed to initialize NewEntities");
}

void PlaceEntity(uint16_t TextureID, Vector2 Position, EntityType Type)
{
    World* W = &CurrentGame.GameWorld;
    Enforce(TextureID < TextureCount, "Precondition broken in PlaceEntity");

    uint16_t ID = CreateEntity();
    Enforce(ID < MaxEntityCount, "Failed to create entity in PlaceEntity");

    Editor.NewEntities[Editor.LastNewEntity++] = ID;
    W->Textures[ID] = TextureID;

    if (Type == Tile)
    {
        W->Components[ID] = CTile;
        W->CollisionBoxes[ID] = (CollisionBox) {
            .Width = GridSize,
            .Height = GridSize
        };

        W->Spatials[ID] = (Spatial) {
            .Position = Position,
            .Scale = 2.5f
        };

        TraceLog(LOG_INFO, "Placed new tile, entity ID: %d", ID);
    }
    else if (Type == Decoration)
    {
        Enforce(false, "No support for decorations yet");

        W->Components[ID] = CDecoration;
        W->Spatials[ID] = (Spatial) {
            .Position = Position,
            .Scale = 1.0f
        };

        TraceLog(LOG_INFO, "Placed new decoration, entity ID: %d", ID);
    }
    else { Enforce(false, "Invalid entity in level editor"); }
}

void LevelEditorUpdate(void)
{
    bool Up = IsKeyDown(KEY_UP);
    bool Down = IsKeyDown(KEY_DOWN);
    bool Left = IsKeyDown(KEY_LEFT);
    bool Right = IsKeyDown(KEY_RIGHT);

    if (Up) { CurrentGame.GameCamera.target.y -= CameraSpeed; }
    if (Down) { CurrentGame.GameCamera.target.y += CameraSpeed; }
    if (Left) { CurrentGame.GameCamera.target.x -= CameraSpeed; }
    if (Right) { CurrentGame.GameCamera.target.x += CameraSpeed; }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Rectangle Panel = (Rectangle) { 0.0f, 0.0f, 360.0f, 120.0f };
        Vector2 ScreenMouse = GetMousePosition();

        // Prevent the editor from adding another tile when clicking the save button
        if (!CheckCollisionPointRec(ScreenMouse, Panel))
        {
            Vector2 WorldMouse = GetScreenToWorld2D(ScreenMouse, CurrentGame.GameCamera);
            WorldMouse.x = floorf(WorldMouse.x / GridSize) * GridSize;
            WorldMouse.y = floorf(WorldMouse.y / GridSize) * GridSize;

            bool Overlap = false;
            World* W = &CurrentGame.GameWorld;

            for (uint16_t i = 0; i < MaxEntityCount; i++)
            {
                if (!W->Actives[i]) { continue; }
                if (W->Spatials[i].Position.x == WorldMouse.x 
                    && W->Spatials[i].Position.y == WorldMouse.y)
                {
                    Overlap = true;
                    TraceLog(LOG_ERROR, "Overlap detected with entity %d", i);
                    break;
                }
            }

            if (!Overlap) { PlaceEntity(Editor.SelectedTexture, WorldMouse, Tile); }
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        // TODO: Do something LOL
    }
}

void RenderEditorWorld(void)
{
    World* W = &CurrentGame.GameWorld;
    BeginMode2D(CurrentGame.GameCamera);

    for (uint16_t i = 0; i < MaxEntityCount; i++)
    {
        if (!W->Actives[i]) { continue; }

        uint64_t Components = W->Components[i];
        if (!(Components & CTexture)) { continue; }

        uint16_t TextureID = W->Textures[i];
        Enforce(TextureID < TextureCount, "Precondition broken inside RenderEditorWorld");

        DrawTextureEx(
            CurrentGame.TextureArray[TextureID],
            W->Spatials[i].Position,
            0.0f,
            W->Spatials[i].Scale,
            WHITE
        );

        if (Components & CCollisionBox)
        {
            Rectangle Box = {
                .x = W->Spatials[i].Position.x,
                .y = W->Spatials[i].Position.y,
                .width = (float)W->CollisionBoxes[i].Width,
                .height = (float)W->CollisionBoxes[i].Height
            };

            DrawRectangleLinesEx(Box, 1.0f, RED);
        }
    }

    EndMode2D();
}

void LevelEditorRender(void)
{
    BeginDrawing();
    ClearBackground(BLACK);
    RenderEditorWorld();

    Vector2 Mouse = GetScreenToWorld2D(GetMousePosition(), CurrentGame.GameCamera);
    DrawText(TextFormat("Mouse: (%.2f, %.2f)", Mouse.x, Mouse.y), 1000, 10, 20, WHITE);
    DrawText(TextFormat("FPS: %d", GetFPS()), 1000, 40, 20, WHITE);

    BeginMode2D(CurrentGame.GameCamera);
    Mouse.x = floorf(Mouse.x / GridSize) * GridSize;
    Mouse.y = floorf(Mouse.y / GridSize) * GridSize;
    DrawRectangleLines(Mouse.x, Mouse.y, GridSize, GridSize, RED);
    EndMode2D();

    GuiPanel((Rectangle) { 0.0f, 0.0f, 360.0f, 120.0f }, "Level Editor");
    int SaveButton = GuiButton((Rectangle) { 230, 40, 100, 30 }, "Save");
    if (SaveButton) { SaveLevel(Editor.LevelFile); }

    EndDrawing();
}

void SaveLevel(const char* File)
{
    FILE* Level = fopen(File, "ab");
    Enforce(Level, "Failed to save level");

    World* W = &CurrentGame.GameWorld;

    for (uint16_t i = 0; i < Editor.LastNewEntity; i++)
    {
        uint16_t ID = Editor.NewEntities[i];
        if (!W->Actives[ID]) { continue; }

        uint64_t C = W->Components[ID];
        if (C == CPlayer) { Enforce(false, "Cannot add player using a level editor"); }

        int64_t PositionX = W->Spatials[ID].Position.x;
        int64_t PositionY = W->Spatials[ID].Position.y;

        uint16_t ScaleNumerator = W->Spatials[ID].Scale * 1000;
        uint16_t ScaleDenominator = 1000;

        if (C == CDecoration)
        {
            Enforce(false, "For now, we only support tiles");
            fprintf(
                Level,
                "%d, %d, %lld, %lld, %d, %d\n",
                Decoration,
                W->Textures[ID],
                PositionX, PositionY,
                ScaleNumerator, ScaleDenominator
            );
        }
        else if (C == CTile)
        {
            fprintf(
                Level,
                "%d, %d, %lld, %lld, %d, %d, %d, %d\n",
                Tile,
                W->Textures[ID],
                PositionX, PositionY,
                ScaleNumerator, ScaleDenominator,
                W->CollisionBoxes[ID].Width,
                W->CollisionBoxes[ID].Height
            );

            TraceLog(LOG_INFO, "Saved tile, entity ID: %d", ID);
        }
        else if (C == CEnemy)
        {
            Enforce(false, "For now, we only support tiles");
            fprintf(
                Level,
                "%d, %d, %lld, %lld, %d, %d, %d, %d\n",
                Enemy,
                W->Animations[ID].AnimationID,
                PositionX, PositionY,
                ScaleNumerator, ScaleDenominator,
                W->CollisionBoxes[ID].Width,
                W->CollisionBoxes[ID].Height
            );
        }
        else { Enforce(false, "Invalid entity type"); }
    }

    TraceLog(LOG_INFO, "Added %d new entities to %s", Editor.LastNewEntity, Editor.LevelFile);
    fclose(Level);
}
