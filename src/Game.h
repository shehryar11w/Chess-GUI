#ifndef GAME_H
#define GAME_H

#include "Forward.h"
#include "raylib.h"
#include <vector>

// Include the complete definitions needed in the header
#include "Team.h"
#include "Piece.h"

class Game {
private:
    static const int TILE_SIZE = 80;
    static const int BOARD_SIZE = 8;
    static const Color LIGHT_SQUARE;
    static const Color DARK_SQUARE;
    static const Color MOVE_HIGHLIGHT;

    Team whiteTeam;
    Team blackTeam;
    Piece* selectedPiece;
    std::vector<Vector2> validMoves;
    bool isWhiteTurn;
    bool boardRotated;

public:
    Game();
    ~Game();

    void Run();
    void DrawBoard();
    Vector2 ScreenToBoard(Vector2 screenPos);
    Vector2 BoardToScreen(int x, int y);
    Vector2 GetCenteredPiecePosition(const Piece& piece);
    void SelectPiece(int x, int y);
    void MovePiece(int x, int y);
    std::vector<Vector2> GetValidMoves(Piece* piece);
    const Team& GetWhiteTeam() const;
    const Team& GetBlackTeam() const;
    const Piece* GetPieceAt(int x, int y) const;
    bool IsSquareUnderAttack(int x, int y, bool byWhite) const;
    void ToggleBoardRotation();
    void DrawLabels();
    void DrawMenu();

private:
    void HandleInput();
    void Draw();
};

#endif



