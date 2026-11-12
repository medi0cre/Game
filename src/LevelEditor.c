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

typedef enum {
    GridSize = 80,
    CameraSpeed = 16,
} EditorConfig;

typedef enum {
    UIWidth = 360,
    UITop = 130,
    UIPadding = 10,
    UIColumnCount = 5,
    UICellSize = 64,
    UIThumbnailSize = 48,
    UIBackgroundCount = 6,
    UIAssetCount = TextureCount - UIBackgroundCount, // The 6 backgrounds should not be in the asset picker
    UIRowCount = (UIAssetCount + UIColumnCount - 1) / UIColumnCount
} UIConfig;

static LevelEditor Editor = { 0 };
static float UIScroll = 0.0f;

static const Rectangle UI = {
    .x = 10.0f,
    .y = UITop,
    .width = UIWidth - 20.0f,
    .height = WindowHeight - UITop - 10.0f
};

static const Rectangle Grid = {
    .x = UI.x + UIPadding,
    .y = UI.y + 45.0f,
    .width = UI.width - 2 * UIPadding,
    .height = UI.height - 120.0f
};

static const Rectangle SaveRectangle = {
    .x = 10.0f,
    .y = 10.0f,
    .width = 100.0f,
    .height = 30.0f
};

const char* GetTextureName(TextureIndex ID);

void UpdateAssetPicker(void)
{
    Vector2 ScreenMouse = GetMousePosition();

    if (CheckCollisionPointRec(ScreenMouse, UI))
    {
        float Wheel = GetMouseWheelMove();
        UIScroll -= Wheel * UICellSize * 2.0f;
    }

    Enforce(UIRowCount < 10000, "TotalRows overflow");

    float MaxScroll = fmaxf(0.0f, UIRowCount * UICellSize - Grid.height);

    UIScroll = UIScroll > 0.0f ? UIScroll : 0.0f;
    UIScroll = UIScroll < MaxScroll ? UIScroll : MaxScroll;

    if (!CheckCollisionPointRec(ScreenMouse, Grid)) { return; }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { return; }

    uint16_t Column = (ScreenMouse.x - Grid.x) / UICellSize;
    uint16_t Row = (ScreenMouse.y - Grid.y + UIScroll) / UICellSize;

    Enforce(
        Column < 10000
        && Row < 10000
        && Column < UIColumnCount
        , "Overflow detected in Column or row"
    );

    uint16_t Index = Row * UIColumnCount + Column;
    if (Index >= UIAssetCount) { return; }

    Editor.SelectedTexture = UIBackgroundCount + Index;
    TraceLog(LOG_INFO, "Selected texture: %d", Editor.SelectedTexture);
}

void RenderAssetPicker(void)
{
    DrawRectangleRec(UI, (Color) { 35, 35, 42, 255 });
    DrawRectangleLinesEx(UI, 1.0f, GRAY);

    DrawText("Asset Picker", UI.x + 10, UI.y + 10, 18, WHITE);
    DrawLine(UI.x, UI.y + 38, UI.x + UI.width, UI.y + 38, GRAY);

    BeginScissorMode(Grid.x, Grid.y, Grid.width, Grid.height);

    for (uint16_t i = 0; i < UIAssetCount; i++)
    {
        uint16_t Column = i % UIColumnCount;
        uint16_t Row = i / UIColumnCount;

        Rectangle Cell = {
            .x = Grid.x + Column * UICellSize,
            .y = Grid.y + Row * UICellSize - UIScroll,
            .width = UICellSize - 4.0f,
            .height = UICellSize - 4.0f
        };

        uint16_t TextureID = UIBackgroundCount + i;
        Texture2D Tex = CurrentGame.TextureArray[TextureID];

        Vector2 ScreenMouse = GetMousePosition();
        Color DarkBlue = { 75, 110, 160, 255 };
        Color DarkGray = { 50, 50, 60, 255 };

        bool Selected = Editor.SelectedTexture == TextureID;
        bool MouseOverCell = CheckCollisionPointRec(ScreenMouse, Cell)
            && CheckCollisionPointRec(ScreenMouse, Grid);

        Color Background = MouseOverCell ? DarkBlue : DarkGray;
        DrawRectangleRec(Cell, Background);
        if (Selected) { DrawRectangleLinesEx(Cell, 2.0f, SKYBLUE); }

        float Scale = fminf(
            (float)UIThumbnailSize / Tex.width,
            (float)UIThumbnailSize / Tex.height
        );

        float Width = Tex.width * Scale;
        float Height = Tex.height * Scale;

        Rectangle Source = {
            .x = 0.0f,
            .y = 0.0f,
            .width = Tex.width,
            .height = Tex.height
        };

        Rectangle Destination = {
            .x = Cell.x + (Cell.width - Width) / 2.0f,
            .y = Cell.y + (Cell.height - Height) / 2.0f,
            .width = Tex.width * Scale,
            .height = Tex.height * Scale
        };

        DrawTexturePro(Tex, Source, Destination, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    EndScissorMode();

    float FooterY = UI.y + UI.height - 65.0f;
    DrawLine(UI.x, FooterY, UI.x + UI.width, FooterY, GRAY);

    const char* Text = TextFormat("Selected: %s", GetTextureName(Editor.SelectedTexture));
    DrawText(Text, UI.x + 10, FooterY + 10, 16, WHITE);
}

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

    ArenaSnapshot(&CurrentGame.GameArena);

    uint16_t TileCount = 0;
    uint16_t* TileArray = ArenaAlloc(&CurrentGame.GameArena, sizeof(uint16_t) * MaxEntityCount, _Alignof(uint16_t));
    Enforce(TileArray, "Failed to allocate memory for TileArray");

    for (uint16_t i = 0; i < MaxEntityCount; i++)
    {
        if (!W->Actives[i]) { continue; }

        uint64_t Components = W->Components[i];
        if (Components != CTile) { continue; }

        TileArray[TileCount++] = i;
    }

    // Use height only to determine the scale for now
    // Might need to be changed later on to also include width
    Texture2D Tex = CurrentGame.TextureArray[TextureID];
    float Scale = (float)GridSize / Tex.height;

    float OffsetX = Tex.width * Scale < GridSize ? (GridSize - Tex.width * Scale) * 0.5f : 0.0f;
    Position.x += OffsetX;

    Rectangle TextureBox = {
        .x = Position.x,
        .y = Position.y,
        .width = Tex.width * Scale,
        .height = Tex.height * Scale
    };

    for (uint16_t i = 0; i < TileCount; i++)
    {
        uint16_t ID = TileArray[i];
        Enforce(ID < MaxEntityCount, "Invalid entity in PlaceEntity");

        Rectangle TileBox = {
            .x = W->Spatials[ID].Position.x,
            .y = W->Spatials[ID].Position.y,
            .width = W->CollisionBoxes[ID].Width,
            .height = W->CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(TextureBox, TileBox);
        if (Overlap.width == 0.0f || Overlap.height == 0.0f) { continue; }

        TraceLog(LOG_ERROR, "Collision detected with entity %d", ID);
        ArenaResetToSnapshot(&CurrentGame.GameArena);
        return;
    }

    ArenaResetToSnapshot(&CurrentGame.GameArena);

    uint16_t ID = CreateEntity();
    Enforce(ID < MaxEntityCount, "Failed to create entity in PlaceEntity");

    Editor.NewEntities[Editor.LastNewEntity++] = ID;
    W->Textures[ID] = TextureID;

    if (Type == Tile)
    {
        W->Components[ID] = CTile;
        W->CollisionBoxes[ID] = (CollisionBox) {
            .Width = TextureBox.width,
            .Height = TextureBox.height
        };

        W->Spatials[ID] = (Spatial) {
            .Position = Position,
            .Scale = Scale
        };

        TraceLog(LOG_INFO, "Placed new tile, entity ID: %d", ID);
    }
    else if (Type == Decoration)
    {
        W->Components[ID] = CDecoration;
        W->Spatials[ID] = (Spatial) {
            .Position = Position,
            .Scale = Scale
        };

        TraceLog(LOG_INFO, "Placed new decoration, entity ID: %d", ID);
    }
    else { Enforce(false, "Invalid entity in level editor"); }
}

void LevelEditorUpdate(void)
{
    UpdateAssetPicker();

    bool Up = IsKeyDown(KEY_UP);
    bool Down = IsKeyDown(KEY_DOWN);
    bool Left = IsKeyDown(KEY_LEFT);
    bool Right = IsKeyDown(KEY_RIGHT);

    if (Up) { CurrentGame.GameCamera.target.y -= CameraSpeed; }
    if (Down) { CurrentGame.GameCamera.target.y += CameraSpeed; }
    if (Left) { CurrentGame.GameCamera.target.x -= CameraSpeed; }
    if (Right) { CurrentGame.GameCamera.target.x += CameraSpeed; }

    Vector2 ScreenMouse = GetMousePosition();

    bool MouseLeft = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool MouseRight = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    bool MouseOverUI = CheckCollisionPointRec(ScreenMouse, SaveRectangle)
        || CheckCollisionPointRec(ScreenMouse, UI);

    if (!(MouseLeft || MouseRight)) { return; }
    if (MouseOverUI) { return; }

    Vector2 WorldMouse = GetScreenToWorld2D(ScreenMouse, CurrentGame.GameCamera);
    WorldMouse.x = floorf(WorldMouse.x / GridSize) * GridSize;
    WorldMouse.y = floorf(WorldMouse.y / GridSize) * GridSize;

    EntityType Type = MouseLeft ? Tile : Decoration;
    PlaceEntity(Editor.SelectedTexture, WorldMouse, Type);
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

    Vector2 ScreenMouse = GetMousePosition();
    Vector2 WorldMouse = GetScreenToWorld2D(ScreenMouse, CurrentGame.GameCamera);

    DrawText(TextFormat("Mouse: (%.2f, %.2f)", WorldMouse.x, WorldMouse.y), 1000, 10, 20, WHITE);
    DrawText(TextFormat("FPS: %d", GetFPS()), 1000, 40, 20, WHITE);

    bool MouseOverUI = CheckCollisionPointRec(ScreenMouse, UI)
        || CheckCollisionPointRec(ScreenMouse, SaveRectangle);

    if (!MouseOverUI)
    {
        BeginMode2D(CurrentGame.GameCamera);
        WorldMouse.x = floorf(WorldMouse.x / GridSize) * GridSize;
        WorldMouse.y = floorf(WorldMouse.y / GridSize) * GridSize;
        DrawRectangleLines(WorldMouse.x, WorldMouse.y, GridSize, GridSize, RED);
        EndMode2D();
    }

    int SaveButton = GuiButton(SaveRectangle, "Save");
    if (SaveButton) { SaveLevel(Editor.LevelFile); }

    RenderAssetPicker();
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
            fprintf(
                Level,
                "%d, %d, %lld, %lld, %d, %d\n",
                Decoration,
                W->Textures[ID],
                PositionX, PositionY,
                ScaleNumerator, ScaleDenominator
            );

            TraceLog(LOG_INFO, "Saved decoration, entity ID: %d", ID);
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
        else { Enforce(false, "Enemies and player cannot be added using the level editor"); }
    }

    TraceLog(LOG_INFO, "Added %d new entities to %s", Editor.LastNewEntity, Editor.LevelFile);
    Editor.LastNewEntity = 0;

    fclose(Level);
}

const char* GetTextureName(TextureIndex ID)
{
    switch (ID)
    {
    case Box1: { return "Box1"; }
    case Box2: { return "Box2"; }
    case Box3: { return "Box3"; }
    case Box4: { return "Box4"; }
    case Box5: { return "Box5"; }
    case Box6: { return "Box6"; }
    case Bush1: { return "Bush1"; }
    case Bush2: { return "Bush2"; }
    case Bush3: { return "Bush3"; }
    case Bush4: { return "Bush4"; }
    case Bush5: { return "Bush5"; }
    case Bush6: { return "Bush6"; }
    case Bush7: { return "Bush7"; }
    case Bush8: { return "Bush8"; }
    case Bush9: { return "Bush9"; }
    case Fence1: { return "Fence1"; }
    case Fence2: { return "Fence2"; }
    case Fence3: { return "Fence3"; }
    case Grass1: { return "Grass1"; }
    case Grass2: { return "Grass2"; }
    case Grass3: { return "Grass3"; }
    case Grass4: { return "Grass4"; }
    case Grass5: { return "Grass5"; }
    case Grass6: { return "Grass6"; }
    case Grass7: { return "Grass7"; }
    case Grass8: { return "Grass8"; }
    case Grass9: { return "Grass9"; }
    case Grass10: { return "Grass10"; }
    case Ladder1: { return "Ladder1"; }
    case Ladder2: { return "Ladder2"; }
    case Ladder3: { return "Ladder3"; }
    case Ladder4: { return "Ladder4"; }
    case Ladder5: { return "Ladder5"; }
    case Ladder6: { return "Ladder6"; }
    case Pointer1: { return "Pointer1"; }
    case Pointer2: { return "Pointer2"; }
    case Pointer3: { return "Pointer3"; }
    case Pointer4: { return "Pointer4"; }
    case Pointer5: { return "Pointer5"; }
    case Pointer6: { return "Pointer6"; }
    case Pointer7: { return "Pointer7"; }
    case Pointer8: { return "Pointer8"; }
    case Ridge1: { return "Ridge1"; }
    case Ridge2: { return "Ridge2"; }
    case Ridge3: { return "Ridge3"; }
    case Ridge4: { return "Ridge4"; }
    case Ridge5: { return "Ridge5"; }
    case Ridge6: { return "Ridge6"; }
    case Stone1: { return "Stone1"; }
    case Stone2: { return "Stone2"; }
    case Stone3: { return "Stone3"; }
    case Stone4: { return "Stone4"; }
    case Stone5: { return "Stone5"; }
    case Tree1: { return "Tree1"; }
    case Tree2: { return "Tree2"; }
    case Tree3: { return "Tree3"; }
    case Willow1: { return "Willow1"; }
    case Willow2: { return "Willow2"; }
    case Willow3: { return "Willow3"; }
    case Tile1: { return "Tile1"; }
    case Tile2: { return "Tile2"; }
    case Tile3: { return "Tile3"; }
    case Tile4: { return "Tile4"; }
    case Tile5: { return "Tile5"; }
    case Tile6: { return "Tile6"; }
    case Tile7: { return "Tile7"; }
    case Tile8: { return "Tile8"; }
    case Tile9: { return "Tile9"; }
    case Tile10: { return "Tile10"; }
    case Tile11: { return "Tile11"; }
    case Tile12: { return "Tile12"; }
    case Tile13: { return "Tile13"; }
    case Tile14: { return "Tile14"; }
    case Tile15: { return "Tile15"; }
    case Tile16: { return "Tile16"; }
    case Tile17: { return "Tile17"; }
    case Tile18: { return "Tile18"; }
    case Tile19: { return "Tile19"; }
    case Tile20: { return "Tile20"; }
    case Tile21: { return "Tile21"; }
    case Tile22: { return "Tile22"; }
    case Tile23: { return "Tile23"; }
    case Tile24: { return "Tile24"; }
    case Tile25: { return "Tile25"; }
    case Tile26: { return "Tile26"; }
    case Tile27: { return "Tile27"; }
    case Tile28: { return "Tile28"; }
    case Tile29: { return "Tile29"; }
    case Tile30: { return "Tile30"; }
    case Tile31: { return "Tile31"; }
    case Tile32: { return "Tile32"; }
    case Tile33: { return "Tile33"; }
    case Tile34: { return "Tile34"; }
    case Tile35: { return "Tile35"; }
    case Tile36: { return "Tile36"; }
    case Tile37: { return "Tile37"; }
    case Tile38: { return "Tile38"; }
    case Tile39: { return "Tile39"; }
    case Tile40: { return "Tile40"; }
    case Tile41: { return "Tile41"; }
    case Tile42: { return "Tile42"; }
    case Tile43: { return "Tile43"; }
    case Tile44: { return "Tile44"; }
    case Tile45: { return "Tile45"; }
    case Tile46: { return "Tile46"; }
    case Tile47: { return "Tile47"; }
    case Tile48: { return "Tile48"; }
    case Tile49: { return "Tile49"; }
    case Tile50: { return "Tile50"; }
    case Tile51: { return "Tile51"; }
    case Tile52: { return "Tile52"; }
    case Tile53: { return "Tile53"; }
    case Tile54: { return "Tile54"; }
    case Tile55: { return "Tile55"; }
    case Tile56: { return "Tile56"; }
    case Tile57: { return "Tile57"; }
    case Tile58: { return "Tile58"; }
    case Tile59: { return "Tile59"; }
    case Tile60: { return "Tile60"; }
    default:
    {
        Enforce(false, "This asset should not be in the asset picker");
        return NULL;
    }
    }
}
