#ifndef GAME_H
#define GAME_H

#include "Forward.h"
#include "raylib.h"
#include <vector>
#include "Team.h"
#include "Piece.h"
using namespace std;
enum GameState {
    MENU,
    PLAY,
    PROMOTION,
    GAME_OVER
};

struct Move {
    Vector2 start;
    Vector2 end;
    Piece* piece;
};

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
    vector<Vector2> validMoves;
    bool isWhiteTurn;
    bool boardRotated;
    bool namesRotated;
    Move lastMove;
    Vector2 promotionSquare;
    GameState currentState;
    Sound moveSound;
    Sound captureSound;
    Sound checkSound;
    Sound promotionSound;
    Texture2D backgroundTexture;

   
    vector<PieceType> whiteCapturedPieces;
    vector<PieceType> blackCapturedPieces;

    // Add captured pieces tracking
    std::vector<PieceType> whiteCapturedPieces;
    std::vector<PieceType> blackCapturedPieces;

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
    vector<Vector2> GetValidMoves(Piece* piece);
    const Team& GetWhiteTeam() const;
    const Team& GetBlackTeam() const;
    const Piece* GetPieceAt(int x, int y) const;
    bool IsSquareUnderAttack(int x, int y, bool byWhite, Vector2 ignorePiecePos = {-1, -1}) const;
    void ToggleBoardRotation();
    void DrawLabels();
    void DrawMenu();
    Move GetLastMove() const { return lastMove; }
    void DrawPromotionUI();
    void PromotePawn(PieceType type);
    GameState GetGameState() const { return currentState; }
    void SetGameState(GameState state) { currentState = state; }

    void AddCapturedPiece(PieceType type, bool isWhite);
    const vector<PieceType>& GetWhiteCapturedPieces() const;
    const vector<PieceType>& GetBlackCapturedPieces() const;

private:
    void HandleInput();
    void Draw();
    bool IsCheckmate(bool isWhite);
    bool IsStalemate(bool isWhite);
    void DrawGameOverUI();
    bool shouldClose = false; 

};

#endif


























