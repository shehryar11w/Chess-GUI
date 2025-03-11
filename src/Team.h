#ifndef TEAM_H
#define TEAM_H

#include "raylib.h"
#include <vector>
#include <map>
#include <memory>
#include "Piece.h"
#include "Pawn.h"
#include "King.h"
#include "Knight.h"
#include "Bishop.h"
#include "Queen.h"
#include "Rook.h"
#include <ctype.h>

class Piece;

class Team {
private:
    bool isWhite;
    std::vector<std::unique_ptr<Piece>> pieces;  // Now stores pointers to Piece objects
    std::map<char, Texture2D> textures;

public:
    Team(bool isWhiteTeam) : isWhite(isWhiteTeam) {
        LoadTextures();
        SetupPieces();
    }

    ~Team() {
        for (auto& pair : textures) {
            UnloadTexture(pair.second);
        }
    }

    void LoadTextures() { /* Same as before */ }

    void SetupPieces();  // Will be updated below

    const std::vector<std::unique_ptr<Piece>>& GetPieces() const { return pieces; }
    Piece* FindPieceAt(int x, int y) {
        for (auto& piece : pieces) {
            if (piece->GetX() == x && piece->GetY() == y) {
                return piece.get();
            }
        }
        return nullptr;
    }

    void RemovePieceAt(int x, int y) {
        auto it = pieces.begin();
        while (it != pieces.end()) {
            if ((*it)->GetX() == x && (*it)->GetY() == y) {
                it = pieces.erase(it);
                return;
            }
            ++it;
        }
    }

private:
    void AddPiece(int x, int y, char type);  // Updated below
};

void Team::SetupPieces() {
    if (isWhite) {
        for (int x = 0; x < 8; x++) AddPiece(x, 6, 'P');
        AddPiece(0, 7, 'R'); AddPiece(1, 7, 'N'); AddPiece(2, 7, 'B');
        AddPiece(3, 7, 'Q'); AddPiece(4, 7, 'K'); AddPiece(5, 7, 'B');
        AddPiece(6, 7, 'N'); AddPiece(7, 7, 'R');
    } else {
        for (int x = 0; x < 8; x++) AddPiece(x, 1, 'P');
        AddPiece(0, 0, 'R'); AddPiece(1, 0, 'N'); AddPiece(2, 0, 'B');
        AddPiece(3, 0, 'Q'); AddPiece(4, 0, 'K'); AddPiece(5, 0, 'B');
        AddPiece(6, 0, 'N'); AddPiece(7, 0, 'R');
    }
}

void Team::AddPiece(int x, int y, char type) {
    switch (toupper(type)) {
        case 'P':
            pieces.push_back(std::make_unique<Pawn>(x, y, textures[type], isWhite));
            break;
        case 'R':
            pieces.push_back(std::make_unique<Rook>(x, y, textures[type], isWhite));
            break;
        case 'N':
            pieces.push_back(std::make_unique<Knight>(x, y, textures[type], isWhite));
            break;
        case 'B':
            pieces.push_back(std::make_unique<Bishop>(x, y, textures[type], isWhite));
            break;
        case 'Q':
            pieces.push_back(std::make_unique<Queen>(x, y, textures[type], isWhite));
            break;
        case 'K':
            pieces.push_back(std::make_unique<King>(x, y, textures[type], isWhite));
            break;
    }
}

#endif