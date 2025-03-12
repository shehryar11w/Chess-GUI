#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "raylib.h"
#include <string>
#include <unordered_map>
#include <memory>

class TextureManager {
private:
    static TextureManager* instance;
    std::unordered_map<std::string, Texture2D> textures;
    bool initialized;

    // Private constructor for singleton
    TextureManager() : initialized(false) {}

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
            textures["default"] = LoadTextureFromImage(whitePixel);
            UnloadImage(whitePixel);
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

    Texture2D LoadTextureFromFile(const std::string& key, const std::string& filepath) {
        if (!initialized) {
            Initialize();
        }

        // Check if texture already exists
        auto it = textures.find(key);
        if (it != textures.end()) {
            return it->second;
        }

        // Load texture using Raylib's high-level function
        Texture2D tex = LoadTexture(filepath.c_str());
        
        // If texture failed to load, return default texture
        if (tex.id == 0) {
            TraceLog(LOG_WARNING, "Failed to load texture: %s", filepath.c_str());
            return textures["default"];
        }

        textures[key] = tex;
        return tex;
    }

    Texture2D GetTexture(const std::string& key) {
        if (!initialized) {
            Initialize();
        }

        auto it = textures.find(key);
        return (it != textures.end()) ? it->second : textures["default"];
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