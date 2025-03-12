#include "raylib.h"
#include "Game.h"
#include "Team.h"
#include "Piece.h"
#include "Pawn.h"
#include "King.h"
#include "Queen.h"
#include "Bishop.h"
#include "Knight.h"
#include "Rook.h"

static const int TILE_SIZE = 80;
static const int BOARD_SIZE = 8;
int main() {
    InitWindow(BOARD_SIZE * TILE_SIZE, BOARD_SIZE * TILE_SIZE + 60, "Chess with Raylib");
    SetTargetFPS(60);
    Game chessGame;
    chessGame.Run();
    return 0;
}
