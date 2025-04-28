#ifndef KNIGHT_H
#define KNIGHT_H

#include "Piece.h"
#include "Game.h"
using namespace std;
class Knight : public Piece {
public:
    Knight(int xPos, int yPos, Texture2D tex, bool white) 
        : Piece(xPos, yPos, tex, white, PieceType::KNIGHT) {}

     vector<Vector2> GetValidMoves(const Game& game) const override {
         vector<Vector2> moves;
        for (auto [dx, dy] :  vector< pair<int, int>>{{1, 2}, {2, 1}, {2, -1}, {1, -2},
                                                              {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}) {
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

