#ifndef PIECE_H
#define PIECE_H

#include "raylib.h"
#include "Forward.h"
#include <vector>
using namespace std;
class Game;

enum class PieceType {
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING
};

class Piece {
protected:
    int x, y;             
    Texture2D texture;      
    bool isWhite;            
    PieceType type;         

public:
    Piece(int xPos, int yPos, Texture2D tex, bool white, PieceType pieceType) 
        : x(xPos), y(yPos), isWhite(white), type(pieceType) {
        texture = tex;  
    }
    
    virtual ~Piece() {
        texture.id = 0;
    }
    virtual  vector<Vector2> GetValidMoves(const Game& game) const = 0;
    int GetX() const { return x; }
    int GetY() const { return y; }
    const Texture2D& GetTexture() const { return texture; }
    bool IsWhite() const { return isWhite; }
    PieceType GetType() const { return type; }
    Vector2 GetPosition() const { return Vector2{static_cast<float>(x), static_cast<float>(y)}; }

   
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

