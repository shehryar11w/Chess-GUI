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
bool isWhiteTurn = true; // White moves first

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
        "rnbqkbnr",  // Black pieces (lowercase)
        "pppppppp",  // Black pawns
        "........",
        "........",
        "........",
        "........",
        "PPPPPPPP",  // White pawns
        "RNBQKBNR"   // White pieces (uppercase)
    };

    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            char c = layout[y][x];
            if (c != '.') {
                bool isWhite = std::isupper(c);  // White pieces are uppercase
                Texture2D texture = isWhite ? whiteTextures[toupper(c)] : blackTextures[toupper(c)];
                pieces.push_back({ x, y, texture, isWhite });
            }
        }
    }
}

void DrawBoard() {
    // Draw squares
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            // Draw square
            Color squareColor = (row + col) % 2 == 0 ? LIGHT_SQUARE : DARK_SQUARE;
            DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, squareColor);
        }
    }

    // Draw column letters (a-h) under the pawns
    for (int col = 0; col < BOARD_SIZE; col++) {
        char colLabel = 'a' + col;
        char label[2] = {colLabel, '\0'};
        int textWidth = MeasureText(label, 20);
        int textX = col * TILE_SIZE + (TILE_SIZE - textWidth) / 2;
        int textY = BOARD_SIZE * TILE_SIZE + 25;  // Moved down to 25 to avoid overlap
        DrawText(label, textX, textY, 20, BLACK);
    }

    // Draw row numbers (1-8) vertically on the left
    for (int row = 0; row < BOARD_SIZE; row++) {
        char rowLabel = '8' - row;
        char label[2] = {rowLabel, '\0'};
        int textWidth = MeasureText(label, 20);
        int textX = 5;
        int textY = row * TILE_SIZE + (TILE_SIZE - 20) / 2;
        DrawText(label, textX, textY, 20, BLACK);
    }
}

char GetPieceType(const Piece* piece) {
    for (const auto& pair : piece->isWhite ? whiteTextures : blackTextures) {
        if (pair.second.id == piece->texture.id) {
            return pair.first;
        }
    }
    return ' ';
}

std::vector<Vector2> GetValidMoves(Piece* piece) {
    std::vector<Vector2> moves;
    if (!piece) return moves;

    int x = piece->x;
    int y = piece->y;
    
    char pieceType = GetPieceType(piece);

    // Helper lambda to check if a position is within board bounds
    auto isValidPosition = [](int x, int y) {
        return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
    };

    // Helper lambda to check if a position is occupied by a piece
    auto getPieceAt = [](int x, int y) -> Piece* {
        for (auto& p : pieces) {
            if (p.x == x && p.y == y) {
                return &p;
            }
        }
        return nullptr;
    };

    // Helper lambda to try to add a move, returns false if blocked
    auto tryAddMove = [&](int newX, int newY) -> bool {
        if (!isValidPosition(newX, newY)) return false;
        Piece* targetPiece = getPieceAt(newX, newY);
        if (!targetPiece || targetPiece->isWhite != piece->isWhite) {
            moves.push_back({(float)newX, (float)newY});
        }
        return targetPiece == nullptr; // Return true if space is empty (not blocked)
    };

    switch (toupper(pieceType)) {
        case 'P': { // Pawn
            int direction = piece->isWhite ? -1 : 1;  // White moves up (-1), Black moves down (+1)
            
            // Forward move one square
            int newY = y + direction;
            if (isValidPosition(x, newY) && !getPieceAt(x, newY)) {
                moves.push_back({(float)x, (float)newY});
                
                // Initial two-square move
                int startRank = piece->isWhite ? 6 : 1;  // White starts on rank 6, Black on rank 1
                if (y == startRank) {
                    int twoSquaresY = y + (2 * direction);
                    if (isValidPosition(x, twoSquaresY) && !getPieceAt(x, twoSquaresY)) {
                        moves.push_back({(float)x, (float)twoSquaresY});
                    }
                }
            }
            
            // Capture moves (diagonally)
            for (int dx : {-1, 1}) {
                int newX = x + dx;
                if (isValidPosition(newX, newY)) {
                    Piece* targetPiece = getPieceAt(newX, newY);
                    if (targetPiece && targetPiece->isWhite != piece->isWhite) {
                        moves.push_back({(float)newX, (float)newY});
                    }
                }
            }
            break;
        }
        case 'R': { // Rook
            // Move horizontally and vertically
            for (auto [dx, dy] : std::vector<std::pair<int, int>>{{0, 1}, {1, 0}, {0, -1}, {-1, 0}}) {
                for (int i = 1; i < BOARD_SIZE; i++) {
                    if (!tryAddMove(x + dx * i, y + dy * i)) break;
                }
            }
            break;
        }
        case 'N': { // Knight
            // L-shaped moves
            for (auto [dx, dy] : std::vector<std::pair<int, int>>{{1, 2}, {2, 1}, {2, -1}, {1, -2},
                                                                 {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}) {
                tryAddMove(x + dx, y + dy);
            }
            break;
        }
        case 'B': { // Bishop
            // Move diagonally
            for (auto [dx, dy] : std::vector<std::pair<int, int>>{{1, 1}, {1, -1}, {-1, -1}, {-1, 1}}) {
                for (int i = 1; i < BOARD_SIZE; i++) {
                    if (!tryAddMove(x + dx * i, y + dy * i)) break;
                }
            }
            break;
        }
        case 'Q': { // Queen (combination of Rook and Bishop moves)
            // Move in all directions
            for (auto [dx, dy] : std::vector<std::pair<int, int>>{{0, 1}, {1, 0}, {0, -1}, {-1, 0},
                                                                 {1, 1}, {1, -1}, {-1, -1}, {-1, 1}}) {
                for (int i = 1; i < BOARD_SIZE; i++) {
                    if (!tryAddMove(x + dx * i, y + dy * i)) break;
                }
            }
            break;
        }
        case 'K': { // King
            // Move one square in any direction
            for (auto [dx, dy] : std::vector<std::pair<int, int>>{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                                                 {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}) {
                tryAddMove(x + dx, y + dy);
            }
            break;
        }
    }

    return moves;
}

void SelectPiece(int x, int y) {
    // First, check if we're clicking on a piece
    Piece* clickedPiece = nullptr;
    for (auto& piece : pieces) {
        if (piece.x == x && piece.y == y) {
            clickedPiece = &piece;
            break;
        }
    }

    // If clicking on the same piece that's already selected, deselect it
    if (selectedPiece && clickedPiece && 
        selectedPiece->x == x && 
        selectedPiece->y == y) {
        selectedPiece = nullptr;
        validMoves.clear();
        return;
    }

    // If clicking on a piece of the current player's color, select it
    if (clickedPiece && clickedPiece->isWhite == isWhiteTurn) {
        selectedPiece = clickedPiece;
        validMoves = GetValidMoves(selectedPiece);
        return;
    }

    // If clicking on an empty square or opponent's piece, deselect current piece
    selectedPiece = nullptr;
    validMoves.clear();
}

void MovePiece(int x, int y) {
    if (!selectedPiece) return;
    
    // Check if it's the correct player's turn
    if (selectedPiece->isWhite != isWhiteTurn) {
        selectedPiece = nullptr;
        validMoves.clear();
        return;
    }

    for (const auto& move : validMoves) {
        if ((int)move.x == x && (int)move.y == y) {
            // Check if there's a piece to capture
            auto it = pieces.begin();
            while (it != pieces.end()) {
                if (it->x == x && it->y == y) {
                    // Remove the captured piece
                    it = pieces.erase(it);
                    break;
                }
                ++it;
            }
            
            // Move the selected piece
            selectedPiece->x = x;
            selectedPiece->y = y;
            
            // Switch turns
            isWhiteTurn = !isWhiteTurn;
            
            // Clear selection
            selectedPiece = nullptr;
            validMoves.clear();
            break;
        }
    }
}

// Add this function to calculate centered position for pieces
Vector2 GetCenteredPiecePosition(const Piece& piece) {
    // Get the texture dimensions
    float pieceWidth = piece.texture.width;
    float pieceHeight = piece.texture.height;
    
    // Calculate the offset to center the piece in the tile
    float offsetX = (TILE_SIZE - pieceWidth) / 2;
    float offsetY = (TILE_SIZE - pieceHeight) / 2;
    
    // Return the centered position
    return {
        piece.x * TILE_SIZE + offsetX,
        piece.y * TILE_SIZE + offsetY
    };
}

int main() {
    InitWindow(BOARD_SIZE * TILE_SIZE, BOARD_SIZE * TILE_SIZE + 60, "Chess with Raylib");  // Increased height to 60
    SetTargetFPS(60);
    
    LoadTextures();
    SetupBoard();
    
    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            // Ensure we're only processing clicks on the board
            if (mousePos.y < BOARD_SIZE * TILE_SIZE) {
                int x = (int)(mousePos.x / TILE_SIZE);
                int y = (int)(mousePos.y / TILE_SIZE);
                
                if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                    if (selectedPiece) MovePiece(x, y);
                    else SelectPiece(x, y);
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBoard();
        
        // Highlight selected piece
        if (selectedPiece) {
            DrawRectangle(
                selectedPiece->x * TILE_SIZE,
                selectedPiece->y * TILE_SIZE,
                TILE_SIZE, TILE_SIZE,
                ColorAlpha(YELLOW, 0.5f)
            );
        }
        
        // Highlight valid moves
        for (auto move : validMoves) {
            DrawCircle(
                move.x * TILE_SIZE + TILE_SIZE / 2,
                move.y * TILE_SIZE + TILE_SIZE / 2,
                10, MOVE_HIGHLIGHT
            );
        }

        // Draw pieces with centering
        for (const auto& piece : pieces) {
            Vector2 pos = GetCenteredPiecePosition(piece);
            DrawTexture(piece.texture, pos.x, pos.y, WHITE);
        }
        
        // Draw turn indicator
        DrawText(isWhiteTurn ? "White's Turn" : "Black's Turn", 10, BOARD_SIZE * TILE_SIZE + 10, 20, BLACK);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}



