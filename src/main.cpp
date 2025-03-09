#include "raylib.h"
#include <vector>
#include <iostream>
#include <map>

const int SQUARE_SIZE = 80;
const int BOARD_SIZE = 8;

struct Piece {
    char type; // 'P', 'R', 'N', 'B', 'Q', 'K'
    bool isWhite;
    int x, y;
};

std::vector<Piece> pieces;
Piece* selectedPiece = nullptr;

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

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            char c = layout[y][x];
            if (c != '.') {
                Piece piece;
                piece.type = toupper(c);
                piece.isWhite = isupper(c);
                piece.x = x;
                piece.y = y;
                pieces.push_back(piece);
            }
        }
    }
}

void DrawBoard() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            Color color = ((x + y) % 2 == 0) ? LIGHTGRAY : DARKGRAY;
            DrawRectangle(x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
        }
    }
}

void DrawPieces() {
    for (const auto& piece : pieces) {
        Texture2D texture = piece.isWhite ? whiteTextures[piece.type] : blackTextures[piece.type];
        DrawTexture(texture, piece.x * SQUARE_SIZE, piece.y * SQUARE_SIZE, WHITE);
    }
}

Piece* GetPieceAt(int x, int y) {
    for (auto& piece : pieces) {
        if (piece.x == x && piece.y == y) {
            return &piece;
        }
    }
    return nullptr;
}

void MovePiece(Piece* piece, int newX, int newY) {
    Piece* target = GetPieceAt(newX, newY);
    if (target && target->isWhite == piece->isWhite) return; // Can't capture same color
    
    piece->x = newX;
    piece->y = newY;
}

void HandleMouseInput() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int mouseX = GetMouseX() / SQUARE_SIZE;
        int mouseY = GetMouseY() / SQUARE_SIZE;
        
        if (selectedPiece) {
            MovePiece(selectedPiece, mouseX, mouseY);
            selectedPiece = nullptr;
        } else {
            selectedPiece = GetPieceAt(mouseX, mouseY);
        }
    }
}

int main() {
    InitWindow(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE, "Raylib Chess");
    LoadTextures();
    SetupBoard();
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawBoard();
        DrawPieces();
        HandleMouseInput();
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
