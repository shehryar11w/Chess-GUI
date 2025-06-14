
#ifndef KING_H
#define KING_H

#include "Piece.h"
#include "Game.h"
using namespace std;
class King : public Piece {
public:
    King(int xPos, int yPos, Texture2D tex, bool white) 
        : Piece(xPos, yPos, tex, white, PieceType::KING) {}

    vector<Vector2> GetValidMoves(const Game& game) const override {
        vector<Vector2> moves;
        for (auto [dx, dy] : vector<pair<int, int>>{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                                              {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}) {
            int newX = x + dx;
            int newY = y + dy;
            if (IsValidPosition(newX, newY)) {
                const Piece* target = game.GetPieceAt(newX, newY);
                if (!target || target->IsWhite() != isWhite) {
                    moves.push_back({(float)newX, (float)newY});
                }
            }
        }
        return moves;
    }
};

#endif

