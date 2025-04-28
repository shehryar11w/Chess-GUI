#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "raylib.h"
#include <string>
#include <vector>
#include <memory>

struct TextureEntry {
    std::string key;
    Texture2D texture;
};

class TextureManager {
private:
    static TextureManager* instance;
    std::vector<TextureEntry> textures;  
    bool initialized;

    TextureManager() : initialized(false) {}

    
    int GetTextureIndex(const std::string& key) {
        for (size_t i = 0; i < textures.size(); ++i) {
            if (textures[i].key == key) {
                return i;
            }
        }
        return -1; 
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
            Image whitePixel = GenImageColor(32, 32, WHITE);
            TextureEntry defaultTexture = {"default", LoadTextureFromImage(whitePixel)};
            textures.push_back(defaultTexture);
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

        
        int index = GetTextureIndex(key);
        if (index != -1) {
            return textures[index].texture;
        }

        
        Texture2D tex = LoadTexture(filepath.c_str());
        if (tex.id == 0) {
            TraceLog(LOG_WARNING, "Failed to load texture: %s", filepath.c_str());
            return textures[0].texture;  
        }

        textures.push_back({key, tex});
        return tex;
    }

    Texture2D GetTexture(const std::string& key) {
        if (!initialized) {
            Initialize();
        }

        int index = GetTextureIndex(key);
        return (index != -1) ? textures[index].texture : textures[0].texture;  
    }

    void UnloadAllTextures() {
        for (auto& entry : textures) {
            if (entry.texture.id > 0) {
                UnloadTexture(entry.texture);
            }
        }
        textures.clear();
        initialized = false;
    }
};

#endif
