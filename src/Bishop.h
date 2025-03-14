#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.h"
#include "Game.h"
#include <utility>

class Bishop : public Piece {
public:
    Bishop(int xPos, int yPos, Texture2D tex, bool white) 
        : Piece(xPos, yPos, tex, white, PieceType::BISHOP) {}

    std::vector<Vector2> GetValidMoves(const Game& game) const override {
        std::vector<Vector2> moves;
        const std::vector<std::pair<int, int>> directions = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};
        
        for (const auto& [dx, dy] : directions) {
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
                    break;
                }
            }
        }
        return moves;
    }
};

#endif

