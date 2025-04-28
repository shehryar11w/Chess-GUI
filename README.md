# Chess GUI - Game

A simple and interactive Chess Game using **Raylib** as a graphics library for the GUI. This project provides an easy-to-use interface for playing chess and is cross-platform, supporting Windows, Linux, and MacOS.

## Features
- **Chess Game Logic**: Implements the basic rules of Chess, including piece movements, checks, and checkmates.
- **GUI Interface**: Built with Raylib for rendering the chessboard and pieces. A simple but effective interface to interact with the game.
- **Game Management**: Ability to start a new game, undo moves, and view game status (check/checkmate).
- **AI Integration** (Optional): Includes a basic AI for playing against the computer (can be turned off).
- **Multiplayer**: Allows two players to play locally on the same machine.

## Supported Platforms
- **Windows**
- **Linux**
- **MacOS**

## Installation

### Windows
1. **Download MinGW-W64** (if not already installed):
   - Go to [W64devkit](https://github.com/skeeto/w64devkit/releases) or use the version that comes with Raylib installer.
   
2. **Build Instructions for MinGW-W64**:
   - Open **Git Bash** or **Command Prompt**.
   - Navigate to the project folder:
     ```bash
     cd "C:\path\to\Chess-GUI-main"
     ```
   - Run the following to build the project:
     ```bash
     make
     ```

3. **Build Instructions for Microsoft Visual Studio**:
   - Run `build-VisualStudio2022.bat`.
   - Open the generated `.sln` file and build the project in Visual Studio.

### Linux
1. **Install Dependencies**:
   - Ensure that **premake5** is installed. You can get it from [premake.github.io](https://premake.github.io/).

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

### MacOS
1. **Install Dependencies**:
   - Ensure that **premake5** is installed. You can install it using Homebrew:
     ```bash
     brew install premake
     ```

2. **Build Instructions**:
   - Open a terminal and navigate to the project folder:
     ```bash
     cd "path/to/Chess-GUI-main"
     ```
   - Run the following commands:
     ```bash
     ./premake5.osx gmake2
     make
     ```

## Usage

- **Start a New Game**: Once the game is built, execute the binary to start a new game. A chessboard will appear, and you can interact with it to move pieces.
- **Multiplayer**: Simply use the mouse or keyboard to move pieces.
- **AI Play** (if enabled): Play against the AI by moving pieces for the player side, and the AI will respond with its move.
- **Undo Move**: Press the "Undo" button to revert to the previous game state.
- **Game Status**: The game will display whether the current game is in check or checkmate.

## Changing to C++
If you want to switch from C to C++, simply rename `src/main.c` to `src/main.cpp` and perform a clean build.

## Customizing the Chess Game
- **Replace Main Code**: You can replace `src/main.c` with your own code while keeping the existing setup, and then build the project again.
- **Add Custom Resources**: The project includes a `resources` folder for storing assets like images and sounds. You can replace the default assets with your own if needed.

## Building for Different OpenGL Versions
If you need to build for a different OpenGL version than the default **OpenGL 3.3**, modify the **premake** command line or use the following options:

- For OpenGL 1.1:
  ```bash
  --graphics=opengl11
