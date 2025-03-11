#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Team.h"
#include <vector>

class Game {
private:
    static const int TILE_SIZE = 80;
    static const int BOARD_SIZE = 8;
    static const Color LIGHT_SQUARE;
    static const Color DARK_SQUARE;
    static const Color MOVE_HIGHLIGHT;

    Team whiteTeam;
    Team blackTeam;
    Piece* selectedPiece = nullptr;
    std::vector<Vector2> validMoves;
    bool isWhiteTurn = true;
    bool boardRotated = false;

public:
    Game() : whiteTeam(true), blackTeam(false) {
        InitWindow(BOARD_SIZE * TILE_SIZE, BOARD_SIZE * TILE_SIZE + 60, "Chess with Raylib");
        SetTargetFPS(60);
    }

    ~Game() { CloseWindow(); }

    void Run() {
        while (!WindowShouldClose()) {
            HandleInput();
            Draw();
        }
    }

    void DrawBoard() {
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                Vector2 pos = BoardToScreen(col, row);
                Color squareColor = (row + col) % 2 == 0 ? LIGHT_SQUARE : DARK_SQUARE;
                DrawRectangle(pos.x, pos.y, TILE_SIZE, TILE_SIZE, squareColor);
            }
        }
        for (int col = 0; col < BOARD_SIZE; col++) {
            char colLabel = boardRotated ? ('h' - col) : ('a' + col);
            char label[2] = {colLabel, '\0'};
            float x = col * TILE_SIZE;
            if (boardRotated) x = (BOARD_SIZE - 1 - col) * TILE_SIZE;
            int textWidth = MeasureText(label, 20);
            DrawText(label, x + (TILE_SIZE - textWidth) / 2, BOARD_SIZE * TILE_SIZE + 25, 20, BLACK);
        }
        for (int row = 0; row < BOARD_SIZE; row++) {
            char rowLabel = boardRotated ? ('8' - row) : ('1' + row);
            char label[2] = {rowLabel, '\0'};
            float y = row * TILE_SIZE;
            if (boardRotated) y = (BOARD_SIZE - 1 - row) * TILE_SIZE;
            DrawText(label, 5, y + (TILE_SIZE - 20) / 2, 20, BLACK);
        }
    }

    Vector2 ScreenToBoard(Vector2 screenPos) {
        int x = (int)(screenPos.x / TILE_SIZE);
        int y = (int)(screenPos.y / TILE_SIZE);
        if (boardRotated) {
            x = BOARD_SIZE - 1 - x;
            y = BOARD_SIZE - 1 - y;
        }
        return {(float)x, (float)y};
    }

    Vector2 BoardToScreen(int x, int y) {
        float centerX = BOARD_SIZE * TILE_SIZE / 2.0f;
        float centerY = BOARD_SIZE * TILE_SIZE / 2.0f;
        float relX = (x + 0.5f) * TILE_SIZE - centerX;
        float relY = (y + 0.5f) * TILE_SIZE - centerY;
        if (boardRotated) {
            relX = -relX;
            relY = -relY;
        }
        return {relX + centerX - TILE_SIZE/2, relY + centerY - TILE_SIZE/2};
    }

    Vector2 GetCenteredPiecePosition(const Piece& piece) {
        float pieceWidth = piece.GetTexture().width;
        float pieceHeight = piece.GetTexture().height;
        float offsetX = (TILE_SIZE - pieceWidth) / 2;
        float offsetY = (TILE_SIZE - pieceHeight) / 2;
        Vector2 pos = BoardToScreen(piece.GetX(), piece.GetY());
        return {pos.x + offsetX, pos.y + offsetY};
    }

    void SelectPiece(int x, int y) {
        const Piece* clickedPiece = GetPieceAt(x, y);
        if (selectedPiece && clickedPiece && selectedPiece == clickedPiece) {
            selectedPiece = nullptr;
            validMoves.clear();
            return;
        }
        selectedPiece = nullptr;
        validMoves.clear();

        // Need to get a non-const pointer to the piece
        Piece* mutablePiece = nullptr;
        if (clickedPiece) {
            if (clickedPiece->IsWhite()) {
                for (auto& piece : whiteTeam.GetPieces()) {
                    if (piece.get() == clickedPiece) {
                        mutablePiece = piece.get();
                        break;
                    }
                }
            } else {
                for (auto& piece : blackTeam.GetPieces()) {
                    if (piece.get() == clickedPiece) {
                        mutablePiece = piece.get();
                        break;
                    }
                }
            }

            if (mutablePiece && mutablePiece->IsWhite() == isWhiteTurn) {
                selectedPiece = mutablePiece;
                validMoves = GetValidMoves(selectedPiece);
            }
        }
    }


    void MovePiece(int x, int y) {
        if (!selectedPiece || selectedPiece->IsWhite() != isWhiteTurn) {
            selectedPiece = nullptr;
            validMoves.clear();
            return;
        }
        for (const auto& move : validMoves) {
            if ((int)move.x == x && (int)move.y == y) {
                Team& opponentTeam = isWhiteTurn ? blackTeam : whiteTeam;
                opponentTeam.RemovePieceAt(x, y);
                selectedPiece->SetPosition(x, y);
                isWhiteTurn = !isWhiteTurn;
                boardRotated = !boardRotated;
                selectedPiece = nullptr;
                validMoves.clear();
                break;
            }
        }
    }

    std::vector<Vector2> GetValidMoves(Piece* piece) {
        if (!piece) return {};
        return piece->GetValidMoves(*this);
    }

    const Team& GetWhiteTeam() const { return whiteTeam; }
    const Team& GetBlackTeam() const { return blackTeam; }

    // Moved from Piece.h
    const Piece* GetPieceAt(int x, int y) const {
        for (const auto& piece : whiteTeam.GetPieces()) {
            if (piece->GetX() == x && piece->GetY() == y) {
                return piece.get();
            }
        }
        for (const auto& piece : blackTeam.GetPieces()) {
            if (piece->GetX() == x && piece->GetY() == y) {
                return piece.get();
            }
        }
        return nullptr;
    }

private:
    void HandleInput() {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            if (mousePos.y < BOARD_SIZE * TILE_SIZE) {
                Vector2 boardPos = ScreenToBoard(mousePos);
                int x = (int)boardPos.x;
                int y = (int)boardPos.y;
                if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                    if (selectedPiece) {
                        bool isValidMove = false;
                        for (const auto& move : validMoves) {
                            if ((int)move.x == x && (int)move.y == y) {
                                isValidMove = true;
                                break;
                            }
                        }
                        if (isValidMove) {
                            MovePiece(x, y);
                        } else {
                            SelectPiece(x, y);
                        }
                    } else {
                        SelectPiece(x, y);
                    }
                }
            }
        }
    }

    void Draw() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBoard();

        if (selectedPiece) {
            Vector2 pos = BoardToScreen(selectedPiece->GetX(), selectedPiece->GetY());
            DrawRectangle(pos.x, pos.y, TILE_SIZE, TILE_SIZE, ColorAlpha(YELLOW, 0.5f));
            for (const auto& move : validMoves) {
                auto& opponentPieces = isWhiteTurn ? blackTeam.GetPieces() : whiteTeam.GetPieces();
                for (const auto& piece : opponentPieces) {
                    if (piece->GetX() == (int)move.x && piece->GetY() == (int)move.y) {
                        Vector2 capturePos = BoardToScreen((int)move.x, (int)move.y);
                        DrawRectangle(capturePos.x, capturePos.y, TILE_SIZE, TILE_SIZE, ColorAlpha(RED, 0.5f));
                        break;
                    }
                }
            }
        }

        for (auto move : validMoves) {
            Vector2 pos = BoardToScreen((int)move.x, (int)move.y);
            DrawCircle(pos.x + TILE_SIZE / 2, pos.y + TILE_SIZE / 2, 10, MOVE_HIGHLIGHT);
        }

        for (const auto& piece : whiteTeam.GetPieces()) {
            Vector2 pos = GetCenteredPiecePosition(*piece);
            DrawTexture(piece->GetTexture(), pos.x, pos.y, WHITE);
        }
        for (const auto& piece : blackTeam.GetPieces()) {
            Vector2 pos = GetCenteredPiecePosition(*piece);
            DrawTexture(piece->GetTexture(), pos.x, pos.y, WHITE);
        }

        DrawText(isWhiteTurn ? "White's Turn" : "Black's Turn", 10, BOARD_SIZE * TILE_SIZE + 10, 20, BLACK);
        EndDrawing();
    }
};

const Color Game::LIGHT_SQUARE = RAYWHITE;
const Color Game::DARK_SQUARE = DARKGRAY;
const Color Game::MOVE_HIGHLIGHT = GREEN;

#endif