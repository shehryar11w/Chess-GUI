#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

int main()
{
    // Create the window
    sf::RenderWindow window(sf::VideoMode({488, 512}), "Play Chess");

    // Load the chessboard texture
    sf::Texture boardTexture;
    if (!boardTexture.loadFromFile("assets/BlankBoard.png"))
    {
        std::cerr << "Error loading board texture." << std::endl;
        return -1; // Handle error if the texture fails to load
    }
    sf::Sprite boardSprite(boardTexture);

    // Load textures for white pieces
    sf::Texture whitePawnTexture, whiteRookTexture, whiteKnightTexture, whiteBishopTexture, whiteQueenTexture, whiteKingTexture;
    if (!whitePawnTexture.loadFromFile("assets/Piece=Pawn, Side=White.png") ||
        !whiteRookTexture.loadFromFile("assets/Piece=Rook, Side=White.png") ||
        !whiteKnightTexture.loadFromFile("assets/Piece=Knight, Side=White.png") ||
        !whiteBishopTexture.loadFromFile("assets/Piece=Bishop, Side=White.png") ||
        !whiteQueenTexture.loadFromFile("assets/Piece=Queen, Side=White.png") ||
        !whiteKingTexture.loadFromFile("assets/Piece=King, Side=White.png"))
    {
        std::cerr << "Error loading white piece textures." << std::endl;
        return -1; // Handle error if any white piece texture fails to load
    }

    // Load textures for black pieces
    sf::Texture blackPawnTexture, blackRookTexture, blackKnightTexture, blackBishopTexture, blackQueenTexture, blackKingTexture;
    if (!blackPawnTexture.loadFromFile("assets/Piece=Pawn, Side=Black.png") ||
        !blackRookTexture.loadFromFile("assets/Piece=Rook, Side=Black.png") ||
        !blackKnightTexture.loadFromFile("assets/Piece=Knight, Side=Black.png") ||
        !blackBishopTexture.loadFromFile("assets/Piece=Bishop, Side=Black.png") ||
        !blackQueenTexture.loadFromFile("assets/Piece=Queen, Side=Black.png") ||
        !blackKingTexture.loadFromFile("assets/Piece=King, Side=Black.png"))
    {
        std::cerr << "Error loading black piece textures." << std::endl;
        return -1; // Handle error if any black piece texture fails to load
    }

    // Create sprites for each chess piece
    std::vector<sf::Sprite> pieces;

    // White pawns
    for (int i = 0; i < 8; ++i)
    {
        sf::Sprite whitePawn(whitePawnTexture);
        whitePawn.setPosition({61.0f * i, 61.0f * 6}); // Position at row 6 (white's pawns)
        pieces.push_back(whitePawn);
    }

    // Black pawns
    for (int i = 0; i < 8; ++i)
    {
        sf::Sprite blackPawn(blackPawnTexture);
        blackPawn.setPosition({61.0f * i, 61.0f * 1}); // Position at row 1 (black's pawns)
        pieces.push_back(blackPawn);
    }

    // Place rooks
    sf::Sprite whiteRook1(whiteRookTexture), whiteRook2(whiteRookTexture);
    whiteRook1.setPosition({61.0f * 0, 61.0f * 7}); // Position at (0, 7)
    whiteRook2.setPosition({61.0f * 7, 61.0f * 7}); // Position at (7, 7)
    pieces.push_back(whiteRook1);
    pieces.push_back(whiteRook2);

    sf::Sprite blackRook1(blackRookTexture), blackRook2(blackRookTexture);
    blackRook1.setPosition({61.0f * 0, 61.0f * 0}); // Position at (0, 0)
    blackRook2.setPosition({61.0f * 7, 61.0f * 0}); // Position at (7, 0)
    pieces.push_back(blackRook1);
    pieces.push_back(blackRook2);

    // Place knights
    sf::Sprite whiteKnight1(whiteKnightTexture), whiteKnight2(whiteKnightTexture);
    whiteKnight1.setPosition({61.0f * 1, 61.0f * 7}); // Position at (1, 7)
    whiteKnight2.setPosition({61.0f * 6, 61.0f * 7}); // Position at (6, 7)
    pieces.push_back(whiteKnight1);
    pieces.push_back(whiteKnight2);

    sf::Sprite blackKnight1(blackKnightTexture), blackKnight2(blackKnightTexture);
    blackKnight1.setPosition({61.0f * 1, 61.0f * 0}); // Position at (1, 0)
    blackKnight2.setPosition({61.0f * 6, 61.0f * 0}); // Position at (6, 0)
    pieces.push_back(blackKnight1);
    pieces.push_back(blackKnight2);

    // Place bishops
    sf::Sprite whiteBishop1(whiteBishopTexture), whiteBishop2(whiteBishopTexture);
    whiteBishop1.setPosition({61.0f * 2, 61.0f * 7}); // Position at (2, 7)
    whiteBishop2.setPosition({61.0f * 5, 61.0f * 7}); // Position at (5, 7)
    pieces.push_back(whiteBishop1);
    pieces.push_back(whiteBishop2);

    sf::Sprite blackBishop1(blackBishopTexture), blackBishop2(blackBishopTexture);
    blackBishop1.setPosition({61.0f * 2, 61.0f * 0}); // Position at (2, 0)
    blackBishop2.setPosition({61.0f * 5, 61.0f * 0}); // Position at (5, 0)
    pieces.push_back(blackBishop1);
    pieces.push_back(blackBishop2);

    // Place queens
    sf::Sprite whiteQueen(whiteQueenTexture);
    whiteQueen.setPosition({61.0f * 3, 61.0f * 7}); // Position at (3, 7)
    pieces.push_back(whiteQueen);

    sf::Sprite blackQueen(blackQueenTexture);
    blackQueen.setPosition({61.0f * 3, 61.0f * 0}); // Position at (3, 0)
    pieces.push_back(blackQueen);

    // Place kings
    sf::Sprite whiteKing(whiteKingTexture);
    whiteKing.setPosition({61.0f * 4, 61.0f * 7}); // Position at (4, 7)
    pieces.push_back(whiteKing);

    sf::Sprite blackKing(blackKingTexture);
    blackKing.setPosition({61.0f * 4, 61.0f * 0}); // Position at (4, 0)
    pieces.push_back(blackKing);

    // Main loop without using event handling
    while (window.isOpen())
    {
        // Clear the window
        window.clear();

        // Draw the chessboard
        window.draw(boardSprite);

        // Draw the chess pieces
        for (const auto& piece : pieces)
        {
            window.draw(piece);
        }

        // Display the window contents
        window.display();

        // Add a small delay to prevent high CPU usage
        sf::sleep(sf::milliseconds(16)); // ~60 FPS
    }

    return 0;
}