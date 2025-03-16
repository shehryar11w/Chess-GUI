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

float currentRotation = 0.0f;
float targetRotation = 0.0f;
const float rotationSpeed = 2.0f;  // Adjust this value to control speed

// Use the Move struct from Move.h
Move lastMove;

Vector2 promotionSquare = {-1, -1};

Game::Game() : 
    whiteTeam(true),
    blackTeam(false),
    selectedPiece(nullptr),
    isWhiteTurn(true),
    boardRotated(false),
    currentState(MENU),  // Start in MENU state
    promotionSquare({-1, -1})
{
    // Set window to be maximized by default
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);

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
            
            switch (GetGameState()) {
                case MENU:
                    DrawMenu();
                    break;
                case PLAY:
                    DrawBoard();
                    DrawLabels();
                    Draw();
                    break;
                case PROMOTION:
                    DrawBoard();
                    DrawLabels();
                    Draw();
                    DrawPromotionUI();
                    break;
            }
        }
        EndDrawing();
    }
}

void Game::Draw() {
    // Draw capture highlights first
    if (selectedPiece && GetGameState() != PROMOTION) {
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
                       lastMove.end.y == selectedPiece->GetY() &&  // Check if enemy pawn is on same rank
                       abs(move.x - selectedPiece->GetX()) == 1 && // Check if moving diagonally
                       move.y == lastMove.end.y + (selectedPiece->IsWhite() ? -1 : 1)) { // Check if moving to correct square
                // Highlight en passant moves with a blue rectangle
                DrawRectangle(
                    drawX * TILE_SIZE,
                    drawY * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE,
                    BLUE
                );
            }
        }
    }

    // Find kings and check if they're in check
    const Piece* whiteKing = nullptr;
    const Piece* blackKing = nullptr;
    for (const auto& piece : whiteTeam.GetPieces()) {
        if (piece->GetType() == PieceType::KING) {
            whiteKing = piece.get();
            break;
        }
    }
    for (const auto& piece : blackTeam.GetPieces()) {
        if (piece->GetType() == PieceType::KING) {
            blackKing = piece.get();
            break;
        }
    }

    // Draw check highlight
    if (whiteKing && IsSquareUnderAttack(whiteKing->GetX(), whiteKing->GetY(), false)) {
        int drawX = boardRotated ? BOARD_SIZE - 1 - whiteKing->GetX() : whiteKing->GetX();
        int drawY = boardRotated ? BOARD_SIZE - 1 - whiteKing->GetY() : whiteKing->GetY();
        DrawRectangle(
            drawX * TILE_SIZE,
            drawY * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE,
            Color{255, 0, 0, 100} // Semi-transparent red
        );
    }
    if (blackKing && IsSquareUnderAttack(blackKing->GetX(), blackKing->GetY(), true)) {
        int drawX = boardRotated ? BOARD_SIZE - 1 - blackKing->GetX() : blackKing->GetX();
        int drawY = boardRotated ? BOARD_SIZE - 1 - blackKing->GetY() : blackKing->GetY();
        DrawRectangle(
            drawX * TILE_SIZE,
            drawY * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE,
            Color{255, 0, 0, 100} // Semi-transparent red
        );
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
    if (selectedPiece && GetGameState() != PROMOTION) {
        for (const auto& move : validMoves) {
            int drawX = boardRotated ? BOARD_SIZE - 1 - move.x : move.x;
            int drawY = boardRotated ? BOARD_SIZE - 1 - move.y : move.y;
            if (!GetPieceAt(move.x, move.y) && 
                !(selectedPiece->GetType() == PieceType::PAWN &&
                  abs(lastMove.end.y - lastMove.start.y) == 2 &&
                  lastMove.piece->GetType() == PieceType::PAWN &&
                  lastMove.end.x == move.x &&
                  lastMove.end.y == selectedPiece->GetY() &&
                  abs(move.x - selectedPiece->GetX()) == 1 &&
                  move.y == lastMove.end.y + (selectedPiece->IsWhite() ? -1 : 1))) {
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
    if (GetGameState() == MENU) {
        // Check for Play button click
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            int playWidth = MeasureText("Play", 30);
            int buttonWidth = playWidth + 60;
            int buttonHeight = 40;
            int buttonX = (GetScreenWidth() - buttonWidth) / 2;
            int buttonY = GetScreenHeight() / 2;

            if (mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth &&
                mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight) {
                SetGameState(PLAY);
            }
        }
        return;
    }

    if (GetGameState() == PROMOTION) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            float spacing = 100.0f;
            float startX = GetScreenWidth() / 2 - (spacing * 4 / 2);
            float y = GetScreenHeight() / 2 - 50;

            // Check which piece was clicked
            for (int i = 0; i < 4; i++) {
                Rectangle pieceRect = {
                    startX + i * spacing,
                    y,
                    64,  // Assuming piece texture size
                    64   // Assuming piece texture size
                };

                if (CheckCollisionPointRec(mousePos, pieceRect)) {
                    PieceType promotionType;
                    switch (i) {
                        case 0: promotionType = PieceType::QUEEN; break;
                        case 1: promotionType = PieceType::ROOK; break;
                        case 2: promotionType = PieceType::BISHOP; break;
                        case 3: promotionType = PieceType::KNIGHT; break;
                        default: return;
                    }
                    PromotePawn(promotionType);
                    SetGameState(PLAY);
                    return;
                }
            }
        }
        return;
    }

    if (GetGameState() == PLAY) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 boardPos = ScreenToBoard(mousePos);
            
            if (boardPos.x >= 0 && boardPos.x < BOARD_SIZE &&
                boardPos.y >= 0 && boardPos.y < BOARD_SIZE) {
                if (selectedPiece) {
                    MovePiece(boardPos.x, boardPos.y);
                } else {
                    SelectPiece(boardPos.x, boardPos.y);
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
    const char* playText = "Play";

    int titleWidth = MeasureText(title, 40);
    int devWidth = MeasureText(developers, 20);
    int playWidth = MeasureText(playText, 30);

    // Draw title
    DrawText(title, (GetScreenWidth() - titleWidth) / 2, GetScreenHeight() / 3, 40, RAYWHITE);

    // Draw Play button
    int buttonWidth = playWidth + 60;  // Add padding
    int buttonHeight = 40;
    int buttonX = (GetScreenWidth() - buttonWidth) / 2;
    int buttonY = GetScreenHeight() / 2;
    
    // Draw button background with hover effect
    Vector2 mousePos = GetMousePosition();
    Color buttonColor = RAYWHITE;
    if (mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth &&
        mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight) {
        buttonColor = LIGHTGRAY;  // Change color on hover
    }
    DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
    
    // Draw button text
    DrawText(playText, 
        buttonX + (buttonWidth - playWidth) / 2,
        buttonY + (buttonHeight - 30) / 2,  // 30 is font size
        30, 
        BLACK
    );

    // Draw developers credit
    DrawText(developers, 
        (GetScreenWidth() - devWidth) / 2,
        GetScreenHeight() - 50,
        20,
        LIGHTGRAY
    );
}

void Game::DrawPromotionUI() {
    // Draw semi-transparent background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});

    // Get texture manager instance
    auto texManager = TextureManager::GetInstance();
    bool isWhite = isWhiteTurn;  // The pawn being promoted belongs to the player who just moved

    // Draw promotion pieces
    const std::vector<std::string> pieceTypes = {"queen", "rook", "bishop", "knight"};
    float spacing = 100.0f;
    float startX = GetScreenWidth() / 2 - (spacing * pieceTypes.size() / 2);
    float y = GetScreenHeight() / 2 - 50;

    for (size_t i = 0; i < pieceTypes.size(); i++) {
        Vector2 piecePos = {startX + i * spacing, y};
        Texture2D texture = texManager->GetTexture(
            isWhite ? "white_" + pieceTypes[i] : "black_" + pieceTypes[i]
        );
        
        DrawTexture(texture, piecePos.x, piecePos.y, WHITE);
    }
}

void Game::PromotePawn(PieceType type) {
    // Create new piece at promotion square
    Team& team = isWhiteTurn ? whiteTeam : blackTeam;
    team.RemovePieceAt(promotionSquare.x, promotionSquare.y);
    team.AddPiece(type, promotionSquare.x, promotionSquare.y);

    // Play move sound
    if (moveSound.stream.buffer != NULL) {
        PlaySound(moveSound);
    }

    // Switch turns and rotate board
    isWhiteTurn = !isWhiteTurn;
    ToggleBoardRotation();
    
    // Reset promotion square
    promotionSquare = {-1, -1};
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
            lastMove.end.y == selectedPiece->GetY() &&  // Check if enemy pawn is on same rank
            abs(x - selectedPiece->GetX()) == 1 &&      // Check if moving diagonally
            y == lastMove.end.y + (selectedPiece->IsWhite() ? -1 : 1)) {  // Check if moving to correct square behind enemy pawn

            // En passant capture
            Piece* capturedPawn = const_cast<Piece*>(GetPieceAt(x, lastMove.end.y));
            if (capturedPawn) {
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

        // Check for pawn promotion
        if (selectedPiece->GetType() == PieceType::PAWN) {
            int promotionRank = selectedPiece->IsWhite() ? 0 : 7;
            if (y == promotionRank) {
                promotionSquare = {(float)x, (float)y};
                SetGameState(PROMOTION);
                return;
            }
        }

        isWhiteTurn = !isWhiteTurn;  // Switch turns
        ToggleBoardRotation();  // Rotate the board after a successful move
    }

    // Clear selection
    selectedPiece = nullptr;
    validMoves.clear();
}

bool Game::IsSquareUnderAttack(int x, int y, bool byWhite, Vector2 ignorePiecePos) const {
    const Team& attackingTeam = byWhite ? whiteTeam : blackTeam;
    
    // Check all pieces of the attacking team
    for (const auto& piece : attackingTeam.GetPieces()) {
        // Skip if this is the piece position we're ignoring (for capture validation)
        if (piece->GetX() == ignorePiecePos.x && piece->GetY() == ignorePiecePos.y) {
            continue;
        }
        
        // Skip if the piece is at the target square (for king move validation)
        if (piece->GetX() == x && piece->GetY() == y) {
            continue;
        }
        
        // Get all possible moves for this piece
        auto moves = piece->GetValidMoves(*this);
        
        // Check if any move can reach the target square
        for (const auto& move : moves) {
            if (move.x == x && move.y == y) {
                return true;
            }
        }
    }
    return false;
}

std::vector<Vector2> Game::GetValidMoves(Piece* piece) {
    if (!piece) return {};
    
    std::vector<Vector2> moves = piece->GetValidMoves(*this);
    std::vector<Vector2> legalMoves;
    
    // Find our king
    const Team& ourTeam = piece->IsWhite() ? whiteTeam : blackTeam;
    const Piece* ourKing = nullptr;
    for (const auto& p : ourTeam.GetPieces()) {
        if (p->GetType() == PieceType::KING) {
            ourKing = p.get();
            break;
        }
    }
    
    if (!ourKing) return moves; // Should never happen in a valid game
    
    // Store original position
    Vector2 originalPos = piece->GetPosition();
    
    // Test each move to see if it leaves our king in check
    for (const auto& move : moves) {
        // Store piece at target position (if any)
        const Piece* capturedPiece = GetPieceAt(move.x, move.y);
        Vector2 capturedPos = {-1, -1};
        
        if (capturedPiece) {
            capturedPos = {(float)capturedPiece->GetX(), (float)capturedPiece->GetY()};
        }
        
        // Make the move temporarily
        piece->SetPosition(move.x, move.y);
        
        // Check if our king is in check after the move
        bool kingInCheck;
        if (piece->GetType() == PieceType::KING) {
            // If moving the king, check the new position
            kingInCheck = IsSquareUnderAttack(move.x, move.y, !piece->IsWhite(), capturedPos);
        } else {
            // For other pieces, check the king's current position
            kingInCheck = IsSquareUnderAttack(ourKing->GetX(), ourKing->GetY(), !piece->IsWhite(), capturedPos);
        }
        
        // If the move doesn't leave/put our king in check, it's legal
        if (!kingInCheck) {
            legalMoves.push_back(move);
        }
    }
    
    // Restore the original position
    piece->SetPosition(originalPos.x, originalPos.y);
    
    return legalMoves;
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
