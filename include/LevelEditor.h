#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <raylib.h>

typedef struct {
    uint16_t SelectedTexture;
    uint16_t LastNewEntity;
    uint16_t* NewEntities;
    char* LevelFile;
} LevelEditor;

void LevelEditorInit(void);
void LevelEditorUpdate(void);
void LevelEditorRender(void);
void UpdateAssetPicker(void);
void RenderAssetPicker(void);
void SaveLevel(const char* File);
