#ifndef PAWN_H
#define PAWN_H

#include "Piece.h"

class Pawn : public Piece {
public:
    Pawn(int xPos, int yPos, Texture2D tex, bool white) 
        : Piece(xPos, yPos, tex, white) {}

    std::vector<Vector2> GetValidMoves(const Game& game) const override {
        std::vector<Vector2> moves;
        int direction = isWhite ? -1 : 1;  // White moves up, Black moves down
        int startRank = isWhite ? 6 : 1;

        // Forward move
        int newY = y + direction;
        if (IsValidPosition(x, newY) && !game.GetPieceAt(x, newY)) {
            moves.push_back({(float)x, (float)newY});
            if (y == startRank && !game.GetPieceAt(x, y + 2 * direction)) {
                moves.push_back({(float)x, (float)(y + 2 * direction)});
            }
        }

        // Captures
        for (int dx : {-1, 1}) {
            int newX = x + dx;
            if (IsValidPosition(newX, newY)) {
                const Piece* target = game.GetPieceAt(newX, newY);
                if (target && target->IsWhite() != isWhite) {
                    moves.push_back({(float)newX, (float)newY});
                }
            }
        }

        return moves;
    }
};

#endif