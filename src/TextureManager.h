#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "raylib.h"
#include <string>
#include <vector>
using namespace std;
class TextureManager {
private:
    static TextureManager* instance;
     vector< pair< string, Texture2D>> textures;
    bool initialized;
    TextureManager() : initialized(false) {}
    Texture2D* FindTexture(const  string& key) {
        for (auto& pair : textures) {
            if (pair.first == key) return &pair.second;
        }
        return nullptr;
    }

public:
    static TextureManager* GetInstance() {
        if (!instance) {
            instance = new TextureManager();
        }
        return instance;
    }

    void Initialize() {
        if (!initialized) {
            // Create a default white texture
            Image whitePixel = GenImageColor(32, 32, WHITE);
            Texture2D defaultTex = LoadTextureFromImage(whitePixel);
            Texture2D defaultTex = LoadTextureFromImage(whitePixel);
            UnloadImage(whitePixel);
            textures.push_back({ "default", defaultTex });
            textures.push_back({ "default", defaultTex });
            initialized = true;
        }
    }

    static void Cleanup() {
        if (instance) {
            instance->UnloadAllTextures();
            delete instance;
            instance = nullptr;
        }
    }

    ~TextureManager() {
        UnloadAllTextures();
    }

    Texture2D LoadTextureFromFile(const  string& key, const  string& filepath) {
        if (!initialized) {
            Initialize();
        }

        Texture2D* existing = FindTexture(key);
        if (existing) {
            return *existing;
        Texture2D* existing = FindTexture(key);
        if (existing) {
            return *existing;
        }

        Texture2D tex = LoadTexture(filepath.c_str());
        if (tex.id == 0) {
            TraceLog(LOG_WARNING, "Failed to load texture: %s", filepath.c_str());
            return *FindTexture("default");
            return *FindTexture("default");
        }

        textures.push_back({ key, tex });
        textures.push_back({ key, tex });
        return tex;
    }

    Texture2D GetTexture(const  string& key) {
        if (!initialized) {
            Initialize();
        }

        Texture2D* found = FindTexture(key);
        return found ? *found : *FindTexture("default");
        Texture2D* found = FindTexture(key);
        return found ? *found : *FindTexture("default");
    }

    void UnloadAllTextures() {
        for (auto& [key, texture] : textures) {
            if (texture.id > 0) {
                UnloadTexture(texture);
            }
        }
        textures.clear();
        initialized = false;
    }
};
#endif

#endif
