#include "Game.h"
#include "Piece.h"
#include "Team.h"
#include "TextureManager.h"
#include <cmath>  
#include <iostream>
#include <cstring>  
#include "raylib.h"
using namespace std;


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
Texture2D profileTexture;  
Font gameFont;


char whitePlayerName[32] = "";
char blackPlayerName[32] = "";
bool whiteNameActive = false;
bool blackNameActive = false;

float currentRotation = 0.0f;
float targetRotation = 0.0f;
const float rotationSpeed = 2.0f;  


Move lastMove;

Vector2 promotionSquare = {-1, -1};

Game::Game() : 
    whiteTeam(true),
    blackTeam(false),
    selectedPiece(nullptr),
    isWhiteTurn(true),
    boardRotated(false),
    namesRotated(false),  
    currentState(MENU),  
    promotionSquare({-1, -1})
{
    
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);

    
    while (!IsWindowReady()) { }

    
    InitAudioDevice();

    // Load custom font
    gameFont = LoadFont("assets/font.ttf");  // Make sure to add your font file to the assets folder
    SetTextureFilter(gameFont.texture, TEXTURE_FILTER_BILINEAR);
    
    moveSound = LoadSound("assets/move.mp3");

    
    captureSound = LoadSound("assets/capture.mp3");

    
    checkSound = LoadSound("assets/check.mp3");

    
    promotionSound = LoadSound("assets/promote.mp3");

    
    gameStartSound = LoadSound("assets/game_start.mp3");

    
    gameOverSound = LoadSound("assets/game-end.mp3");

    
    checkmateSound = LoadSound("assets/checkmate.mp3");

    
    stalemateSound = LoadSound("assets/stalemate.mp3");

    
    backgroundTexture = LoadTexture("assets/background.jpg");
    
    
    menuBackgroundTexture = LoadTexture("assets/Mainmenu.png");

    
    profileTexture = LoadTexture("assets/Profile-Male-Transparent.png");

    
    auto texManager = TextureManager::GetInstance();
    texManager->Initialize();

    
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
    
    UnloadSound(moveSound);

    
    UnloadSound(captureSound);

    
    UnloadSound(checkSound);

    
    UnloadSound(promotionSound);

    
    UnloadSound(gameStartSound);

    
    UnloadSound(gameOverSound);

    
    UnloadSound(checkmateSound);

    
    UnloadSound(stalemateSound);

    
    UnloadTexture(backgroundTexture);

    
    UnloadTexture(menuBackgroundTexture);

    
    UnloadTexture(profileTexture);

    // Unload custom font
    UnloadFont(gameFont);

    
    CloseAudioDevice();

    
    TextureManager::GetInstance()->UnloadAllTextures();
    TextureManager::Cleanup();
    
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose() && !shouldClose) {
        HandleInput();

        
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
    
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    
    DrawBoard();
    
    
    DrawLabels();

    
    if (GetGameState() == PLAY) {
        
        if (selectedPiece) {
            for (const auto& move : validMoves) {
                int drawX = boardRotated ? BOARD_SIZE - 1 - move.x : move.x;
                int drawY = boardRotated ? BOARD_SIZE - 1 - move.y : move.y;
                if (GetPieceAt(move.x, move.y) && GetPieceAt(move.x, move.y)->GetType() != PieceType::KING) {
                    
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
                           lastMove.end.y == selectedPiece->GetY() &&  
                           abs(move.x - selectedPiece->GetX()) == 1 && 
                           move.y == lastMove.end.y + (selectedPiece->IsWhite() ? -1 : 1)) { 
                    
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

    
    if (whiteKing && IsSquareUnderAttack(whiteKing->GetX(), whiteKing->GetY(), false)) {
        int drawX = boardRotated ? BOARD_SIZE - 1 - whiteKing->GetX() : whiteKing->GetX();
        int drawY = boardRotated ? BOARD_SIZE - 1 - whiteKing->GetY() : whiteKing->GetY();
        DrawRectangle(
            offsetX + drawX * TILE_SIZE,
            offsetY + drawY * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE,
            Color{255, 0, 0, 100} 
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
            Color{255, 0, 0, 100} 
        );
    }

    
    for (const auto& piece : whiteTeam.GetPieces()) {
        Vector2 pos = GetCenteredPiecePosition(*piece);
        Texture2D tex = piece->GetTexture();
        if (tex.id > 0) {  
            DrawTexture(tex, pos.x, pos.y, WHITE);
        } else {
            
            DrawRectangle(pos.x, pos.y, TILE_SIZE/2, TILE_SIZE/2, WHITE);
        }
    }
    
    for (const auto& piece : blackTeam.GetPieces()) {
        Vector2 pos = GetCenteredPiecePosition(*piece);
        Texture2D tex = piece->GetTexture();
        if (tex.id > 0) {  
            DrawTexture(tex, pos.x, pos.y, WHITE);
        } else {
            
            DrawRectangle(pos.x, pos.y, TILE_SIZE/2, TILE_SIZE/2, BLACK);
        }
    }

    
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
                
                DrawCircle(
                    offsetX + (drawX * TILE_SIZE) + TILE_SIZE/2,
                    offsetY + (drawY * TILE_SIZE) + TILE_SIZE/2,
                    10,
                    MOVE_HIGHLIGHT
                );
            }
        }
    }

    
    if (GetGameState() == PROMOTION) {
        DrawPromotionUI();
    }
}
void Game::HandleInput() {
    if (GetGameState() == MENU) {
        Vector2 mousePos = GetMousePosition();

        
        int inputWidth = 400;  
        int inputHeight = 50;  
        int inputX = (GetScreenWidth() - inputWidth) / 2;
        int inputY = GetScreenHeight() / 3 + 50;  

        
        Rectangle whiteInputRect = {
            (float)inputX,
            (float)(inputY - 12),  
            (float)inputWidth,
            (float)inputHeight
        };
        Rectangle blackInputRect = {
            (float)inputX,
            (float)(inputY + 140 - 12),  
            (float)inputWidth,
            (float)inputHeight
        };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            
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

            
            const char* playText = "Play";
            int playWidth = MeasureTextEx(gameFont, playText, 40, 0).x;
            int buttonWidth = playWidth + 100;  
            int buttonHeight = 60;
            int buttonX = (GetScreenWidth() - buttonWidth) / 2;
            int buttonY = GetScreenHeight() / 2 + 50 + 40;  

            
            Rectangle playButtonRect = {
                (float)buttonX,
                (float)(buttonY + (27)),  
                (float)buttonWidth,
                (float)buttonHeight
            };

            if (CheckCollisionPointRec(mousePos, playButtonRect)) {
                if (strlen(whitePlayerName) > 0 && strlen(blackPlayerName) > 0) {
                    
                    if (gameStartSound.stream.buffer != NULL) {
                        PlaySound(gameStartSound);
                    }
                    SetGameState(PLAY);
                }
            }
        }

        
        if (whiteNameActive) {
            int key = GetCharPressed();
            while (key > 0) {
                if (strlen(whitePlayerName) < 31) {  
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
                if (strlen(blackPlayerName) < 31) {  
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
            
            
            int boardPixelSize = TILE_SIZE * BOARD_SIZE;
            int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
            int offsetY = (GetScreenHeight() - boardPixelSize) / 2;

            float spacing = 100.0f;
            float startX = offsetX + (boardPixelSize - spacing * 4) / 2;
            float y = offsetY + (boardPixelSize - 64) / 2;  

            
            for (int i = 0; i < 4; i++) {
                Rectangle pieceRect = {
                    startX + i * spacing,
                    y,
                    64,  
                    64   
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
            
            
            int centerX = GetScreenWidth() / 2;
            int startY = GetScreenHeight() / 3;
            const int BUTTON_PADDING = 20;
            const int BUTTON_HEIGHT = 50;
            
            const char* exitText = "Exit";
            const char* playAgainText = "Play Again";
            
            int exitWidth = MeasureTextEx(gameFont, exitText, 30, 0).x + BUTTON_PADDING * 2;
            int playAgainWidth = MeasureTextEx(gameFont, playAgainText, 30, 0).x + BUTTON_PADDING * 2;
            
            int totalWidth = exitWidth + playAgainWidth + 50;
            int startX = centerX - totalWidth / 2;
            int buttonY = startY + 8 * 40; 

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
                
                isWhiteTurn = true;
                boardRotated = false;
                namesRotated = false;
                selectedPiece = nullptr;
                validMoves.clear();
                
                
                whiteTeam.Reset();
                blackTeam.Reset();
                
                
                whiteCapturedPieces.clear();
                blackCapturedPieces.clear();
                
                
                memset(whitePlayerName, 0, sizeof(whitePlayerName));
                memset(blackPlayerName, 0, sizeof(blackPlayerName));
                whiteNameActive = false;
                blackNameActive = false;
                
                
                SetGameState(MENU);
            }
        }
        return;
    }

    if (GetGameState() == PLAY) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 boardPos = ScreenToBoard(mousePos);
            
            
            int boardPixelSize = TILE_SIZE * BOARD_SIZE;
            int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
            int offsetY = (GetScreenHeight() - boardPixelSize) / 2;
            
            const int RESIGN_BUTTON_WIDTH = 100;
            const int RESIGN_BUTTON_HEIGHT = 40;
            const int RESIGN_BUTTON_MARGIN = 20;
            const int LABEL_MARGIN = 8;
            const int LABEL_SIZE = 20;
            
            Rectangle resignButton = {
                (float)(offsetX + boardPixelSize - RESIGN_BUTTON_WIDTH),  
                (float)(offsetY + boardPixelSize + LABEL_MARGIN + LABEL_SIZE + RESIGN_BUTTON_MARGIN),
                (float)RESIGN_BUTTON_WIDTH,
                (float)RESIGN_BUTTON_HEIGHT
            };
            
            if (CheckCollisionPointRec(mousePos, resignButton)) {
                
                if (gameOverSound.stream.buffer != NULL) {
                    PlaySound(gameOverSound);
                }
                SetGameState(GAME_OVER);
                return;
            }
            
            if (boardPos.x >= 0 && boardPos.x < BOARD_SIZE &&
                boardPos.y >= 0 && boardPos.y < BOARD_SIZE) {
                
                
                Piece* clickedPiece = const_cast<Piece*>(GetPieceAt(boardPos.x, boardPos.y));
                
                if (selectedPiece) {
                    
                    if (clickedPiece && clickedPiece->IsWhite() == isWhiteTurn) {
                        
                        selectedPiece = clickedPiece;
                        validMoves = GetValidMoves(selectedPiece);
                    } else {
                        
                        bool isValidMove = false;
                        for (const auto& move : validMoves) {
                            if (move.x == boardPos.x && move.y == boardPos.y) {
                                isValidMove = true;
                                break;
                            }
                        }
                        
                        if (isValidMove) {
                            MovePiece(boardPos.x, boardPos.y);
                            
                            
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
                            
                            selectedPiece = nullptr;
                            validMoves.clear();
                        }
                    }
                } else if (clickedPiece && clickedPiece->IsWhite() == isWhiteTurn) {
                    
                    selectedPiece = clickedPiece;
                    validMoves = GetValidMoves(selectedPiece);
                }
            }
        }
    }
}

void Game::MovePiece(int x, int y) {
    if (!selectedPiece) return;

    
    Vector2 targetPos = {(float)x, (float)y};
    bool isValidMove = false;
    for (const auto& move : validMoves) {
        if (move.x == targetPos.x && move.y == targetPos.y) {
            isValidMove = true;
            break;
        }
    }

    if (isValidMove) {
        
        if (selectedPiece->GetType() == PieceType::PAWN &&
            abs(lastMove.end.y - lastMove.start.y) == 2 &&
            lastMove.piece->GetType() == PieceType::PAWN &&
            lastMove.end.x == x &&
            lastMove.end.y == selectedPiece->GetY() &&  
            abs(x - selectedPiece->GetX()) == 1 &&      
            y == lastMove.end.y + (selectedPiece->IsWhite() ? -1 : 1)) {  

            
            Piece* capturedPawn = const_cast<Piece*>(GetPieceAt(x, lastMove.end.y));
            if (capturedPawn) {
                AddCapturedPiece(capturedPawn->GetType(), capturedPawn->IsWhite());
                if (capturedPawn->IsWhite()) {
                    whiteTeam.RemovePieceAt(x, lastMove.end.y);
                } else {
                    blackTeam.RemovePieceAt(x, lastMove.end.y);
                }
            }

            
            if (captureSound.stream.buffer != NULL) {
                PlaySound(captureSound);
            }
        }

        
        Piece* targetPiece = const_cast<Piece*>(GetPieceAt(x, y));
        if (targetPiece && targetPiece->IsWhite() != selectedPiece->IsWhite()) {
            if (targetPiece->GetType() == PieceType::KING) {
                
                return;
            }
            AddCapturedPiece(targetPiece->GetType(), targetPiece->IsWhite());
            if (targetPiece->IsWhite()) {
                whiteTeam.RemovePieceAt(x, y);
            } else {
                blackTeam.RemovePieceAt(x, y);
            }
            
            if (captureSound.stream.buffer != NULL) {
                PlaySound(captureSound);
            }
        }

        
        lastMove = {selectedPiece->GetPosition(), targetPos, selectedPiece};

        selectedPiece->SetPosition(x, y);

        
        if (selectedPiece->GetType() == PieceType::PAWN) {
            int promotionRank = selectedPiece->IsWhite() ? 0 : 7;
            if (y == promotionRank) {
                promotionSquare = {(float)x, (float)y};
                SetGameState(PROMOTION);
                
                return;
            }
        }

        
        
        if ((!targetPiece || targetPiece->IsWhite() == selectedPiece->IsWhite()) &&
            moveSound.stream.buffer != NULL) {
            PlaySound(moveSound);
        }

        
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
            
            else if (checkSound.stream.buffer != NULL) {
                PlaySound(checkSound);
            }
        }

        
        if (GetGameState() != PROMOTION) {
            isWhiteTurn = !isWhiteTurn;
            boardRotated = !boardRotated;
            namesRotated = !namesRotated;
        }
    }

    
    selectedPiece = nullptr;
    validMoves.clear();
}

void Game::PromotePawn(PieceType type) {
    
    Team& team = isWhiteTurn ? whiteTeam : blackTeam;
    team.RemovePieceAt(promotionSquare.x, promotionSquare.y);
    team.AddPiece(type, promotionSquare.x, promotionSquare.y);

    
    if (promotionSound.stream.buffer != NULL) {
        PlaySound(promotionSound);
        }

    
    selectedPiece = nullptr;
    validMoves.clear();

    
    isWhiteTurn = !isWhiteTurn;
    boardRotated = !boardRotated;
    namesRotated = !namesRotated;
    
    

    
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
    
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});

    
    auto texManager = TextureManager::GetInstance();
    
    
    bool isWhitePiece = isWhiteTurn;

    
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
    int offsetY = (GetScreenHeight() - boardPixelSize) / 2;

    
    const  vector< string> pieceTypes = {"queen", "rook", "bishop", "knight"};
    float spacing = 100.0f;
    float startX = offsetX + (boardPixelSize - spacing * pieceTypes.size()) / 2;
    float y = offsetY + (boardPixelSize - 64) / 2;  

    for (size_t i = 0; i < pieceTypes.size(); i++) {
        Vector2 piecePos = {startX + i * spacing, y};
         string prefix = isWhitePiece ? "white_" : "black_";
        Texture2D texture = texManager->GetTexture(prefix + pieceTypes[i]);
        
        DrawTexture(texture, piecePos.x, piecePos.y, WHITE);  
    }
}

void Game::DrawLabels() {
    
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    
    const int LABEL_SIZE = 20;
    const int LABEL_MARGIN = 8;
    const Color LABEL_COLOR = RAYWHITE;
    const Color SHADOW_COLOR = BLACK;
    const int SHADOW_OFFSET = 1;
    const int PROFILE_SIZE = 32;
    const int NAME_MARGIN = 12;
    const int PLAYER_NAME_SIZE = 24;
    const int VERTICAL_PADDING = 20;

    
    const int CAPTURED_PIECE_SIZE = 30;
    const int CAPTURED_PIECE_SPACING = 15;
    const int CAPTURED_HEADER_SIZE = 25;
    const int CAPTURED_SECTION_MARGIN = 40;
    const int CAPTURED_HEADER_VERTICAL_OFFSET = 50;
    const int CAPTURED_SECTION_WIDTH = 200;  
    const int CAPTURED_LINE_SPACING = 40;    

    
    if (GetGameState() == PLAY) {
        
        auto texManager = TextureManager::GetInstance();

        
        int leftSectionX = (windowWidth - boardPixelSize) / 4 - CAPTURED_SECTION_WIDTH / 2;
        int rightSectionX = windowWidth - (windowWidth - boardPixelSize) / 4 - CAPTURED_SECTION_WIDTH / 2;
        int capturedY = offsetY + boardPixelSize / 2;

        
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

        
        if (this->isWhiteTurn) {
            
            
            const char* blackHeader = "Black's Captures";
            int blackHeaderWidth = MeasureTextEx(gameFont, blackHeader, CAPTURED_HEADER_SIZE, 0).x;
            DrawTextEx(gameFont, blackHeader,
                Vector2{(float)(rightSectionX + CAPTURED_SECTION_WIDTH / 2 - blackHeaderWidth / 2), (float)(capturedY - CAPTURED_HEADER_VERTICAL_OFFSET)},
                CAPTURED_HEADER_SIZE, 0, LABEL_COLOR);

            
            drawCapturedPieces(blackCapturedPieces, rightSectionX, capturedY, false);

            
            const char* whiteHeader = "White's Captures";
            int whiteHeaderWidth = MeasureTextEx(gameFont, whiteHeader, CAPTURED_HEADER_SIZE, 0).x;
            DrawTextEx(gameFont, whiteHeader,
                Vector2{(float)(leftSectionX + CAPTURED_SECTION_WIDTH / 2 - whiteHeaderWidth / 2), (float)(capturedY - CAPTURED_HEADER_VERTICAL_OFFSET)},
                CAPTURED_HEADER_SIZE, 0, LABEL_COLOR);

            
            drawCapturedPieces(whiteCapturedPieces, leftSectionX, capturedY, true);
        } else {
            
            
            const char* whiteHeader = "White's Captures";
            int whiteHeaderWidth = MeasureTextEx(gameFont, whiteHeader, CAPTURED_HEADER_SIZE, 0).x;
            DrawTextEx(gameFont, whiteHeader,
                Vector2{(float)(rightSectionX + CAPTURED_SECTION_WIDTH / 2 - whiteHeaderWidth / 2), (float)(capturedY - CAPTURED_HEADER_VERTICAL_OFFSET)},
                CAPTURED_HEADER_SIZE, 0, LABEL_COLOR);

            
            drawCapturedPieces(whiteCapturedPieces, rightSectionX, capturedY, true);

            
            const char* blackHeader = "Black's Captures";
            int blackHeaderWidth = MeasureTextEx(gameFont, blackHeader, CAPTURED_HEADER_SIZE, 0).x;
            DrawTextEx(gameFont, blackHeader,
                Vector2{(float)(leftSectionX + CAPTURED_SECTION_WIDTH / 2 - blackHeaderWidth / 2), (float)(capturedY - CAPTURED_HEADER_VERTICAL_OFFSET)},
                CAPTURED_HEADER_SIZE, 0, LABEL_COLOR);

            
            drawCapturedPieces(blackCapturedPieces, leftSectionX, capturedY, false);
        }
    }

    
    const char* activePlayerName = namesRotated ? blackPlayerName : whitePlayerName;
    const char* inactivePlayerName = namesRotated ? whitePlayerName : blackPlayerName;

    
    int activeProfileY = offsetY + boardPixelSize + LABEL_MARGIN + LABEL_SIZE + VERTICAL_PADDING;
    
    Rectangle sourceRec = { 0, 0, (float)profileTexture.width, (float)profileTexture.height };
    Rectangle destRec = { (float)offsetX, (float)activeProfileY, (float)PROFILE_SIZE, (float)PROFILE_SIZE };
    DrawTexturePro(profileTexture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    DrawTextEx(gameFont, activePlayerName, Vector2{(float)(offsetX + PROFILE_SIZE + NAME_MARGIN), (float)(activeProfileY + (PROFILE_SIZE - PLAYER_NAME_SIZE) / 2)}, PLAYER_NAME_SIZE, 0, LABEL_COLOR);

    
    int inactiveProfileY = offsetY - PROFILE_SIZE - LABEL_MARGIN - VERTICAL_PADDING;
    
    destRec = { (float)offsetX, (float)inactiveProfileY, (float)PROFILE_SIZE, (float)PROFILE_SIZE };
    DrawTexturePro(profileTexture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    DrawTextEx(gameFont, inactivePlayerName, Vector2{(float)(offsetX + PROFILE_SIZE + NAME_MARGIN), (float)(inactiveProfileY + (PROFILE_SIZE - PLAYER_NAME_SIZE) / 2)}, PLAYER_NAME_SIZE, 0, LABEL_COLOR);

    
    for (int y = 0; y < BOARD_SIZE; y++) {
        
        int actualY = boardRotated ? y : (BOARD_SIZE - 1 - y);
        char rankLabel = '1' + actualY;
        char label[2] = {rankLabel, '\0'};
        
        
        DrawTextEx(gameFont, label,
            Vector2{(float)(offsetX - LABEL_SIZE - LABEL_MARGIN * 3 + SHADOW_OFFSET), (float)(offsetY + y * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2 + SHADOW_OFFSET)},
            LABEL_SIZE, 0, SHADOW_COLOR);
        DrawTextEx(gameFont, label,
            Vector2{(float)(offsetX - LABEL_SIZE - LABEL_MARGIN * 3), (float)(offsetY + y * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2)},
            LABEL_SIZE, 0, LABEL_COLOR);
    }

    
    for (int x = 0; x < BOARD_SIZE; x++) {
        
        int actualX = boardRotated ? (BOARD_SIZE - 1 - x) : x;
        char colLabel = 'a' + actualX;
        char label[2] = {colLabel, '\0'};
        
        
        DrawTextEx(gameFont, label,
            Vector2{(float)(offsetX + x * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2 + SHADOW_OFFSET), (float)(offsetY + boardPixelSize + LABEL_MARGIN + SHADOW_OFFSET)},
            LABEL_SIZE, 0, SHADOW_COLOR);
        DrawTextEx(gameFont, label,
            Vector2{(float)(offsetX + x * TILE_SIZE + (TILE_SIZE - LABEL_SIZE) / 2), (float)(offsetY + boardPixelSize + LABEL_MARGIN)},
            LABEL_SIZE, 0, LABEL_COLOR);
    }

    
    if (GetGameState() == PLAY) {
        const char* resignText = "Resign";
        const int RESIGN_BUTTON_WIDTH = 100;
        const int RESIGN_BUTTON_HEIGHT = 40;
        const int RESIGN_BUTTON_MARGIN = 20;
        const int RESIGN_TEXT_SIZE = 20;
        
        
        int textWidth = MeasureTextEx(gameFont, resignText, RESIGN_TEXT_SIZE, 0).x;
        
        
        int resignX = offsetX + boardPixelSize - RESIGN_BUTTON_WIDTH;  
        int resignY = offsetY + boardPixelSize + LABEL_MARGIN + LABEL_SIZE + RESIGN_BUTTON_MARGIN;
        
        
        Rectangle resignButton = {
            (float)resignX,
            (float)resignY,
            (float)RESIGN_BUTTON_WIDTH,
            (float)RESIGN_BUTTON_HEIGHT
        };

        
        Rectangle hoverButton = {
            (float)(resignX - 3),
            (float)(resignY - 17),
            (float)(RESIGN_BUTTON_WIDTH + 5),
            (float)(RESIGN_BUTTON_HEIGHT + 5)
        };
        
        
        Vector2 mousePos = GetMousePosition();
        Color buttonColor = CheckCollisionPointRec(mousePos, hoverButton) ? LIGHTGRAY : RAYWHITE;
        DrawRectangleRec(resignButton, buttonColor);
        
        
        DrawTextEx(gameFont, resignText,
            Vector2{(float)(resignX + (RESIGN_BUTTON_WIDTH - textWidth) / 2), (float)(resignY + (RESIGN_BUTTON_HEIGHT - RESIGN_TEXT_SIZE) / 2)},
            RESIGN_TEXT_SIZE, 0, BLACK);
    }
}

void Game::DrawBoard() {
    
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    
    Rectangle sourceRect = { 0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height };
    Rectangle destRect = { 0, 0, (float)windowWidth, (float)windowHeight };
    DrawTexturePro(backgroundTexture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);

    
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

    
    for (const auto& piece : whiteTeam.GetPieces()) {
        Vector2 pos = GetCenteredPiecePosition(*piece);
        Texture2D tex = piece->GetTexture();
        if (tex.id > 0) {  
            DrawTexture(tex, pos.x, pos.y, WHITE);
        } else {
            
            DrawRectangle(pos.x, pos.y, TILE_SIZE/2, TILE_SIZE/2, WHITE);
        }
    }
    
    for (const auto& piece : blackTeam.GetPieces()) {
        Vector2 pos = GetCenteredPiecePosition(*piece);
        Texture2D tex = piece->GetTexture();
        if (tex.id > 0) {  
            DrawTexture(tex, pos.x, pos.y, WHITE);
        } else {
            
            DrawRectangle(pos.x, pos.y, TILE_SIZE/2, TILE_SIZE/2, BLACK);
        }
    }
}

void Game::DrawMenu() {
    
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

    int titleWidth = MeasureTextEx(gameFont, title, 60, 0).x;  
    int devWidth = MeasureTextEx(gameFont, developers, 35, 0).x;
    int playWidth = MeasureTextEx(gameFont, playText, 40, 0).x;  

    
    DrawTextEx(gameFont, title, Vector2{(float)(GetScreenWidth() - titleWidth) / 2, (float)GetScreenHeight() / 8}, 60, 0, RAYWHITE);

    
    DrawTextEx(gameFont, developers, 
        Vector2{(float)(GetScreenWidth() - devWidth) / 2, (float)GetScreenHeight() / 6 + 60},
        35, 0, RAYWHITE
    );

    
    int inputWidth = 400;  
    int inputHeight = 50;  
    int inputX = (GetScreenWidth() - inputWidth) / 2;
    int inputY = GetScreenHeight() / 3 + 50;  

    
    DrawTextEx(gameFont, "White Player Name:", Vector2{(float)inputX, (float)(inputY - 40)}, 25, 0, RAYWHITE);  
    DrawRectangle(inputX, inputY, inputWidth, inputHeight, whiteNameActive ? LIGHTGRAY : RAYWHITE);
    DrawTextEx(gameFont, whitePlayerName, Vector2{(float)(inputX + 15), (float)(inputY + 12)}, 25, 0, BLACK);  

    
    DrawTextEx(gameFont, "Black Player Name:", Vector2{(float)inputX, (float)(inputY + 80 + 20)}, 25, 0, RAYWHITE);  
    DrawRectangle(inputX, inputY + 120 + 20, inputWidth, inputHeight, blackNameActive ? LIGHTGRAY : RAYWHITE);
    DrawTextEx(gameFont, blackPlayerName, Vector2{(float)(inputX + 15), (float)(inputY + 135 + 20 - 3)}, 25, 0, BLACK);  

    
    int buttonWidth = playWidth + 100;  
    int buttonHeight = 60;  
    int buttonX = (GetScreenWidth() - buttonWidth) / 2;
    int buttonY = GetScreenHeight() / 2 + 50 + 40 + 40;
    
    
    Vector2 mousePos = GetMousePosition();
    Color buttonColor = RAYWHITE;
    if (mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth &&
        mousePos.y >= buttonY - (14) && mousePos.y <= buttonY - (14) + buttonHeight) {
        buttonColor = LIGHTGRAY;  
    }
    DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
    
    
    DrawTextEx(gameFont, playText, 
        Vector2{(float)(buttonX + (buttonWidth - playWidth) / 2), (float)(buttonY + (buttonHeight - 40) / 2)},
        40, 0, BLACK
    );

    
    if (strlen(whitePlayerName) == 0 || strlen(blackPlayerName) == 0) {
        const char* errorMsg = "Please enter names for both players";
        int errorWidth = MeasureTextEx(gameFont, errorMsg, 25, 0).x;
        DrawTextEx(gameFont, errorMsg, 
            Vector2{(float)(GetScreenWidth() - errorWidth) / 2, (float)(buttonY + buttonHeight + 20 + 40)},
            25, 0, RED
        );
    }
}

Vector2 Game::ScreenToBoard(Vector2 screenPos) {
    
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (GetScreenWidth() - boardPixelSize) / 2;
    int offsetY = (GetScreenHeight() - boardPixelSize) / 2;

    
    float relativeX = screenPos.x - offsetX;
    float relativeY = screenPos.y - offsetY;

    
    int boardX = (int)(relativeX / TILE_SIZE);
    int boardY = (int)(relativeY / TILE_SIZE);

    
    if (boardRotated) {
        boardX = BOARD_SIZE - 1 - boardX;
        boardY = BOARD_SIZE - 1 - boardY;
    }

    return Vector2{(float)boardX, (float)boardY};
}

Vector2 Game::BoardToScreen(int x, int y) {
    
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    
    
    int boardPixelSize = TILE_SIZE * BOARD_SIZE;
    int offsetX = (windowWidth - boardPixelSize) / 2;
    int offsetY = (windowHeight - boardPixelSize) / 2;

    
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
    
    selectedPiece = nullptr;
    validMoves.clear();

    
    const Piece* piece = GetPieceAt(x, y);
    if (piece && piece->IsWhite() == isWhiteTurn) {
        selectedPiece = const_cast<Piece*>(piece);
        validMoves = GetValidMoves(selectedPiece);
    }
}

bool Game::IsSquareUnderAttack(int x, int y, bool byWhite, Vector2 ignorePiecePos) const {
    const Team& attackingTeam = byWhite ? whiteTeam : blackTeam;
    
    
    for (const auto& piece : attackingTeam.GetPieces()) {
        
        if (piece->GetX() == ignorePiecePos.x && piece->GetY() == ignorePiecePos.y) {
            continue;
        }
        
        
        auto moves = piece->GetValidMoves(*this);
        
        
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

    
    const Team& ourTeam = piece->IsWhite() ? whiteTeam : blackTeam;
    const Piece* ourKing = nullptr;
    for (const auto& p : ourTeam.GetPieces()) {
        if (p->GetType() == PieceType::KING) {
            ourKing = p.get();
            break;
        }
    }

    if (!ourKing) return moves; 

    
    Vector2 originalPos = piece->GetPosition();

    
    for (const auto& move : moves) {
        
        Piece* capturedPiece = const_cast<Piece*>(GetPieceAt(move.x, move.y));
        bool wasCaptured = false;

        
        if (capturedPiece) {
            wasCaptured = true;
            capturedPiece->SetPosition(-1, -1); 
        }

        
        piece->SetPosition(move.x, move.y);

        
        bool kingInCheck;
        if (piece->GetType() == PieceType::KING) {
            
            kingInCheck = IsSquareUnderAttack(move.x, move.y, !piece->IsWhite(), Vector2{-1, -1});
        } else {
            kingInCheck = IsSquareUnderAttack(ourKing->GetX(), ourKing->GetY(), !piece->IsWhite(), originalPos);
        }

        
        piece->SetPosition(originalPos.x, originalPos.y);
        if (wasCaptured) {
            capturedPiece->SetPosition(move.x, move.y); 
        }

        
        if (!kingInCheck) {
            legalMoves.push_back(move);
        }
    }

    return legalMoves;
}

const Piece* Game::GetPieceAt(int x, int y) const {
    
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

void Game::ToggleBoardRotation() {
    boardRotated = !boardRotated;
}

bool Game::IsCheckmate(bool isWhite) {
    
    const Team& team = isWhite ? whiteTeam : blackTeam;
    
    
    const Piece* king = nullptr;
    for (const auto& piece : team.GetPieces()) {
        if (piece->GetType() == PieceType::KING) {
            king = piece.get();
            break;
        }
    }
    
    if (!king) return false; 
    
    
    if (!IsSquareUnderAttack(king->GetX(), king->GetY(), !isWhite)) {
        return false;
    }
    
    
    for (const auto& piece : team.GetPieces()) {
        auto moves = GetValidMoves(const_cast<Piece*>(piece.get()));
        if (!moves.empty()) {
            return false; 
        }
    }
    
    return true; 
}

bool Game::IsStalemate(bool isWhite) {
    
    const Team& team = isWhite ? whiteTeam : blackTeam;
    
    
    const Piece* king = nullptr;
    for (const auto& piece : team.GetPieces()) {
        if (piece->GetType() == PieceType::KING) {
            king = piece.get();
            break;
        }
    }
    
    if (!king) return false; 
    
    
    if (IsSquareUnderAttack(king->GetX(), king->GetY(), !isWhite)) {
        return false;
    }
    
    
    for (const auto& piece : team.GetPieces()) {
        auto moves = GetValidMoves(const_cast<Piece*>(piece.get()));
        if (!moves.empty()) {
            return false; 
        }
    }
    
    return true; 
}

void Game::DrawGameOverUI() {
    
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});

    
    bool isCheckmate = IsCheckmate(isWhiteTurn);
    bool isStalemate = IsStalemate(isWhiteTurn);
    bool isResignation = !isCheckmate && !isStalemate;  

    
    const int TITLE_SIZE = 60;
    const int MESSAGE_SIZE = 35;
    const int CONGRATS_SIZE = 30;
    const Color TEXT_COLOR = RAYWHITE;
    const int LINE_SPACING = 40;
    const int BUTTON_PADDING = 20;
    const int BUTTON_HEIGHT = 50;

    
    int centerX = GetScreenWidth() / 2;
    int startY = GetScreenHeight() / 3;

    
    const char* title = "Game Over!";
    int titleWidth = MeasureTextEx(gameFont, title, TITLE_SIZE, 0).x;
    DrawTextEx(gameFont, title, Vector2{(float)(centerX - titleWidth / 2), (float)startY}, TITLE_SIZE, 0, TEXT_COLOR);

    if (isCheckmate) {
        
        const char* winnerName = isWhiteTurn ? blackPlayerName : whitePlayerName;
        const char* loserName = isWhiteTurn ? whitePlayerName : blackPlayerName;

        
        char winnerMsg[100];
        snprintf(winnerMsg, sizeof(winnerMsg), "%s Won!", winnerName);
        int winnerWidth = MeasureTextEx(gameFont, winnerMsg, MESSAGE_SIZE, 0).x;
        DrawTextEx(gameFont, winnerMsg, Vector2{(float)(centerX - winnerWidth / 2), (float)(startY + LINE_SPACING * 2)}, MESSAGE_SIZE, 0, TEXT_COLOR);

        
        const char* byMsg = "(By Checkmate)";
        int byWidth = MeasureTextEx(gameFont, byMsg, MESSAGE_SIZE, 0).x;
        DrawTextEx(gameFont, byMsg, Vector2{(float)(centerX - byWidth / 2), (float)(startY + LINE_SPACING * 4 - 27)}, MESSAGE_SIZE, 0, TEXT_COLOR);

        
        char congratsMsg[200];
        snprintf(congratsMsg, sizeof(congratsMsg), "Congratulations %s!", winnerName);
        int congratsWidth = MeasureTextEx(gameFont, congratsMsg, CONGRATS_SIZE, 0).x;
        DrawTextEx(gameFont, congratsMsg, Vector2{(float)(centerX - congratsWidth / 2), (float)(startY + LINE_SPACING * 6)}, CONGRATS_SIZE, 0, TEXT_COLOR);

        
        char luckMsg[200];
        snprintf(luckMsg, sizeof(luckMsg), "Better Luck next time %s", loserName);
        int luckWidth = MeasureTextEx(gameFont, luckMsg, CONGRATS_SIZE, 0).x;
        DrawTextEx(gameFont, luckMsg, Vector2{(float)(centerX - luckWidth / 2), (float)(startY + LINE_SPACING * 6 + 35)}, CONGRATS_SIZE, 0, TEXT_COLOR);

    } else if (isStalemate) {
        
        const char* stalemateMsg = "Game Ended in a Draw!";
        int stalemateWidth = MeasureTextEx(gameFont, stalemateMsg, MESSAGE_SIZE, 0).x;
        DrawTextEx(gameFont, stalemateMsg, Vector2{(float)(centerX - stalemateWidth / 2), (float)(startY + LINE_SPACING * 2)}, MESSAGE_SIZE, 0, TEXT_COLOR);

        
        const char* byMsg = "(By Stalemate)";
        int byWidth = MeasureTextEx(gameFont, byMsg, MESSAGE_SIZE, 0).x;
        DrawTextEx(gameFont, byMsg, Vector2{(float)(centerX - byWidth / 2), (float)(startY + LINE_SPACING * 4 - 27)}, MESSAGE_SIZE, 0, TEXT_COLOR);

        
        char playersMsg[200];
        snprintf(playersMsg, sizeof(playersMsg), "Well played %s and %s!", whitePlayerName, blackPlayerName);
        int playersWidth = MeasureTextEx(gameFont, playersMsg, CONGRATS_SIZE, 0).x;
        DrawTextEx(gameFont, playersMsg, Vector2{(float)(centerX - playersWidth / 2), (float)(startY + LINE_SPACING * 6)}, CONGRATS_SIZE, 0, TEXT_COLOR);
    } else if (isResignation) {
        
        const char* winnerName = isWhiteTurn ? blackPlayerName : whitePlayerName;
        const char* loserName = isWhiteTurn ? whitePlayerName : blackPlayerName;

        
        char winnerMsg[100];
        snprintf(winnerMsg, sizeof(winnerMsg), "%s Won!", winnerName);
        int winnerWidth = MeasureTextEx(gameFont, winnerMsg, MESSAGE_SIZE, 0).x;
        DrawTextEx(gameFont, winnerMsg, Vector2{(float)(centerX - winnerWidth / 2), (float)(startY + LINE_SPACING * 2)}, MESSAGE_SIZE, 0, TEXT_COLOR);

        
        const char* byMsg = "(By Resignation of Opponent)";
        int byWidth = MeasureTextEx(gameFont, byMsg, MESSAGE_SIZE, 0).x;
        DrawTextEx(gameFont, byMsg, Vector2{(float)(centerX - byWidth / 2), (float)(startY + LINE_SPACING * 4 - 27)}, MESSAGE_SIZE, 0, TEXT_COLOR);

        
        char congratsMsg[200];
        snprintf(congratsMsg, sizeof(congratsMsg), "Congratulations %s!", winnerName);
        int congratsWidth = MeasureTextEx(gameFont, congratsMsg, CONGRATS_SIZE, 0).x;
        DrawTextEx(gameFont, congratsMsg, Vector2{(float)(centerX - congratsWidth / 2), (float)(startY + LINE_SPACING * 6)}, CONGRATS_SIZE, 0, TEXT_COLOR);

        
        char luckMsg[200];
        snprintf(luckMsg, sizeof(luckMsg), "Do not give up like that next time %s!", loserName);
        int luckWidth = MeasureTextEx(gameFont, luckMsg, CONGRATS_SIZE, 0).x;
        DrawTextEx(gameFont, luckMsg, Vector2{(float)(centerX - luckWidth / 2), (float)(startY + LINE_SPACING * 6 + 35)}, CONGRATS_SIZE, 0, TEXT_COLOR);
    }

    
    const char* exitText = "Exit";
    const char* playAgainText = "Play Again";
    
    int exitWidth = MeasureTextEx(gameFont, exitText, 30, 0).x + BUTTON_PADDING * 2;
    int playAgainWidth = MeasureTextEx(gameFont, playAgainText, 30, 0).x + BUTTON_PADDING * 2;
    
    int totalWidth = exitWidth + playAgainWidth + 50; 
    int startX = centerX - totalWidth / 2;
    int buttonY = startY + LINE_SPACING * 8 + 20;

    
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

    
    Vector2 mousePos = GetMousePosition();
    Color exitColor = CheckCollisionPointRec(mousePos, exitButton) ? LIGHTGRAY : RAYWHITE;
    Color playAgainColor = CheckCollisionPointRec(mousePos, playAgainButton) ? LIGHTGRAY : RAYWHITE;

    DrawRectangleRec(exitButton, exitColor);
    DrawRectangleRec(playAgainButton, playAgainColor);

    
    int exitTextWidth = MeasureTextEx(gameFont, exitText, 30, 0).x;
    int playAgainTextWidth = MeasureTextEx(gameFont, playAgainText, 30, 0).x;

    DrawTextEx(gameFont, exitText, Vector2{exitButton.x + (exitButton.width - exitTextWidth) / 2, exitButton.y + (exitButton.height - 30) / 2}, 30, 0, BLACK);
    DrawTextEx(gameFont, playAgainText, Vector2{playAgainButton.x + (playAgainButton.width - playAgainTextWidth) / 2, playAgainButton.y + (playAgainButton.height - 30) / 2}, 30, 0, BLACK);
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
