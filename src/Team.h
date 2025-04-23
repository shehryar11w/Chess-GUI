#ifndef TEAM_H
#define TEAM_H

#include "Forward.h"
#include <vector>
#include <memory>
#include <string>
#include "raylib.h"
#include "Piece.h"
using namespace std;
class Team {
private:
    bool isWhite;
     vector< unique_ptr<Piece>> pieces;

public:
    Team(bool isWhiteTeam);
    ~Team();

    const  vector< unique_ptr<Piece>>& GetPieces() const { return pieces; }
    Piece* FindPieceAt(int x, int y);
    void RemovePieceAt(int x, int y);
    void AddPiece(PieceType type, int x, int y);  
    void Reset();  

private:
    void SetupPieces();
    void AddPiece(int x, int y, char type);  
};

#endif

