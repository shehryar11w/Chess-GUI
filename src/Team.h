#ifndef TEAM_H
#define TEAM_H

#include "Forward.h"
#include <vector>
#include <memory>
#include <string>
#include "raylib.h"

class Team {
private:
    bool isWhite;
    std::vector<std::unique_ptr<Piece>> pieces;

public:
    Team(bool isWhiteTeam);
    ~Team();

    const std::vector<std::unique_ptr<Piece>>& GetPieces() const { return pieces; }
    Piece* FindPieceAt(int x, int y);
    void RemovePieceAt(int x, int y);

private:
    void SetupPieces();
    void AddPiece(int x, int y, char type);
};

#endif
