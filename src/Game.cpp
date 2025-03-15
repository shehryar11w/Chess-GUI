#include "Game.h"
#include "Piece.h"
#include "Team.h"
#include "TextureManager.h"
#include <cmath>  // For floorf
#include <iostream>
#include "raylib.h"
using namespace std;

// Add static member definitions
const Color Game::LIGHT_SQUARE = RAYWHITE;
const Color Game::DARK_SQUARE = DARKGRAY;
const Color Game::MOVE_HIGHLIGHT = GREEN;

Sound moveSound;
Sound captureSound;

enum GameState {
    MENU,
    PLAY
};

GameState currentState = MENU;

float currentRotation = 0.0f;
float targetRotation = 0.0f;
const float rotationSpeed = 2.0f;  // Adjust this value to control speed

// Use the Move struct from Move.h
Move lastMove;

Game::Game() : 
    whiteTeam(true),
    blackTeam(false),
    selectedPiece(nullptr),
    isWhiteTurn(true),
    boardRotated(false)
{
    

    // Initialize OpenGL context and wait for it to be ready
    while (!IsWindowReady()) { }

    // Initialize audio device
    InitAudioDevice();

    // Load move sound
    moveSound = LoadSound("assets/move.mp3");

    // Load capture sound
    captureSound = LoadSound("assets/capture.mp3");

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
    // Unload move sound
    UnloadSound(moveSound);

    // Unload capture sound
    UnloadSound(captureSound);

    // Close audio device
    CloseAudioDevice();

    // Cleanup textures first
    TextureManager::GetInstance()->UnloadAllTextures();
    TextureManager::Cleanup();
    
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose()) {
        HandleInput();

        // Smooth rotation logic
        if (currentRotation != targetRotation) {
            if (currentRotation < targetRotation) {
                currentRotation += rotationSpeed;
                if (currentRotation > targetRotation) currentRotation = targetRotation;
            } else {
                currentRotation -= rotationSpeed;
                if (currentRotation < targetRotation) currentRotation = targetRotation;
            }
        }

        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            
            switch (currentState) {
                case MENU:
                    DrawMenu();
                    break;
                case PLAY:
                    DrawBoard();
                    DrawLabels();  // Draw labels separately
                    Draw();
                    break;
            }
        }
        EndDrawing();
    }
}

void Game::Draw() {
    // Draw capture highlights first
    if (selectedPiece) {
        for (const auto& move : validMoves) {
            int drawX = boardRotated ? BOARD_SIZE - 1 - move.x : move.x;
            int drawY = boardRotated ? BOARD_SIZE - 1 - move.y : move.y;
            if (GetPieceAt(move.x, move.y) && GetPieceAt(move.x, move.y)->GetType() != PieceType::KING) {
                // Highlight capture moves with a rectangle
                DrawRectangle(
                    drawX * TILE_SIZE,
                    drawY * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE,
                    RED
                );
            } else if (selectedPiece->GetType() == PieceType::PAWN &&
                       abs(lastMove.end.y - lastMove.start.y) == 2 &&
                       lastMove.piece->GetType() == PieceType::PAWN &&
                       lastMove.end.x == move.x &&
                       abs(lastMove.end.y - move.y) == 1) {
                // Highlight en passant moves with a red rectangle
                DrawRectangle(
                    drawX * TILE_SIZE,
                    drawY * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE,
                    RED
                );
            }
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

    // Draw available move highlights
    if (selectedPiece) {
        for (const auto& move : validMoves) {
            int drawX = boardRotated ? BOARD_SIZE - 1 - move.x : move.x;
            int drawY = boardRotated ? BOARD_SIZE - 1 - move.y : move.y;
            if (!GetPieceAt(move.x, move.y)) {
                // Highlight available moves with a circle
                DrawCircle(
                    (drawX * TILE_SIZE) + TILE_SIZE/2,
                    (drawY * TILE_SIZE) + TILE_SIZE/2,
                    10,
                    MOVE_HIGHLIGHT
                );
            }
        }
    }
}

void Game::HandleInput() {
    if (currentState == MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            currentState = PLAY;
        }

        // Check for Play button click
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            int buttonWidth = 200;
            int buttonHeight = 50;
            int buttonX = (GetScreenWidth() - buttonWidth) / 2;
            int buttonY = GetScreenHeight() / 2 + 20;  // Ensure this matches the DrawMenu position
            if (mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth &&
                mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight) {
                currentState = PLAY;
            }
        }
    }

    if (currentState == PLAY) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 boardPos = ScreenToBoard(mousePos);

            if (boardPos.x >= 0 && boardPos.x < BOARD_SIZE &&
                boardPos.y >= 0 && boardPos.y < BOARD_SIZE) {

                const Piece* clickedPiece = GetPieceAt(boardPos.x, boardPos.y);
                if (clickedPiece && clickedPiece->IsWhite() == isWhiteTurn) {
                    SelectPiece(boardPos.x, boardPos.y);
                } else if (selectedPiece) {
                    MovePiece(boardPos.x, boardPos.y);
                }
            }
        }
    }
}

void Game::DrawBoard() {
    int offsetRightX = 20;  // Add scaling from the right
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int drawX = boardRotated ? BOARD_SIZE - 1 - x : x;
            int drawY = boardRotated ? BOARD_SIZE - 1 - y : y;
            Color squareColor = ((x + y) % 2 == 0) ? LIGHT_SQUARE : DARK_SQUARE;
            DrawRectangle(drawX * TILE_SIZE, drawY * TILE_SIZE, TILE_SIZE, TILE_SIZE, squareColor);

            // Draw horizontal labels (A to H)
            if (y == BOARD_SIZE - 1) {
                char colLabel = boardRotated ? ('h' - x) : ('a' + x);
                char label[2] = {colLabel, '\0'};
                int textWidth = MeasureText(label, 20);
                int labelY = BOARD_SIZE * TILE_SIZE + 5;
                DrawText(label, drawX * TILE_SIZE + (TILE_SIZE - textWidth) / 2, labelY, 20, BLACK);
            }

            // Draw vertical labels (1 to 8)
            if (x == 0) {
                char rowLabel = (isWhiteTurn && !boardRotated) ? ('1' + y) : ('8' - y);
                char label[2] = {rowLabel, '\0'};
                int labelX = 10;  // Adjust position to ensure visibility
                DrawText(label, labelX, drawY * TILE_SIZE + TILE_SIZE / 2 - 10, 25, BLACK);
            }
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

void Game::DrawLabels() {
    int offsetRightX = 20;  // Add scaling from the right
    for (int y = 0; y < BOARD_SIZE; y++) {
        // Draw vertical labels (1 to 8) on the left side
        char rowLabel = boardRotated ? ('8' - y) : ('1' + y);
        char label[2] = {rowLabel, '\0'};
        int labelX = 10;  // Adjust position to ensure visibility
        DrawText(label, labelX, y * TILE_SIZE + TILE_SIZE / 2 - 10, 25, BLACK);
    }

    for (int x = 0; x < BOARD_SIZE; x++) {
        // Draw horizontal labels (A to H) at the bottom
        char colLabel = 'a' + x;
        char label[2] = {colLabel, '\0'};
        int textWidth = MeasureText(label, 20);
        int labelY = BOARD_SIZE * TILE_SIZE + 5;
        DrawText(label, x * TILE_SIZE + (TILE_SIZE - textWidth) / 2, labelY, 20, BLACK);
    }
}

void Game::DrawMenu() {
    // Draw background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGRAY);

    const char* title = "Chess Game";
    const char* developers = "Developed by: Shehryar, Sufyan & Faizan";
    int titleWidth = MeasureText(title, 40);
    int devWidth = MeasureText(developers, 20);
    DrawText(title, (GetScreenWidth() - titleWidth) / 2, GetScreenHeight() / 2 - 100, 40, RAYWHITE);

    // Draw Play button with hover effect
    int buttonWidth = 200;
    int buttonHeight = 50;
    int buttonX = (GetScreenWidth() - buttonWidth) / 2;
    int buttonY = GetScreenHeight() / 2 + 20;
    Color buttonColor = LIGHTGRAY;
    Vector2 mousePos = GetMousePosition();
    if (mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth &&
        mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight) {
        buttonColor = GRAY;  // Change color on hover
    }
    DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
    DrawRectangleLines(buttonX, buttonY, buttonWidth, buttonHeight, BLACK);  // Add border
    DrawText("Play", buttonX + (buttonWidth - MeasureText("Play", 20)) / 2, buttonY + 15, 20, BLACK);

    DrawText(developers, (GetScreenWidth() - devWidth) / 2, GetScreenHeight() / 2 + 100, 20, LIGHTGRAY);
}

Vector2 Game::ScreenToBoard(Vector2 screenPos) {
    Vector2 boardPos = (Vector2){
        floorf(screenPos.x / TILE_SIZE),
        floorf(screenPos.y / TILE_SIZE)
    };
    if (boardRotated) {
        boardPos.x = BOARD_SIZE - 1 - boardPos.x;
        boardPos.y = BOARD_SIZE - 1 - boardPos.y;
    }
    return boardPos;
}

Vector2 Game::BoardToScreen(int x, int y) {
    return (Vector2){
        static_cast<float>(x * TILE_SIZE),
        static_cast<float>(y * TILE_SIZE)
    };
}

Vector2 Game::GetCenteredPiecePosition(const Piece& piece) {
    Vector2 boardPos = BoardToScreen(
        boardRotated ? BOARD_SIZE - 1 - piece.GetX() : piece.GetX(),
        boardRotated ? BOARD_SIZE - 1 - piece.GetY() : piece.GetY()
    );
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
        // Check for en passant
        if (selectedPiece->GetType() == PieceType::PAWN &&
            abs(lastMove.end.y - lastMove.start.y) == 2 &&
            lastMove.piece->GetType() == PieceType::PAWN &&
            lastMove.end.x == x &&
            abs(lastMove.end.y - y) == 1) {

            // En passant capture
            Piece* capturedPawn = const_cast<Piece*>(GetPieceAt(x, lastMove.end.y));
            if (capturedPawn) {
                // std::cout << "Captured Pawn at: (" << x << ", " << lastMove.end.y << ")" << std::endl;
                if (capturedPawn->IsWhite()) {
                    whiteTeam.RemovePieceAt(x, lastMove.end.y);
                } else {
                    blackTeam.RemovePieceAt(x, lastMove.end.y);
                }
            }

            // Play capture sound for en passant
            if (captureSound.stream.buffer != NULL) {
                PlaySound(captureSound);
            }
        }

        // Remove the captured piece if present
        Piece* targetPiece = const_cast<Piece*>(GetPieceAt(x, y));
        if (targetPiece && targetPiece->IsWhite() != selectedPiece->IsWhite()) {
            if (targetPiece->GetType() == PieceType::KING) {
                // Cannot capture the king, invalidate the move
                return;
            }
            if (targetPiece->IsWhite()) {
                whiteTeam.RemovePieceAt(x, y);
            } else {
                blackTeam.RemovePieceAt(x, y);
            }
            // Play capture sound
            if (captureSound.stream.buffer != NULL) {
                PlaySound(captureSound);
            }
        } else {
            // Play move sound
            if (moveSound.stream.buffer != NULL) {
                PlaySound(moveSound);
            }
        }

        // Update last move
        lastMove = {selectedPiece->GetPosition(), targetPos, selectedPiece};

        selectedPiece->SetPosition(x, y);
        isWhiteTurn = !isWhiteTurn;  // Switch turns
        ToggleBoardRotation();  // Rotate the board after a successful move
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

void Game::ToggleBoardRotation() {
    boardRotated = !boardRotated;
    targetRotation = boardRotated ? 180.0f : 0.0f;
} 
