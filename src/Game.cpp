#include "Game.h"
#include "Piece.h"
#include "Team.h"
#include "TextureManager.h"
#include <cmath>  // For floorf
#include <iostream>
// Add static member definitions
const Color Game::LIGHT_SQUARE = RAYWHITE;
const Color Game::DARK_SQUARE = DARKGRAY;
const Color Game::MOVE_HIGHLIGHT = GREEN;

Game::Game() : 
    whiteTeam(true),
    blackTeam(false),
    selectedPiece(nullptr),
    isWhiteTurn(true),
    boardRotated(false)
{
    // Initialize OpenGL context and wait for it to be ready
    while (!IsWindowReady()) { }

    // Initialize texture manager and ensure it's ready
    auto texManager = TextureManager::GetInstance();
    texManager->Initialize();

    // Load piece textures
    const std::string textureBasePath = "assets/";
    const std::vector<std::string> pieceTypes = {"pawn", "rook", "knight", "bishop", "queen", "king"};
    const std::vector<std::string> colors = {"white", "black"};

    for (const auto& color : colors) {
        for (const auto& piece : pieceTypes) {
            std::string key = color + "_" + piece;
            std::string path = textureBasePath + key + ".png";
            // std::cout << path << std::endl;
            texManager->LoadTextureFromFile(key, path);
        }
    }
}

Game::~Game() {
    // Cleanup textures first
    TextureManager::GetInstance()->UnloadAllTextures();
    TextureManager::Cleanup();
    
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose()) {
        HandleInput();
        
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            DrawBoard();
            Draw();
        }
        EndDrawing();
    }
}

void Game::Draw() {
    // Draw pieces
    if (selectedPiece) {
        // Draw selected piece and valid moves
        for (const auto& move : validMoves) {
            DrawCircle(
                (move.x * TILE_SIZE) + TILE_SIZE/2,
                (move.y * TILE_SIZE) + TILE_SIZE/2,
                10,
                MOVE_HIGHLIGHT
            );
        }
    }
}

void Game::HandleInput() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        Vector2 boardPos = ScreenToBoard(mousePos);
        
        if (boardPos.x >= 0 && boardPos.x < BOARD_SIZE && 
            boardPos.y >= 0 && boardPos.y < BOARD_SIZE) {
            
            if (selectedPiece == nullptr) {
                SelectPiece(boardPos.x, boardPos.y);
            } else {
                MovePiece(boardPos.x, boardPos.y);
            }
        }
    }
}

void Game::DrawBoard() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            Color squareColor = ((x + y) % 2 == 0) ? LIGHT_SQUARE : DARK_SQUARE;
            DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, squareColor);
        }
    }
    
    // Draw pieces
    for (const auto& piece : whiteTeam.GetPieces()) {
        Vector2 pos = GetCenteredPiecePosition(*piece);
        Texture2D tex = piece->GetTexture();
        if (tex.id > 0) {  // Only draw if texture is valid
            DrawTexture(tex, pos.x, pos.y, WHITE);
        } else {
            // Draw a placeholder if texture is missing
            DrawRectangle(pos.x, pos.y, TILE_SIZE/2, TILE_SIZE/2, WHITE);
        }
    }
    
    for (const auto& piece : blackTeam.GetPieces()) {
        Vector2 pos = GetCenteredPiecePosition(*piece);
        Texture2D tex = piece->GetTexture();
        if (tex.id > 0) {  // Only draw if texture is valid
            DrawTexture(tex, pos.x, pos.y, WHITE);
        } else {
            // Draw a placeholder if texture is missing
            DrawRectangle(pos.x, pos.y, TILE_SIZE/2, TILE_SIZE/2, BLACK);
        }
    }
}

Vector2 Game::ScreenToBoard(Vector2 screenPos) {
    return (Vector2){
        floorf(screenPos.x / TILE_SIZE),
        floorf(screenPos.y / TILE_SIZE)
    };
}

Vector2 Game::BoardToScreen(int x, int y) {
    return (Vector2){
        static_cast<float>(x * TILE_SIZE),
        static_cast<float>(y * TILE_SIZE)
    };
}

Vector2 Game::GetCenteredPiecePosition(const Piece& piece) {
    Vector2 boardPos = BoardToScreen(piece.GetX(), piece.GetY());
    return (Vector2){
        boardPos.x + (TILE_SIZE - piece.GetTexture().width) / 2,
        boardPos.y + (TILE_SIZE - piece.GetTexture().height) / 2
    };
}

const Team& Game::GetWhiteTeam() const {
    return whiteTeam;
}

const Team& Game::GetBlackTeam() const {
    return blackTeam;
}

void Game::SelectPiece(int x, int y) {
    // Clear previous selection
    selectedPiece = nullptr;
    validMoves.clear();

    // Get piece at the clicked position
    const Piece* piece = GetPieceAt(x, y);
    if (piece && piece->IsWhite() == isWhiteTurn) {
        selectedPiece = const_cast<Piece*>(piece);
        validMoves = GetValidMoves(selectedPiece);
    }
}

void Game::MovePiece(int x, int y) {
    if (!selectedPiece) return;

    // Check if the move is valid
    Vector2 targetPos = {(float)x, (float)y};
    bool isValidMove = false;
    for (const auto& move : validMoves) {
        if (move.x == targetPos.x && move.y == targetPos.y) {
            isValidMove = true;
            break;
        }
    }

    if (isValidMove) {
        selectedPiece->SetPosition(x, y);
        isWhiteTurn = !isWhiteTurn;  // Switch turns
    }

    // Clear selection
    selectedPiece = nullptr;
    validMoves.clear();
}

std::vector<Vector2> Game::GetValidMoves(Piece* piece) {
    if (!piece) return {};
    return piece->GetValidMoves(*this);
}

const Piece* Game::GetPieceAt(int x, int y) const {
    // Check white team pieces
    for (const auto& piece : whiteTeam.GetPieces()) {
        if (piece->GetX() == x && piece->GetY() == y) {
            return piece.get();
        }
    }
    
    // Check black team pieces
    for (const auto& piece : blackTeam.GetPieces()) {
        if (piece->GetX() == x && piece->GetY() == y) {
            return piece.get();
        }
    }
    
    return nullptr;
} 