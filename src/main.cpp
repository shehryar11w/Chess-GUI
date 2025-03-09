#include "raylib.h"
#include <vector>
#include <iostream>
#include <map>
#include <cctype>
#include <iostream>

const int TILE_SIZE = 80;
const int BOARD_SIZE = 8;
const Color LIGHT_SQUARE = RAYWHITE;
const Color DARK_SQUARE = DARKGRAY;
const Color MOVE_HIGHLIGHT = GREEN;

struct Piece {
    int x, y;
    Texture2D texture;
    bool isWhite;
};

std::vector<Piece> pieces;
Piece* selectedPiece = nullptr;
std::vector<Vector2> validMoves;
std::map<char, Texture2D> whiteTextures, blackTextures;

void LoadTextures() {
    whiteTextures['P'] = LoadTexture("assets/white_pawn.png");
    whiteTextures['R'] = LoadTexture("assets/white_rook.png");
    whiteTextures['N'] = LoadTexture("assets/white_knight.png");
    whiteTextures['B'] = LoadTexture("assets/white_bishop.png");
    whiteTextures['Q'] = LoadTexture("assets/white_queen.png");
    whiteTextures['K'] = LoadTexture("assets/white_king.png");
    
    blackTextures['P'] = LoadTexture("assets/black_pawn.png");
    blackTextures['R'] = LoadTexture("assets/black_rook.png");
    blackTextures['N'] = LoadTexture("assets/black_knight.png");
    blackTextures['B'] = LoadTexture("assets/black_bishop.png");
    blackTextures['Q'] = LoadTexture("assets/black_queen.png");
    blackTextures['K'] = LoadTexture("assets/black_king.png");
}

void SetupBoard() {
    const char* layout[8] = {
        "RNBQKBNR",
        "PPPPPPPP",
        "........",
        "........",
        "........",
        "........",
        "pppppppp",
        "rnbqkbnr"
    };

    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            char c = layout[y][x];
            if (c != '.') {
                bool isWhite = (c < 'a'); // White pieces are uppercase
                Texture2D texture = isWhite ? whiteTextures[toupper(c)] : blackTextures[toupper(c)];
                pieces.push_back({ x, y, texture, isWhite });
            }
        }
    }
}

void DrawBoard() {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            Color squareColor = (row + col) % 2 == 0 ? LIGHT_SQUARE : DARK_SQUARE;
            DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, squareColor);
        }
    }
}

std::vector<Vector2> GetValidMoves(Piece* piece) {
    std::vector<Vector2> moves;
    if (!piece) return moves;

    int x = piece->x;
    int y = piece->y;

    // Pawn movement logic
    int direction = piece->isWhite ? -1 : 1;
    if (y + direction >= 0 && y + direction < BOARD_SIZE) {
        bool isBlocked = false;
        for (auto& p : pieces) {
            if (p.x == x && p.y == y + direction) {
                isBlocked = true;
                break;
            }
        }
        if (!isBlocked) {
            moves.push_back({ (float)x, (float)(y + direction) });
        }
    }

    return moves;
}

void SelectPiece(int x, int y) {
    selectedPiece = nullptr;
    validMoves.clear();
    for (auto& piece : pieces) {
        if (piece.x == x && piece.y == y) {
            selectedPiece = &piece;
            std::cout << "selected piece " << piece.x <<piece.y <<piece.isWhite << std::endl;
            validMoves = GetValidMoves(selectedPiece);
            return;
        }
    }
}

void MovePiece(int x, int y) {
    if (!selectedPiece) return;

    for (auto move : validMoves) {
        if ((int)move.x == x && (int)move.y == y) {
            selectedPiece->x = x;
            selectedPiece->y = y;
            selectedPiece = nullptr;
            validMoves.clear();
            return;
        }
    }
}

int main() {
    InitWindow(BOARD_SIZE * TILE_SIZE, BOARD_SIZE * TILE_SIZE, "Chess with Raylib");
    SetTargetFPS(60);
    
    LoadTextures();
    SetupBoard();
    
    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int x = GetMouseX() / TILE_SIZE;
            int y = GetMouseY() / TILE_SIZE;
            if (selectedPiece) MovePiece(x, y);
            else SelectPiece(x, y);
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBoard();
        
        // Highlight valid moves
        for (auto move : validMoves) {
            DrawCircle(move.x * TILE_SIZE + TILE_SIZE / 2, move.y * TILE_SIZE + TILE_SIZE / 2, 10, MOVE_HIGHLIGHT);
        }

        // Draw pieces
        for (auto& piece : pieces) {
            DrawTexture(piece.texture, piece.x * TILE_SIZE, piece.y * TILE_SIZE, WHITE);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
