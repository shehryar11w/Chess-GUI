#ifndef PIECE_H
#define PIECE_H

#include "raylib.h"
#include "Forward.h"
#include <vector>

// Forward declarations
class Game;

class Piece {
protected:
    int x, y;                // Position on the board
    Texture2D texture;       // Visual representation
    bool isWhite;            // Color of the piece

public:
    Piece(int xPos, int yPos, Texture2D tex, bool white) 
        : x(xPos), y(yPos), isWhite(white) {
        texture = tex;  // Texture is managed by Team class
    }
    
    virtual ~Piece() {
        // Do not unload texture here - it's managed by Team class
        texture.id = 0;
    }

    // Pure virtual function for valid moves - implementation in derived classes
    virtual std::vector<Vector2> GetValidMoves(const Game& game) const = 0;

    // Getters
    int GetX() const { return x; }
    int GetY() const { return y; }
    const Texture2D& GetTexture() const { return texture; }
    bool IsWhite() const { return isWhite; }

    // Setters
    void SetPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

protected:
    bool IsValidPosition(int checkX, int checkY) const {
        return checkX >= 0 && checkX < 8 && checkY >= 0 && checkY < 8;
    }
};

#endif
