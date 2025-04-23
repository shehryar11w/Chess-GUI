#include "Game.h"
#include "Piece.h"
#include "Team.h"
#include "TextureManager.h"
#include <cmath>  
#include <iostream>
#include <cstring>  // Add for strlen
#include "raylib.h"
using namespace std;

// Add static member definitions
const Color Game::LIGHT_SQUARE = RAYWHITE;
const Color Game::DARK_SQUARE = DARKGRAY;
const Color Game::MOVE_HIGHLIGHT = GREEN;

Sound moveSound;
Sound captureSound;
Sound checkSound;
Sound promotionSound;
Sound gameStartSound;
Sound gameOverSound;
Sound checkmateSound;
Sound stalemateSound;
Texture2D backgroundTexture;
Texture2D menuBackgroundTexture;
Texture2D profileTexture;  // Add profile texture

// Add player name variables
char whitePlayerName[32] = "";
char blackPlayerName[32] = "";
bool whiteNameActive = false;
bool blackNameActive = false;

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
    namesRotated(false),  // Initialize namesRotated to false
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

    // Load check sound
    checkSound = LoadSound("assets/check.mp3");

    // Load promotion sound
    promotionSound = LoadSound("assets/promote.mp3");

    // Load game start sound
    gameStartSound = LoadSound("assets/game_start.mp3");

    // Load game over sound
    gameOverSound = LoadSound("assets/game-end.mp3");

    // Load checkmate sound
    checkmateSound = LoadSound("assets/checkmate.mp3");

    // Load stalemate sound
    stalemateSound = LoadSound("assets/stalemate.mp3");

    // Load background texture
    backgroundTexture = LoadTexture("assets/background.jpg");
    
    // Load menu background texture
    menuBackgroundTexture = LoadTexture("assets/Mainmenu.png");

    // Load profile texture
    profileTexture = LoadTexture("assets/Profile-Male-Transparent.png");

    // Initialize texture manager and ensure it's ready
    auto texManager = TextureManager::GetInstance();
    texManager->Initialize();

    // Load piece textures
    const  string textureBasePath = "assets/";
    const  vector< string> pieceTypes = {"pawn", "rook", "knight", "bishop", "queen", "king"};
    const  vector< string> colors = {"white", "black"};

    for (const auto& color : colors) {
        for (const auto& piece : pieceTypes) {
             string key = color + "_" + piece;
             string path = textureBasePath + key + ".png";
            texManager->LoadTextureFromFile(key, path);
        }
    }
}

Game::~Game() {
    // Unload move sound
    UnloadSound(moveSound);

    // Unload capture sound
    UnloadSound(captureSound);

    // Unload check sound
    UnloadSound(checkSound);

    // Unload promotion sound
    UnloadSound(promotionSound);

    // Unload game start sound
    UnloadSound(gameStartSound);

    // Unload game over sound
    UnloadSound(gameOverSound);

    // Unload checkmate sound
    UnloadSound(checkmateSound);

    // Unload stalemate sound
    UnloadSound(stalemateSound);

    // Unload background texture
    UnloadTexture(backgroundTexture);

    // Unload menu background texture
    UnloadTexture(menuBackgroundTexture);

    // Unload profile texture
    UnloadTexture(profileTexture);

    // Close audio device
    CloseAudioDevice();

    // Cleanup textures first
    TextureManager::GetInstance()->UnloadAllTextures();
    TextureManager::Cleanup();
    
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose() && !shouldClose) {
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
                case GAME_OVER:
                    DrawBoard();
                    DrawLabels();
                    Draw();
                    DrawGameOverUI();
                    break;
            }
        }
        EndDrawing();
    }
}

void Game::Draw() {
    // Get window dimensions for centering
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    // Calculate board offset
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    // Draw the board first
    DrawBoard();
    
    // Draw labels
    DrawLabels();

    // Only draw move highlights if we're in PLAY state
    if (GetGameState() == PLAY) {
        // Draw capture highlights first
        if (selectedPiece) {
            for (const auto& move : validMoves) {
                int drawX = boardRotated ? BOARD_SIZE - 1 - move.x : move.x;
                int drawY = boardRotated ? BOARD_SIZE - 1 - move.y : move.y;
                if (GetPieceAt(move.x, move.y) && GetPieceAt(move.x, move.y)->GetType() != PieceType::KING) {
                    // Highlight capture moves with a rectangle
                    DrawRectangle(
                        offsetX + drawX * TILE_SIZE,
                        offsetY + drawY * TILE_SIZE,
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
                        offsetX + drawX * TILE_SIZE,
                        offsetY + drawY * TILE_SIZE,
                        TILE_SIZE,
                        TILE_SIZE,
                        BLUE
                    );
                }
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
            offsetX + drawX * TILE_SIZE,
            offsetY + drawY * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE,
            Color{255, 0, 0, 100} // Semi-transparent red
        );
    }
    if (blackKing && IsSquareUnderAttack(blackKing->GetX(), blackKing->GetY(), true)) {
        int drawX = boardRotated ? BOARD_SIZE - 1 - blackKing->GetX() : blackKing->GetX();
        int drawY = boardRotated ? BOARD_SIZE - 1 - blackKing->GetY() : blackKing->GetY();
        DrawRectangle(
            offsetX + drawX * TILE_SIZE,
            offsetY + drawY * TILE_SIZE,
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

    // Draw available move highlights only in PLAY state
    if (GetGameState() == PLAY && selectedPiece) {
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
                    offsetX + (drawX * TILE_SIZE) + TILE_SIZE/2,
                    offsetY + (drawY * TILE_SIZE) + TILE_SIZE/2,
                    10,
                    MOVE_HIGHLIGHT
                );
            }
        }
    }

    // Draw promotion UI if in promotion state
    if (GetGameState() == PROMOTION) {
        DrawPromotionUI();
    }
}
void Game::HandleInput() {
    if (GetGameState() == MENU) {
        Vector2 mousePos = GetMousePosition();

        // Recalculate input field parameters to match DrawMenu
        int inputWidth = 400;  // Match DrawMenu's inputWidth
        int inputHeight = 50;  // Match DrawMenu's inputHeight
        int inputX = (GetScreenWidth() - inputWidth) / 2;
        int inputY = GetScreenHeight() / 3 + 50;  // Match DrawMenu's inputY

        // Define rectangles for input fields using the same positions as DrawMenu
        Rectangle whiteInputRect = {
            (float)inputX,
            (float)(inputY - 12),  // Y position matches DrawMenu's white input field
            (float)inputWidth,
            (float)inputHeight
        };
        Rectangle blackInputRect = {
            (float)inputX,
            (float)(inputY + 140 - 12),  // Y position matches DrawMenu's black input field
            (float)inputWidth,
            (float)inputHeight
        };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // Check clicks on input fields
            if (CheckCollisionPointRec(mousePos, whiteInputRect)) {
                whiteNameActive = true;
                blackNameActive = false;
            } else if (CheckCollisionPointRec(mousePos, blackInputRect)) {
                whiteNameActive = false;
                blackNameActive = true;
            } else {
                whiteNameActive = false;
                blackNameActive = false;
            }

            // Recalculate Play button position to match DrawMenu
            const char* playText = "Play";
            int playWidth = MeasureText(playText, 40);
            int buttonWidth = playWidth + 100;  // Match DrawMenu's button padding
            int buttonHeight = 60;
            int buttonX = (GetScreenWidth() - buttonWidth) / 2;
            int buttonY = GetScreenHeight() / 2 + 50 + 40;  // Base Y position from DrawMenu

            // Adjust button's Y position to match drawn position (buttonY + 40)
            Rectangle playButtonRect = {
                (float)buttonX,
                (float)(buttonY + (27)),  // Matches DrawMenu's button Y offset
                (float)buttonWidth,
                (float)buttonHeight
            };

            if (CheckCollisionPointRec(mousePos, playButtonRect)) {
                if (strlen(whitePlayerName) > 0 && strlen(blackPlayerName) > 0) {
                    // Play game start sound
                    if (gameStartSound.stream.buffer != NULL) {
                        PlaySound(gameStartSound);
                    }
                    SetGameState(PLAY);
                }
            }
        }

        // Handle text input
        if (whiteNameActive) {
            int key = GetCharPressed();
            while (key > 0) {
                if (strlen(whitePlayerName) < 31) {  // Leave room for null terminator
                    whitePlayerName[strlen(whitePlayerName)] = (char)key;
                    whitePlayerName[strlen(whitePlayerName) + 1] = '\0';
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && strlen(whitePlayerName) > 0) {
                whitePlayerName[strlen(whitePlayerName) - 1] = '\0';
            }
        }
        else if (blackNameActive) {
            int key = GetCharPressed();
            while (key > 0) {
                if (strlen(blackPlayerName) < 31) {  // Leave room for null terminator
                    blackPlayerName[strlen(blackPlayerName)] = (char)key;
                    blackPlayerName[strlen(blackPlayerName) + 1] = '\0';
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && strlen(blackPlayerName) > 0) {
                blackPlayerName[strlen(blackPlayerName) - 1] = '\0';
            }
        }
        return;
    }

    if (GetGameState() == PROMOTION) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            
            // Calculate board position
            int boardPixelSize = TILE_SIZE * BOARD_SIZE;
            int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
            int offsetY = (GetScreenHeight() - boardPixelSize) / 2;

            float spacing = 100.0f;
            float startX = offsetX + (boardPixelSize - spacing * 4) / 2;
            float y = offsetY + (boardPixelSize - 64) / 2;  // 64 is piece texture size

            // Check which piece was clicked
            for (int i = 0; i < 4; i++) {
                Rectangle pieceRect = {
                    startX + i * spacing,
                    y,
                    64,  // Piece texture size
                    64   // Piece texture size
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
                    return;
                }
            }
        }
        return;
    }

    if (GetGameState() == GAME_OVER) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            
            // Calculate button positions (same as in DrawGameOverUI)
            int centerX = GetScreenWidth() / 2;
            int startY = GetScreenHeight() / 3;
            const int BUTTON_PADDING = 20;
            const int BUTTON_HEIGHT = 50;
            
            const char* exitText = "Exit";
            const char* playAgainText = "Play Again";
            
            int exitWidth = MeasureText(exitText, 30) + BUTTON_PADDING * 2;
            int playAgainWidth = MeasureText(playAgainText, 30) + BUTTON_PADDING * 2;
            
            int totalWidth = exitWidth + playAgainWidth + 50;
            int startX = centerX - totalWidth / 2;
            int buttonY = startY + 8 * 40; // LINE_SPACING * 8

            Rectangle exitButton = {
                (float)startX,
                (float)buttonY,
                (float)exitWidth,
                (float)BUTTON_HEIGHT
            };
            
            Rectangle playAgainButton = {
                (float)(startX + exitWidth + 50),
                (float)buttonY,
                (float)playAgainWidth,
                (float)BUTTON_HEIGHT
            };

            if (CheckCollisionPointRec(mousePos, exitButton)) {
                shouldClose = true;
            } else if (CheckCollisionPointRec(mousePos, playAgainButton)) {
                // Reset game state
                isWhiteTurn = true;
                boardRotated = false;
                namesRotated = false;
                selectedPiece = nullptr;
                validMoves.clear();
                
                // Reset teams
                whiteTeam.Reset();
                blackTeam.Reset();
                
                // Clear captured pieces
                whiteCapturedPieces.clear();
                blackCapturedPieces.clear();
                
                // Clear player names and reset name flags
                memset(whitePlayerName, 0, sizeof(whitePlayerName));
                memset(blackPlayerName, 0, sizeof(blackPlayerName));
                whiteNameActive = false;
                blackNameActive = false;
                
                // Return to menu
                SetGameState(MENU);
            }
        }
        return;
    }

    if (GetGameState() == PLAY) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 boardPos = ScreenToBoard(mousePos);
            
            // Check for resign button click
            int boardPixelSize = TILE_SIZE * BOARD_SIZE;
            int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
            int offsetY = (GetScreenHeight() - boardPixelSize) / 2;
            
            const int RESIGN_BUTTON_WIDTH = 100;
            const int RESIGN_BUTTON_HEIGHT = 40;
            const int RESIGN_BUTTON_MARGIN = 20;
            const int LABEL_MARGIN = 8;
            const int LABEL_SIZE = 20;
            
            Rectangle resignButton = {
                (float)(offsetX + boardPixelSize - RESIGN_BUTTON_WIDTH),  // Align to right end
                (float)(offsetY + boardPixelSize + LABEL_MARGIN + LABEL_SIZE + RESIGN_BUTTON_MARGIN),
                (float)RESIGN_BUTTON_WIDTH,
                (float)RESIGN_BUTTON_HEIGHT
            };
            
            if (CheckCollisionPointRec(mousePos, resignButton)) {
                // Play game over sound for resignation
                if (gameOverSound.stream.buffer != NULL) {
                    PlaySound(gameOverSound);
                }
                SetGameState(GAME_OVER);
                return;
            }
            
            if (boardPos.x >= 0 && boardPos.x < BOARD_SIZE &&
                boardPos.y >= 0 && boardPos.y < BOARD_SIZE) {
                
                // Get the clicked piece
                Piece* clickedPiece = const_cast<Piece*>(GetPieceAt(boardPos.x, boardPos.y));
                
                if (selectedPiece) {
                    // If clicking a piece of the current player's color
                    if (clickedPiece && clickedPiece->IsWhite() == isWhiteTurn) {
                        // Select the new piece
                        selectedPiece = clickedPiece;
                        validMoves = GetValidMoves(selectedPiece);
                    } else {
                        // Try to move to the clicked square
                        bool isValidMove = false;
                        for (const auto& move : validMoves) {
                            if (move.x == boardPos.x && move.y == boardPos.y) {
                                isValidMove = true;
                                break;
                            }
                        }
                        
                        if (isValidMove) {
                            MovePiece(boardPos.x, boardPos.y);
                            
                            // After move, check for checkmate or stalemate
                            if (IsCheckmate(isWhiteTurn)) {
                                namesRotated = !namesRotated;
                                boardRotated = !boardRotated;
                                if (checkmateSound.stream.buffer != NULL) {
                                    PlaySound(checkmateSound);
                                }
                                SetGameState(GAME_OVER);
                            } else if (IsStalemate(isWhiteTurn)) {
                                namesRotated = !namesRotated;
                                boardRotated = !boardRotated;
                                if (stalemateSound.stream.buffer != NULL) {
                                    PlaySound(stalemateSound);
                                }
                                SetGameState(GAME_OVER);
                            }
                        } else {
                            // Deselect if clicking an invalid move
                            selectedPiece = nullptr;
                            validMoves.clear();
                        }
                    }
                } else if (clickedPiece && clickedPiece->IsWhite() == isWhiteTurn) {
                    // Select new piece if it's the current player's turn
                    selectedPiece = clickedPiece;
                    validMoves = GetValidMoves(selectedPiece);
                }
            }
        }
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
                AddCapturedPiece(capturedPawn->GetType(), capturedPawn->IsWhite());
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
            AddCapturedPiece(targetPiece->GetType(), targetPiece->IsWhite());
            if (targetPiece->IsWhite()) {
                whiteTeam.RemovePieceAt(x, y);
            } else {
                blackTeam.RemovePieceAt(x, y);
            }
            // Play capture sound
            if (captureSound.stream.buffer != NULL) {
                PlaySound(captureSound);
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
                // Don't switch turns yet - wait for promotion to complete
                return;
            }
        }

        // If not promoting, then play the move sound if no capture was made.
        // (If a capture occurred, the capture sound was already played.)
        if ((!targetPiece || targetPiece->IsWhite() == selectedPiece->IsWhite()) &&
            moveSound.stream.buffer != NULL) {
            PlaySound(moveSound);
        }

        // Check if this move puts the opponent's king in check
        const Team& opposingTeam = isWhiteTurn ? blackTeam : whiteTeam;
        const Piece* opposingKing = nullptr;
        for (const auto& piece : opposingTeam.GetPieces()) {
            if (piece->GetType() == PieceType::KING) {
                opposingKing = piece.get();
                break;
            }
        }
        
        if (opposingKing && IsSquareUnderAttack(opposingKing->GetX(), opposingKing->GetY(), isWhiteTurn)) {
             if (IsCheckmate(!isWhiteTurn) || IsStalemate(!isWhiteTurn)) {

             }
            // Play check sound
            else if (checkSound.stream.buffer != NULL) {
                PlaySound(checkSound);
            }
        }

        // Only switch turns if not promoting
        if (GetGameState() != PROMOTION) {
            isWhiteTurn = !isWhiteTurn;
            boardRotated = !boardRotated;
            namesRotated = !namesRotated;
        }
    }

    // Clear selection
    selectedPiece = nullptr;
    validMoves.clear();
}

void Game::PromotePawn(PieceType type) {
    // Create new piece at promotion square
    Team& team = isWhiteTurn ? whiteTeam : blackTeam;
    team.RemovePieceAt(promotionSquare.x, promotionSquare.y);
    team.AddPiece(type, promotionSquare.x, promotionSquare.y);

    // Play move sound
    if (promotionSound.stream.buffer != NULL) {
        PlaySound(promotionSound);
        }

    // Clear any selected piece and valid moves
    selectedPiece = nullptr;
    validMoves.clear();

    // Switch turns and rotate board after promotion is complete
    isWhiteTurn = !isWhiteTurn;
    boardRotated = !boardRotated;
    namesRotated = !namesRotated;
    
    // Reset promotion square and state

    // Check for checkmate/stalemate after switching turns
    if (IsCheckmate(isWhiteTurn)) {
        boardRotated = !boardRotated;
        namesRotated = !namesRotated;
        if (checkmateSound.stream.buffer != NULL) {
                    PlaySound(checkmateSound);
        }
        SetGameState(GAME_OVER);
    } else if (IsStalemate(isWhiteTurn)) {
        boardRotated = !boardRotated;
        namesRotated = !namesRotated;
        if (stalemateSound.stream.buffer != NULL) {
                    PlaySound(stalemateSound);
        }
        SetGameState(GAME_OVER);
    } else {
        SetGameState(PLAY);
    }
        promotionSquare = {-1, -1};
}

void Game::DrawPromotionUI() {
    // Draw semi-transparent background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});

    // Get texture manager instance
    auto texManager = TextureManager::GetInstance();
    
    // The pawn being promoted belongs to the current player
    bool isWhitePiece = isWhiteTurn;

    // Calculate board position
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
    int offsetY = (GetScreenHeight() - boardPixelSize) / 2;

    // Draw promotion pieces
    const  vector< string> pieceTypes = {"queen", "rook", "bishop", "knight"};
    float spacing = 100.0f;
    float startX = offsetX + (boardPixelSize - spacing * pieceTypes.size()) / 2;
    float y = offsetY + (boardPixelSize - 64) / 2;  // 64 is piece texture size

    for (size_t i = 0; i < pieceTypes.size(); i++) {
        Vector2 piecePos = {startX + i * spacing, y};
         string prefix = isWhitePiece ? "white_" : "black_";
        Texture2D texture = texManager->GetTexture(prefix + pieceTypes[i]);
        
        DrawTexture(texture, piecePos.x, piecePos.y, WHITE);  // Always use WHITE tint to preserve piece color
    }
}

void Game::DrawLabels() {
    // Get window dimensions for centering
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    // Calculate board offset
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    // Constants for label styling
    const int LABEL_SIZE = 20;
    const int LABEL_MARGIN = 8;
    const Color LABEL_COLOR = RAYWHITE;
    const Color SHADOW_COLOR = BLACK;
    const int SHADOW_OFFSET = 1;
    const int PROFILE_SIZE = 32;
    const int NAME_MARGIN = 12;
    const int PLAYER_NAME_SIZE = 24;
    const int VERTICAL_PADDING = 20;

    // Constants for captured pieces display
    const int CAPTURED_PIECE_SIZE = 30;
    const int CAPTURED_PIECE_SPACING = 15;
    const int CAPTURED_HEADER_SIZE = 25;
    const int CAPTURED_SECTION_MARGIN = 40;
    const int CAPTURED_HEADER_VERTICAL_OFFSET = 50;
    const int CAPTURED_SECTION_WIDTH = 200;  // Width for each capture section
    const int CAPTURED_LINE_SPACING = 40;    // Space between two lines of pieces

    // Draw captured pieces sections
    if (GetGameState() == PLAY) {
        // Get texture manager instance
        auto texManager = TextureManager::GetInstance();

        // Calculate positions based on screen width
        int leftSectionX = (windowWidth - boardPixelSize) / 4 - CAPTURED_SECTION_WIDTH / 2;
        int rightSectionX = windowWidth - (windowWidth - boardPixelSize) / 4 - CAPTURED_SECTION_WIDTH / 2;
        int capturedY = offsetY + boardPixelSize / 2;

        // Helper function to draw captured pieces in two lines
        auto drawCapturedPieces = [&](const  vector<PieceType>& pieces, int startX, int startY, bool isWhite) {
            int currentX = startX;
            int currentY = startY;
            int piecesInFirstLine = 0;
            int maxPiecesInLine = CAPTURED_SECTION_WIDTH / (CAPTURED_PIECE_SIZE + CAPTURED_PIECE_SPACING);

            for (const auto& pieceType : pieces) {
                 string pieceName;
                switch (pieceType) {
                    case PieceType::PAWN: pieceName = isWhite ? "white_pawn" : "black_pawn"; break;
                    case PieceType::ROOK: pieceName = isWhite ? "white_rook" : "black_rook"; break;
                    case PieceType::KNIGHT: pieceName = isWhite ? "white_knight" : "black_knight"; break;
                    case PieceType::BISHOP: pieceName = isWhite ? "white_bishop" : "black_bishop"; break;
                    case PieceType::QUEEN: pieceName = isWhite ? "white_queen" : "black_queen"; break;
                    case PieceType::KING: pieceName = isWhite ? "white_king" : "black_king"; break;
                }
                Texture2D texture = texManager->GetTexture(pieceName);
                DrawTexture(texture, currentX, currentY, WHITE);
                
                piecesInFirstLine++;
                if (piecesInFirstLine >= maxPiecesInLine) {
                    currentX = startX;
                    currentY += CAPTURED_PIECE_SIZE + CAPTURED_LINE_SPACING;
                    piecesInFirstLine = 0;
                } else {
                    currentX += (CAPTURED_PIECE_SIZE + CAPTURED_PIECE_SPACING);
                }
            }
        };

        // Draw headers and pieces based on whose turn it is
        if (this->isWhiteTurn) {
            // White's turn - show black's captures on right, white's on left
            // Draw "Black's Captures" header (right side)
            const char* blackHeader = "Black's Captures";
            int blackHeaderWidth = MeasureText(blackHeader, CAPTURED_HEADER_SIZE);
            DrawText(blackHeader,
                rightSectionX + CAPTURED_SECTION_WIDTH / 2 - blackHeaderWidth / 2,
                capturedY - CAPTURED_HEADER_VERTICAL_OFFSET,
                CAPTURED_HEADER_SIZE, LABEL_COLOR);

            // Draw black's captured pieces (right side)
            drawCapturedPieces(blackCapturedPieces, rightSectionX, capturedY, false);

            // Draw "White's Captures" header (left side)
            const char* whiteHeader = "White's Captures";
            int whiteHeaderWidth = MeasureText(whiteHeader, CAPTURED_HEADER_SIZE);
            DrawText(whiteHeader,
                leftSectionX + CAPTURED_SECTION_WIDTH / 2 - whiteHeaderWidth / 2,
                capturedY - CAPTURED_HEADER_VERTICAL_OFFSET,
                CAPTURED_HEADER_SIZE, LABEL_COLOR);

            // Draw white's captured pieces (left side)
            drawCapturedPieces(whiteCapturedPieces, leftSectionX, capturedY, true);
        } else {
            // Black's turn - show white's captures on right, black's on left
            // Draw "White's Captures" header (right side)
            const char* whiteHeader = "White's Captures";
            int whiteHeaderWidth = MeasureText(whiteHeader, CAPTURED_HEADER_SIZE);
            DrawText(whiteHeader,
                rightSectionX + CAPTURED_SECTION_WIDTH / 2 - whiteHeaderWidth / 2,
                capturedY - CAPTURED_HEADER_VERTICAL_OFFSET,
                CAPTURED_HEADER_SIZE, LABEL_COLOR);

            // Draw white's captured pieces (right side)
            drawCapturedPieces(whiteCapturedPieces, rightSectionX, capturedY, true);

            // Draw "Black's Captures" header (left side)
            const char* blackHeader = "Black's Captures";
            int blackHeaderWidth = MeasureText(blackHeader, CAPTURED_HEADER_SIZE);
            DrawText(blackHeader,
                leftSectionX + CAPTURED_SECTION_WIDTH / 2 - blackHeaderWidth / 2,
                capturedY - CAPTURED_HEADER_VERTICAL_OFFSET,
                CAPTURED_HEADER_SIZE, LABEL_COLOR);

            // Draw black's captured pieces (left side)
            drawCapturedPieces(blackCapturedPieces, leftSectionX, capturedY, false);
        }
    }

    // Draw player names and profiles
    const char* activePlayerName = namesRotated ? blackPlayerName : whitePlayerName;
    const char* inactivePlayerName = namesRotated ? whitePlayerName : blackPlayerName;

    // Draw active player (bottom)
    int activeProfileY = offsetY + boardPixelSize + LABEL_MARGIN + LABEL_SIZE + VERTICAL_PADDING;
    // Draw profile picture with proper scaling
    Rectangle sourceRec = { 0, 0, (float)profileTexture.width, (float)profileTexture.height };
    Rectangle destRec = { (float)offsetX, (float)activeProfileY, (float)PROFILE_SIZE, (float)PROFILE_SIZE };
    DrawTexturePro(profileTexture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    DrawText(activePlayerName, offsetX + PROFILE_SIZE + NAME_MARGIN, activeProfileY + (PROFILE_SIZE - PLAYER_NAME_SIZE) / 2, PLAYER_NAME_SIZE, LABEL_COLOR);

    // Draw inactive player (top)
    int inactiveProfileY = offsetY - PROFILE_SIZE - LABEL_MARGIN - VERTICAL_PADDING;
    // Draw profile picture with proper scaling
    destRec = { (float)offsetX, (float)inactiveProfileY, (float)PROFILE_SIZE, (float)PROFILE_SIZE };
    DrawTexturePro(profileTexture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    DrawText(inactivePlayerName, offsetX + PROFILE_SIZE + NAME_MARGIN, inactiveProfileY + (PROFILE_SIZE - PLAYER_NAME_SIZE) / 2, PLAYER_NAME_SIZE, LABEL_COLOR);

    // Draw vertical labels (1 to 8)
    for (int y = 0; y < BOARD_SIZE; y++) {
        // Calculate the correct rank number based on board rotation
        int actualY = boardRotated ? y : (BOARD_SIZE - 1 - y);
        char rankLabel = '1' + actualY;
        char label[2] = {rankLabel, '\0'};
        
        // Draw numbers outside the board area with shadow
        DrawText(  // Draw shadow first
            label,
            offsetX - LABEL_SIZE - LABEL_MARGIN * 3 + SHADOW_OFFSET,
            offsetY + y * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2 + SHADOW_OFFSET,
            LABEL_SIZE,
            SHADOW_COLOR
        );
        DrawText(  // Draw main text
            label,
            offsetX - LABEL_SIZE - LABEL_MARGIN * 3,
            offsetY + y * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2,
            LABEL_SIZE,
            LABEL_COLOR
        );
    }

    // Draw horizontal labels (a to h)
    for (int x = 0; x < BOARD_SIZE; x++) {
        // Calculate the correct file letter based on board rotation
        int actualX = boardRotated ? (BOARD_SIZE - 1 - x) : x;
        char colLabel = 'a' + actualX;
        char label[2] = {colLabel, '\0'};
        
        // Draw below the board with shadow effect
        DrawText(  // Draw shadow first
            label,
            offsetX + x * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2 + SHADOW_OFFSET,
            offsetY + boardPixelSize + LABEL_MARGIN + SHADOW_OFFSET,
            LABEL_SIZE,
            SHADOW_COLOR
        );
        DrawText(  // Draw main text
            label,
            offsetX + x * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2,
            offsetY + boardPixelSize + LABEL_MARGIN,
            LABEL_SIZE,
            LABEL_COLOR
        );
    }

    // Draw resign button
    if (GetGameState() == PLAY) {
        const char* resignText = "Resign";
        const int RESIGN_BUTTON_WIDTH = 100;
        const int RESIGN_BUTTON_HEIGHT = 40;
        const int RESIGN_BUTTON_MARGIN = 20;
        const int RESIGN_TEXT_SIZE = 20;
        
        // Calculate text width for proper centering
        int textWidth = MeasureText(resignText, RESIGN_TEXT_SIZE);
        
        // Position the resign button aligned with the right end of the board
        int resignX = offsetX + boardPixelSize - RESIGN_BUTTON_WIDTH;  // Align to right end
        int resignY = offsetY + boardPixelSize + LABEL_MARGIN + LABEL_SIZE + RESIGN_BUTTON_MARGIN;
        
        // Create button rectangle with exact dimensions
        Rectangle resignButton = {
            (float)resignX,
            (float)resignY,
            (float)RESIGN_BUTTON_WIDTH,
            (float)RESIGN_BUTTON_HEIGHT
        };

        // Create hover rectangle with padding
        Rectangle hoverButton = {
            (float)(resignX - 3),
            (float)(resignY - 17),
            (float)(RESIGN_BUTTON_WIDTH + 5),
            (float)(RESIGN_BUTTON_HEIGHT + 5)
        };
        
        // Draw button with hover effect
        Vector2 mousePos = GetMousePosition();
        Color buttonColor = CheckCollisionPointRec(mousePos, hoverButton) ? LIGHTGRAY : RAYWHITE;
        DrawRectangleRec(resignButton, buttonColor);
        
        // Draw button text centered in the button
        DrawText(resignText,
            resignX + (RESIGN_BUTTON_WIDTH - textWidth) / 2,
            resignY + (RESIGN_BUTTON_HEIGHT - RESIGN_TEXT_SIZE) / 2,
            RESIGN_TEXT_SIZE, BLACK);
    }
}

void Game::DrawBoard() {
    // Get window dimensions for centering
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    // Calculate board offset
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    // Draw background to cover entire screen
    Rectangle sourceRect = { 0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height };
    Rectangle destRect = { 0, 0, (float)windowWidth, (float)windowHeight };
    DrawTexturePro(backgroundTexture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);

    // Draw the chess board squares
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int drawX = boardRotated ? BOARD_SIZE - 1 - x : x;
            int drawY = boardRotated ? BOARD_SIZE - 1 - y : y;
            Color squareColor = ((x + y) % 2 == 0) ? LIGHT_SQUARE : DARK_SQUARE;
            DrawRectangle(
                offsetX + drawX * TILE_SIZE,
                offsetY + drawY * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE,
                squareColor
            );
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

void Game::DrawMenu() {
    // Draw background texture
    DrawTexturePro(
        menuBackgroundTexture,
        Rectangle{0, 0, (float)menuBackgroundTexture.width, (float)menuBackgroundTexture.height},
        Rectangle{0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
        Vector2{0, 0},
        0.0f,
        WHITE
    );

    const char* title = "Chess++";
    const char* developers = "Developers: Shehryar [24K-0569], Sufyan [24K-0806], Faizan [24K-0571]";
    const char* playText = "Play";

    int titleWidth = MeasureText(title, 60);  // Increased font size
    int devWidth = MeasureText(developers, 35);
    int playWidth = MeasureText(playText, 40);  // Increased font size

    // Draw title
    DrawText(title, (GetScreenWidth() - titleWidth) / 2, GetScreenHeight() / 8, 60, RAYWHITE);

    // Draw developers credit
    DrawText(developers, 
        (GetScreenWidth() - devWidth) / 2,
        GetScreenHeight() / 6 + 60,
        35,
        RAYWHITE
    );

    // Draw player name input fields with increased size and adjusted spacing
    int inputWidth = 400;  // Increased from 300
    int inputHeight = 50;  // Increased from 40
    int inputX = (GetScreenWidth() - inputWidth) / 2;
    int inputY = GetScreenHeight() / 3 + 50;  // Moved up from /2 - 100

    // White player name input
    DrawText("White Player Name:", inputX, inputY - 40, 25, RAYWHITE);  // Increased font size from 20
    DrawRectangle(inputX, inputY, inputWidth, inputHeight, whiteNameActive ? LIGHTGRAY : RAYWHITE);
    DrawText(whitePlayerName, inputX + 15, inputY + 12, 25, BLACK);  // Increased font size from 20

    // Black player name input
    DrawText("Black Player Name:", inputX, inputY + 80 + 20, 25, RAYWHITE);  // Increased font size from 20
    DrawRectangle(inputX, inputY + 120 + 20, inputWidth, inputHeight, blackNameActive ? LIGHTGRAY : RAYWHITE);
    DrawText(blackPlayerName, inputX + 15, inputY + 135 + 20 - 3, 25, BLACK);  // Increased font size from 20

    // Draw Play button
    int buttonWidth = playWidth + 100;  // Increased padding
    int buttonHeight = 60;  // Increased height
    int buttonX = (GetScreenWidth() - buttonWidth) / 2;
    int buttonY = GetScreenHeight() / 2 + 50 + 40 + 40;
    
    // Draw button background with hover effect
    Vector2 mousePos = GetMousePosition();
    Color buttonColor = RAYWHITE;
    if (mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth &&
        mousePos.y >= buttonY - (14) && mousePos.y <= buttonY - (14) + buttonHeight) {
        buttonColor = LIGHTGRAY;  // Change color on hover
    }
    DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
    
    // Draw button text
    DrawText(playText, 
        buttonX + (buttonWidth - playWidth) / 2,
        buttonY + (buttonHeight - 40) / 2,  // Adjusted for new font size
        40, 
        BLACK
    );

    // Draw error message if names are not entered
    if (strlen(whitePlayerName) == 0 || strlen(blackPlayerName) == 0) {
        const char* errorMsg = "Please enter names for both players";
        int errorWidth = MeasureText(errorMsg, 25);
        DrawText(errorMsg, 
            (GetScreenWidth() - errorWidth) / 2,
            buttonY + buttonHeight + 20 + 40,
            25,
            RED
        );
    }
}

Vector2 Game::ScreenToBoard(Vector2 screenPos) {
    // Calculate board offset
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
    int offsetY = (GetScreenHeight() - boardPixelSize) / 2;

    // Adjust screen position by offset
    float relativeX = screenPos.x - offsetX;
    float relativeY = screenPos.y - offsetY;

    // Convert to board coordinates
    int boardX = (int)(relativeX / TILE_SIZE);
    int boardY = (int)(relativeY / TILE_SIZE);

    // Handle board rotation
    if (boardRotated) {
        boardX = BOARD_SIZE - 1 - boardX;
        boardY = BOARD_SIZE - 1 - boardY;
    }

    return Vector2{(float)boardX, (float)boardY};
}

Vector2 Game::BoardToScreen(int x, int y) {
    // Get window dimensions for centering
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    // Calculate board offset
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    // Handle board rotation
    if (boardRotated) {
        x = BOARD_SIZE - 1 - x;
        y = BOARD_SIZE - 1 - y;
    }

    return (Vector2){
        static_cast<float>(offsetX + x * TILE_SIZE),
        static_cast<float>(offsetY + y * TILE_SIZE)
    };
}

Vector2 Game::GetCenteredPiecePosition(const Piece& piece) {
    // Get the base board position with proper offset
    Vector2 boardPos = BoardToScreen(piece.GetX(), piece.GetY());

    // Center the piece within its tile
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

bool Game::IsSquareUnderAttack(int x, int y, bool byWhite, Vector2 ignorePiecePos) const {
    const Team& attackingTeam = byWhite ? whiteTeam : blackTeam;
    
    // Check all pieces of the attacking team
    for (const auto& piece : attackingTeam.GetPieces()) {
        // Skip if this is the piece position we're ignoring (for capture validation)
        if (piece->GetX() == ignorePiecePos.x && piece->GetY() == ignorePiecePos.y) {
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

 vector<Vector2> Game::GetValidMoves(Piece* piece) {
    if (!piece) return {};

     vector<Vector2> moves = piece->GetValidMoves(*this);
     vector<Vector2> legalMoves;

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
        Piece* capturedPiece = const_cast<Piece*>(GetPieceAt(move.x, move.y));
        bool wasCaptured = false;

        // Remove captured piece from board temporarily (before moving)
        if (capturedPiece) {
            wasCaptured = true;
            capturedPiece->SetPosition(-1, -1); // Off-board
        }

        // Temporarily move the piece
        piece->SetPosition(move.x, move.y);

        // Check if the move puts our king in check
        bool kingInCheck;
        if (piece->GetType() == PieceType::KING) {
            // FIX: Do not ignore the king's original position — pass dummy value
            kingInCheck = IsSquareUnderAttack(move.x, move.y, !piece->IsWhite(), Vector2{-1, -1});
        } else {
            kingInCheck = IsSquareUnderAttack(ourKing->GetX(), ourKing->GetY(), !piece->IsWhite(), originalPos);
        }

        // Restore board state
        piece->SetPosition(originalPos.x, originalPos.y);
        if (wasCaptured) {
            capturedPiece->SetPosition(move.x, move.y); // Restore captured piece
        }

        // If the move doesn't leave our king in check, it's legal
        if (!kingInCheck) {
            legalMoves.push_back(move);
        }
    }

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
}

bool Game::IsCheckmate(bool isWhite) {
    // Get the team that might be in checkmate
    const Team& team = isWhite ? whiteTeam : blackTeam;
    
    // Find the king
    const Piece* king = nullptr;
    for (const auto& piece : team.GetPieces()) {
        if (piece->GetType() == PieceType::KING) {
            king = piece.get();
            break;
        }
    }
    
    if (!king) return false; // Should never happen in a valid game
    
    // If king is not in check, it's not checkmate
    if (!IsSquareUnderAttack(king->GetX(), king->GetY(), !isWhite)) {
        return false;
    }
    
    // Check if any piece can make a legal move
    for (const auto& piece : team.GetPieces()) {
        auto moves = GetValidMoves(const_cast<Piece*>(piece.get()));
        if (!moves.empty()) {
            return false; // Found a legal move, not checkmate
        }
    }
    
    return true; // No legal moves and king is in check
}

bool Game::IsStalemate(bool isWhite) {
    // Get the team that might be in stalemate
    const Team& team = isWhite ? whiteTeam : blackTeam;
    
    // Find the king
    const Piece* king = nullptr;
    for (const auto& piece : team.GetPieces()) {
        if (piece->GetType() == PieceType::KING) {
            king = piece.get();
            break;
        }
    }
    
    if (!king) return false; // Should never happen in a valid game
    
    // If king is in check, it's not stalemate
    if (IsSquareUnderAttack(king->GetX(), king->GetY(), !isWhite)) {
        return false;
    }
    
    // Check if any piece can make a legal move
    for (const auto& piece : team.GetPieces()) {
        auto moves = GetValidMoves(const_cast<Piece*>(piece.get()));
        if (!moves.empty()) {
            return false; // Found a legal move, not stalemate
        }
    }
    
    return true; // No legal moves and king is not in check
}

void Game::DrawGameOverUI() {
    // Draw semi-transparent background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});

    // Determine if it's checkmate, stalemate, or resignation
    bool isCheckmate = IsCheckmate(isWhiteTurn);
    bool isStalemate = IsStalemate(isWhiteTurn);
    bool isResignation = !isCheckmate && !isStalemate;  // If neither checkmate nor stalemate, it's resignation

    // Constants for UI
    const int TITLE_SIZE = 60;
    const int MESSAGE_SIZE = 35;
    const int CONGRATS_SIZE = 30;
    const Color TEXT_COLOR = RAYWHITE;
    const int LINE_SPACING = 40;
    const int BUTTON_PADDING = 20;
    const int BUTTON_HEIGHT = 50;

    // Calculate positions
    int centerX = GetScreenWidth() / 2;
    int startY = GetScreenHeight() / 3;

    // Draw "Game Over!" title
    const char* title = "Game Over!";
    int titleWidth = MeasureText(title, TITLE_SIZE);
    DrawText(title, centerX - titleWidth / 2, startY, TITLE_SIZE, TEXT_COLOR);

    if (isCheckmate) {
        // Get the winner and loser names
        const char* winnerName = isWhiteTurn ? blackPlayerName : whitePlayerName;
        const char* loserName = isWhiteTurn ? whitePlayerName : blackPlayerName;

        // Draw winner message
        char winnerMsg[100];
        snprintf(winnerMsg, sizeof(winnerMsg), "%s Won!", winnerName);
        int winnerWidth = MeasureText(winnerMsg, MESSAGE_SIZE);
        DrawText(winnerMsg, centerX - winnerWidth / 2, startY + LINE_SPACING * 2, MESSAGE_SIZE, TEXT_COLOR);

        // Draw By Message
        const char* byMsg = "(By Checkmate)";
        int byWidth = MeasureText(byMsg, MESSAGE_SIZE);
        DrawText(byMsg, centerX - byWidth / 2, startY + LINE_SPACING * 4 - 27, MESSAGE_SIZE, TEXT_COLOR);

        // Draw congratulations message
        char congratsMsg[200];
        snprintf(congratsMsg, sizeof(congratsMsg), "Congratulations %s!", winnerName);
        int congratsWidth = MeasureText(congratsMsg, CONGRATS_SIZE);
        DrawText(congratsMsg, centerX - congratsWidth / 2, startY + LINE_SPACING * 6, CONGRATS_SIZE, TEXT_COLOR);

        // Draw Luck Message
        char luckMsg[200];
        snprintf(luckMsg, sizeof(luckMsg), "Better Luck next time %s", loserName);
        int luckWidth = MeasureText(luckMsg, CONGRATS_SIZE);
        DrawText(luckMsg, centerX - luckWidth / 2, startY + LINE_SPACING * 6 + 35, CONGRATS_SIZE, TEXT_COLOR);

    } else if (isStalemate) {
        // Draw stalemate message
        const char* stalemateMsg = "Game Ended in a Draw!";
        int stalemateWidth = MeasureText(stalemateMsg, MESSAGE_SIZE);
        DrawText(stalemateMsg, centerX - stalemateWidth / 2, startY + LINE_SPACING * 2, MESSAGE_SIZE, TEXT_COLOR);

        // Draw By Message
        const char* byMsg = "(By Stalemate)";
        int byWidth = MeasureText(byMsg, MESSAGE_SIZE);
        DrawText(byMsg, centerX - byWidth / 2, startY + LINE_SPACING * 4 - 27, MESSAGE_SIZE, TEXT_COLOR);

        // Draw player names
        char playersMsg[200];
        snprintf(playersMsg, sizeof(playersMsg), "Well played %s and %s!", whitePlayerName, blackPlayerName);
        int playersWidth = MeasureText(playersMsg, CONGRATS_SIZE);
        DrawText(playersMsg, centerX - playersWidth / 2, startY + LINE_SPACING * 6, CONGRATS_SIZE, TEXT_COLOR);
    } else if (isResignation) {
        // Get the winner and loser names
        const char* winnerName = isWhiteTurn ? blackPlayerName : whitePlayerName;
        const char* loserName = isWhiteTurn ? whitePlayerName : blackPlayerName;

        // Draw winner message
        char winnerMsg[100];
        snprintf(winnerMsg, sizeof(winnerMsg), "%s Won!", winnerName);
        int winnerWidth = MeasureText(winnerMsg, MESSAGE_SIZE);
        DrawText(winnerMsg, centerX - winnerWidth / 2, startY + LINE_SPACING * 2, MESSAGE_SIZE, TEXT_COLOR);

        // Draw By Message
        const char* byMsg = "(By Resignation of Opponent)";
        int byWidth = MeasureText(byMsg, MESSAGE_SIZE);
        DrawText(byMsg, centerX - byWidth / 2, startY + LINE_SPACING * 4 - 27, MESSAGE_SIZE, TEXT_COLOR);

        // Draw congratulations message
        char congratsMsg[200];
        snprintf(congratsMsg, sizeof(congratsMsg), "Congratulations %s!", winnerName);
        int congratsWidth = MeasureText(congratsMsg, CONGRATS_SIZE);
        DrawText(congratsMsg, centerX - congratsWidth / 2, startY + LINE_SPACING * 6, CONGRATS_SIZE, TEXT_COLOR);

        // Draw Luck Message
        char luckMsg[200];
        snprintf(luckMsg, sizeof(luckMsg), "Do not give up like that next time %s!", loserName);
        int luckWidth = MeasureText(luckMsg, CONGRATS_SIZE);
        DrawText(luckMsg, centerX - luckWidth / 2, startY + LINE_SPACING * 6 + 35, CONGRATS_SIZE, TEXT_COLOR);
    }

    // Draw buttons
    const char* exitText = "Exit";
    const char* playAgainText = "Play Again";
    
    int exitWidth = MeasureText(exitText, 30) + BUTTON_PADDING * 2;
    int playAgainWidth = MeasureText(playAgainText, 30) + BUTTON_PADDING * 2;
    
    int totalWidth = exitWidth + playAgainWidth + 50; // 50 pixels gap between buttons
    int startX = centerX - totalWidth / 2;
    int buttonY = startY + LINE_SPACING * 8 + 20;

    // Exit button
    Rectangle exitButton = {
        (float)startX,
        (float)buttonY,
        (float)exitWidth,
        (float)BUTTON_HEIGHT
    };
    
    // Play Again button
    Rectangle playAgainButton = {
        (float)(startX + exitWidth + 50),
        (float)buttonY,
        (float)playAgainWidth,
        (float)BUTTON_HEIGHT
    };

    // Draw buttons with hover effect
    Vector2 mousePos = GetMousePosition();
    Color exitColor = CheckCollisionPointRec(mousePos, exitButton) ? LIGHTGRAY : RAYWHITE;
    Color playAgainColor = CheckCollisionPointRec(mousePos, playAgainButton) ? LIGHTGRAY : RAYWHITE;

    DrawRectangleRec(exitButton, exitColor);
    DrawRectangleRec(playAgainButton, playAgainColor);

    // Draw button text
    DrawText(exitText, 
        exitButton.x + (exitButton.width - MeasureText(exitText, 30)) / 2,
        exitButton.y + (exitButton.height - 30) / 2,
        30, BLACK);

    DrawText(playAgainText,
        playAgainButton.x + (playAgainButton.width - MeasureText(playAgainText, 30)) / 2,
        playAgainButton.y + (playAgainButton.height - 30) / 2,
        30, BLACK);
}

void Game::AddCapturedPiece(PieceType type, bool isWhite) {
    if (isWhite) {
        whiteCapturedPieces.push_back(type);
    } else {
        blackCapturedPieces.push_back(type);
    }
}

const  vector<PieceType>& Game::GetWhiteCapturedPieces() const {
    return whiteCapturedPieces;
}

const vector<PieceType>& Game::GetBlackCapturedPieces() const {
    return blackCapturedPieces;
}
