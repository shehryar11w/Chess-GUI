#ifndef ROOK_H
#define ROOK_H

#include "Piece.h"
#include "Game.h"

class Rook : public Piece {
public:
    Rook(int xPos, int yPos, Texture2D tex, bool white) 
        : Piece(xPos, yPos, tex, white) {}

    std::vector<Vector2> GetValidMoves(const Game& game) const override {
        std::vector<Vector2> moves;
        for (auto [dx, dy] : std::vector<std::pair<int, int>>{{0, 1}, {1, 0}, {0, -1}, {-1, 0}}) {
            for (int i = 1; i < 8; i++) {
                int newX = x + dx * i;
                int newY = y + dy * i;
                if (!IsValidPosition(newX, newY)) break;
                const Piece* target = game.GetPieceAt(newX, newY);
                if (!target) {
                    moves.push_back({(float)newX, (float)newY});
                } else {
                    if (target->IsWhite() != isWhite) {
                        moves.push_back({(float)newX, (float)newY});
                    }
                    break;  // Blocked by piece
                }
            }
        }
        return moves;
    }
};

#endif
