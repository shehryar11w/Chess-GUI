#include "Team.h"
#include "Piece.h"
#include "Pawn.h"
#include "King.h"
#include "Knight.h"
#include "Bishop.h"
#include "Queen.h"
#include "Rook.h"
#include "TextureManager.h"
#include <cctype>
#include <string>
#include <iostream>
using namespace std;
Team::Team(bool isWhiteTeam) : isWhite(isWhiteTeam)
{
    SetupPieces();
}

Team::~Team()
{
    pieces.clear();
}

Piece *Team::FindPieceAt(int x, int y)
{
    for (auto &piece : pieces)
    {
        if (piece->GetX() == x && piece->GetY() == y)
        {
            return piece.get();
        }
    }
    return nullptr;
}

void Team::RemovePieceAt(int x, int y)
{
    auto it = pieces.begin();
    while (it != pieces.end())
    {
        if ((*it)->GetX() == x && (*it)->GetY() == y)
        {
            it = pieces.erase(it);
            return;
        }
        ++it;
    }
}

void Team::SetupPieces()
{
     string color = isWhite ? "white" : "black";
    auto *texManager = TextureManager::GetInstance();

    // Load all piece textures
    const char *pieceTypes[] = {"pawn", "rook", "knight", "bishop", "queen", "king"};
    const char pieceChars[] = "PRNBQK";

    for (int i = 0; i < 6; i++)
    {
         string key = color + "_" + pieceTypes[i];
         string filepath = "assets/" + key + ".png";
        Texture2D texture = texManager->LoadTextureFromFile(key, filepath);

        if (texture.id == 0) 
        {
             cerr << "ERROR: Texture failed to load: " << filepath <<  endl;
        }
        else
        {
             cout << "Successfully loaded: " << filepath <<  endl;
        }
    }

    if (isWhite)
    {
        for (int x = 0; x < 8; x++)
            AddPiece(x, 6, 'P');
        AddPiece(0, 7, 'R');
        AddPiece(1, 7, 'N');
        AddPiece(2, 7, 'B');
        AddPiece(3, 7, 'Q');
        AddPiece(4, 7, 'K');
        AddPiece(5, 7, 'B');
        AddPiece(6, 7, 'N');
        AddPiece(7, 7, 'R');
    }
    else
    {
        for (int x = 0; x < 8; x++)
            AddPiece(x, 1, 'P');
        AddPiece(0, 0, 'R');
        AddPiece(1, 0, 'N');
        AddPiece(2, 0, 'B');
        AddPiece(3, 0, 'Q');
        AddPiece(4, 0, 'K');
        AddPiece(5, 0, 'B');
        AddPiece(6, 0, 'N');
        AddPiece(7, 0, 'R');
    }
}

void Team::AddPiece(int x, int y, char type)
{
    auto *texManager = TextureManager::GetInstance();
     string color = isWhite ? "white" : "black";
    char upperType =  toupper(type);
     string texKey;

    switch (upperType)
    {
    case 'P':
        texKey = color + "_pawn";
        break;
    case 'R':
        texKey = color + "_rook";
        break;
    case 'N':
        texKey = color + "_knight";
        break;
    case 'B':
        texKey = color + "_bishop";
        break;
    case 'Q':
        texKey = color + "_queen";
        break;
    case 'K':
        texKey = color + "_king";
        break;
    default:
        return;
    }

    Texture2D tex = texManager->GetTexture(texKey);
    if (tex.id == 0)
        return; 

    switch (upperType)
    {
    case 'P':
        pieces.push_back( make_unique<Pawn>(x, y, tex, isWhite));
        break;
    case 'R':
        pieces.push_back( make_unique<Rook>(x, y, tex, isWhite));
        break;
    case 'N':
        pieces.push_back( make_unique<Knight>(x, y, tex, isWhite));
        break;
    case 'B':
        pieces.push_back( make_unique<Bishop>(x, y, tex, isWhite));
        break;
    case 'Q':
        pieces.push_back( make_unique<Queen>(x, y, tex, isWhite));
        break;
    case 'K':
        pieces.push_back( make_unique<King>(x, y, tex, isWhite));
        break;
    }
}

void Team::AddPiece(PieceType type, int x, int y)
{
    auto* texManager = TextureManager::GetInstance();
     string color = isWhite ? "white" : "black";
     string texKey;

    switch (type)
    {
    case PieceType::QUEEN:
        texKey = color + "_queen";
        break;
    case PieceType::ROOK:
        texKey = color + "_rook";
        break;
    case PieceType::BISHOP:
        texKey = color + "_bishop";
        break;
    case PieceType::KNIGHT:
        texKey = color + "_knight";
        break;
    default:
        return;
    }

    Texture2D tex = texManager->GetTexture(texKey);
    if (tex.id == 0)
        return; 

    switch (type)
    {
    case PieceType::QUEEN:
        pieces.push_back( make_unique<Queen>(x, y, tex, isWhite));
        break;
    case PieceType::ROOK:
        pieces.push_back( make_unique<Rook>(x, y, tex, isWhite));
        break;
    case PieceType::BISHOP:
        pieces.push_back( make_unique<Bishop>(x, y, tex, isWhite));
        break;
    case PieceType::KNIGHT:
        pieces.push_back( make_unique<Knight>(x, y, tex, isWhite));
        break;
    }
}

void Team::Reset() {
    // Clear all pieces
    pieces.clear();
    Team::SetupPieces();
}
