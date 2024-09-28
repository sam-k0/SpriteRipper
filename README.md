# SpriteRipper

A tool to extract spritesheets from GameMaker Studio 1.4 binaries.

This works on `.win` and `.exe` files (aka bytecode and compiled games).
Untested on GameMaker Studio 2+ games.

## Usage (Windows)

1. Drag-and-drop the `.win` or `.exe` file onto the program.
 2. The spritesheets will be extracted to the same directory as the input file.

> [!NOTE]  
> Alternatively, you can run the program from the command line and pass the path to the file as an argument.


## Usage (Linux)
Run the program from the command line and pass the path to the file as an argument.


## Building

### Windows
Through Visual Studio:
- Open the vcxproj file in Visual Studio and click build. Should be fine

### Linux
1. `g++ main.cpp -o SpriteRipper`
2. `chmod +x SpriteRipper` <-- add the execute permission, may be unnecessary
3. `./SpriteRipper <path to .win or .exe file>`



