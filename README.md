# Chess Game

A complete Chess Game built using **C++** and **Raylib** library for graphics. This project offers a rich, interactive, and visually appealing platform to play chess locally.

---

## 🚀 Project Overview

This Chess Game implements all official chess rules, including special moves like **en passant** and **pawn promotion**, with a focus on providing a smooth user experience. Players can enjoy local multiplayer, a responsive interface, and sound effects for a more immersive experience. The game features both **board rotation**, **fullscreen mode**, and the ability to **display captured pieces**.

---

## 💡 Features

### Core Features:
- **Full Chess Rules**: Includes all standard chess moves, as well as special rules like pawn promotion, en passant, check, checkmate, and stalemate.
- **Local Multiplayer**: Two players can play on the same machine.
- **Pawn Promotion**: Automatically promotes pawns based on player choice.
- **En Passant**: Implemented according to chess rules.
- **Check/Checkmate/Stalemate**: Detects check, checkmate, and stalemate conditions.
- **Enhanced User Interface**: Includes menus for player names, a quit button, and captured pieces display.
- **Sound Effects**: Adds sound feedback on moves for a better user experience.
- **Fullscreen and Rotation**: Supports rotating the chessboard and fullscreen mode for a more realistic experience.

---

## 🛠️ Tools and Technologies

- **Programming Language**: C++
- **Graphics Library**: Raylib
- **Platform**: Desktop (Windows)
- **Version Control**: GitHub
- **Communication**: WhatsApp group.

---

## 📜 Project Team

- **Shehryar Rafiq** (Group Leader): Setup of Raylib environment, board drawing, piece movement, board rotation, fullscreen mode.
- **Faizan Basheer**: Pawn promotion, en passant move logic, check detection, sound effects integration.
- **Syed Muhammad Sufyan**: Checkmate and stalemate detection, enhanced interface, player name input, captured pieces display.

---

## 💻 Installation

### Windows
1. **Download MinGW-W64** (if not already installed):
   - Go to https://github.com/skeeto/w64devkit/releases or use the version that comes with Raylib installer.

2. **Build Instructions for MinGW-W64**:
   - Open **Git Bash** or **Command Prompt**.
   - Navigate to the project folder:
     ```bash
     cd "C:\path\to\Chess-GUI-main"
     ```
   - Run the following command to build the project:
     ```bash
     make
     ```

---

### Linux
1. **Install Dependencies**:
   - Ensure **premake5** is installed from https://premake.github.io/.

2. **Build Instructions**:
   - Open a terminal and navigate to the project folder:
     ```bash
     cd "path/to/Chess-GUI-main"
     ```
   - Run the following commands:
     ```bash
     ./premake5 gmake2
     make
     ```

---

### MacOS
1. **Install Dependencies**:
   - Use Homebrew to install **premake5**:
     ```bash
     brew install premake
     ```

2. **Build Instructions**:
   - Open a terminal and navigate to the project folder:
     ```bash
     cd "path/to/Chess-GUI-main"
     ```
   - Run:
     ```bash
     ./premake5.osx gmake2
     make
     ```

---

## 🎮 Usage

- **Start a New Game**: Launch the built executable to start the game.
- **Multiplayer**: Use mouse or keyboard for moving pieces.
- **Sound Effects**: Each move has sound feedback.
- **Board Rotation**: Rotate the board for a different perspective.
- **Captured Pieces**: See captured pieces for both players.
- **Check/Checkmate**: The game detects check, checkmate, and stalemate.

---

## 🔧 Future Work & Improvements

- **AI Opponent**: Add an AI opponent to enable single-player mode.
- **Save/Load**: Implement save and load functionality to resume games.
- **Online Multiplayer**: Add support for online multiplayer matches.

---

## 📚 Conclusion

We successfully developed a **full chess game** with all essential features, including:
- Correct chess logic
- Special rules like **pawn promotion** and **en passant**
- **Checkmate** and **stalemate** detection
- Enhanced user interface, **captured pieces display**, and **sound effects**
- **Fullscreen mode** and **board rotation**

### Key Features Recap:
- Full Chess Rule Implementation
- Smooth Movement and Sound Feedback
- Player Name Input Menu
- Captured Pieces Display
- Checkmate and Stalemate Detection
- Fullscreen Mode and Board Rotation

---

## 💬 Acknowledgments

We'd like to thank our instructors for their support, and Raylib for providing a great platform for game development.

---
