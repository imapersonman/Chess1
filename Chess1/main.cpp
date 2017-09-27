//
//  main.cpp
//  Chess1
//
//  Created by Koissi Adjorlolo on 2/21/16.
//  Copyright © 2016 centuryapps. All rights reserved.
//

#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct
{
    int x, y;
} Vector2i;

typedef std::function<bool(Vector2i currentPosition,
                           Vector2i nextPosition)> ValidMoveFunction;

typedef struct
{
    int id;
    std::string name;
    ValidMoveFunction canMoveToPosition;
    SDL_Texture *texture;
} ChessPiece;

static const char *TITLE = "Chess";
static const int WINDOW_POSX = SDL_WINDOWPOS_UNDEFINED;
static const int WINDOW_POSY = SDL_WINDOWPOS_UNDEFINED;
static const int WINDOW_WIDTH = 1024;
static const int WINDOW_HEIGHT = 1024;
static const Uint32 WINDOW_FLAGS = 0;
static const Uint32 RENDERER_FLAGS = SDL_RENDERER_ACCELERATED |
                                     SDL_RENDERER_PRESENTVSYNC;
static const double MS_PER_UPDATE = 1000 / 60;

static const int BOARD_SIZE = 8;
static const int COLOR_WHITE = -1;
static const int COLOR_BLACK = 1;
static const int TEXTURE_WIDTH = 128;
static const int TEXTURE_HEIGHT = 128;

static void initPieces();
static void initBoard();
static bool pieceAtPosition(Vector2i position);
static void update();
static void updateMouseBox();
static void render(SDL_Renderer *renderer);
static void renderBoard(SDL_Renderer *renderer);
static void renderPieces(SDL_Renderer *renderer);
static void renderMouseBox(SDL_Renderer *renderer);
static int addPossiblePiece(ChessPiece piece);
static void createPiece(std::string name,
                        ValidMoveFunction function,
                        std::string texturePath);
static int idForNameAndColor(std::string name, int color);
static SDL_Texture *textureForPath(SDL_Renderer *renderer, std::string path);
static Vector2i getMouseBoxSquarePosition();
static ChessPiece getPieceAtPosition(Vector2i position);
static ChessPiece getPieceAtPosition(Vector2i position);
static void setMoveSelectedAtPosition(Vector2i position);
static void clearSelections();
static void setPieceSelectedAtPosition(Vector2i position);
static bool pieceCanMove(ChessPiece piece,
                         Vector2i position,
                         Vector2i nextPosition);
static void movePiece(ChessPiece piece,
                      Vector2i position,
                      Vector2i nextPosition);
static int getIntSign(int num);
static void switchTurns();
static bool isColorsTurn(int color);
static int getIdAtPosition(Vector2i position);
static int getColorAtPosition(Vector2i position);
static void takePiece(int color, Vector2i takePosition);
static void printGameState();
static void printTakenPieces(std::vector<int> takenPieces);
static std::string getPieceNameForId(int id);
static inline int positiveMod(int i, int n);
static bool positionIsVulnerable(int color, Vector2i position);
static bool isKingInCheck(int color);
static bool isKingInCheckMate(int color);
static void checkEndGame();
static void reset();
static bool outOfBounds(Vector2i position);
static bool canBeTakenOutOfCheck(int color);
static bool nextMoveTakesColorOutOfCheck(int color,
                                         Vector2i currentPosition,
                                         Vector2i nextPosition);

SDL_Renderer *gRenderer = nullptr;

static int GAME_BOARD[8][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};

static Vector2i selectedPiecePosition = { 0, 0 };
static Vector2i selectedMovePosition = { 0, 0 };
bool pieceSelected = false;
bool moveSelected = false;

static std::vector<ChessPiece> possiblePieces;

static SDL_Rect mouseBox = {
    0, 0,
    TEXTURE_WIDTH,
    TEXTURE_HEIGHT
};

static Uint32 mouseState = SDL_GetMouseState(&mouseBox.x, &mouseBox.y);

static int currentTurn = COLOR_WHITE;
static std::vector<int> whitesTakenPieces;
static std::vector<int> blacksTakenPieces;
static Vector2i whiteKingPosition;
static Vector2i blackKingPosition;
static int winner = 0;

int main(int argc, const char * argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Unable to init SDL" << std::endl;
        exit(1);
    }
    
    SDL_Window *window = SDL_CreateWindow(TITLE,
                                          WINDOW_POSX,
                                          WINDOW_POSY,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          WINDOW_FLAGS);
    
    int flags = IMG_INIT_PNG;
    int initted = IMG_Init(flags);
    
    if (initted != flags)
    {
        std::cout << "Unable to init SDL_image" << std::endl;
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
    
    if (window == nullptr)
    {
        std::cout << "Unable to create window" << std::endl;
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
    
    gRenderer = SDL_CreateRenderer(window, -1, RENDERER_FLAGS);
    
    if (gRenderer == nullptr)
    {
        std::cout << "Unable to create renderer" << std::endl;
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
    
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    
    initPieces();
    reset();
    
    bool running = true;
    SDL_Event event;
    
    double previous = (double)SDL_GetTicks();
    double lag = 0.0;
    
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                    
                case SDL_KEYDOWN:
                    
                    if (winner != 0 &&
                        event.key.keysym.sym == SDLK_RETURN)
                    {
                        reset();
                    }
                    
                    break;
                    
                default:
                    break;
            }
        }
        
        double current = (double)SDL_GetTicks();
        double elapsed = current - previous;
        lag += elapsed;
        previous = current;
        
        while (lag >= MS_PER_UPDATE)
        {
            if (winner == 0)
            {
                update();
            }
            lag -= elapsed;
        }
        
        render(gRenderer);
    }
    
    return 0;
}

static bool pieceAtPosition(Vector2i position)
{
    return (GAME_BOARD[position.x][position.y] != 0);
}

static void initPieces()
{
    createPiece("Pawn", [](Vector2i currentPos, Vector2i nextPos) -> bool {
        // A but hacky and unclear but it works.
        // The pieces color corresponds to the direction it can move
        // (Up is negative, down is positive).
        int allowedDirection = getColorAtPosition(currentPos);
        
        if (nextPos.y == currentPos.y + allowedDirection &&
            abs(nextPos.x - currentPos.x) == 1 &&
            pieceAtPosition(nextPos))
        {
            return true;
        }
        
        if (nextPos.x != currentPos.x)
        {
            return false;
        }
        
        if (nextPos.y == currentPos.y + allowedDirection)
        {
            return true;
        }
        
        int secondRelativeRank = positiveMod(allowedDirection, BOARD_SIZE - 1);
        
        if (nextPos.y == currentPos.y + (allowedDirection * 2) &&
            // Definitely hacky and unclear, but still, it works.
            // Example:
            //      If the piece's color is white, which corresponds to -1,
            //      and BOARD_SIZE == 8, -1 % (8 - 1) == -1 % 7 == 6.
            //      If the piece's color is black, which corresponds to 1,
            //      and BOARD_SIZE == 8, 1 % (8 - 1) == 1 % 7 == 1.
            //      In each of these cases, this trick correctly picks the
            //      second rank relative to the direction the player is facing.
            currentPos.y == secondRelativeRank)
        {
            return true;
        }
        
        // There currently is not enough information being stored to represent
        // an En Passant.  If the last move's position was stored for each color
        // it would be possible.  I'll do it later.
        
        return false;
    }, "pawn.png");
    
    createPiece("Rook", [](Vector2i currentPos, Vector2i nextPos) -> bool {
        // Castling will be implemented in the King's callback.
        if (nextPos.x == currentPos.x)
        {
            for (int y = currentPos.y + getIntSign(nextPos.y - currentPos.y);
                 y < BOARD_SIZE && y >= 0 && y != nextPos.y;
                 y += getIntSign(nextPos.y - currentPos.y))
            {
                if (GAME_BOARD[currentPos.x][y] != 0)
                {
                    return false;
                }
            }
        }
        else if (nextPos.y == currentPos.y)
        {
            for (int x = currentPos.x + getIntSign(nextPos.x - currentPos.x);
                 x < BOARD_SIZE && x >= 0 && x != nextPos.x;
                 x += getIntSign(nextPos.x - currentPos.x))
            {
                if (GAME_BOARD[x][currentPos.y] != 0)
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
        
        return true;
    }, "rook.png");
    
    createPiece("Knight", [](Vector2i currentPos, Vector2i nextPos) -> bool {
        if ((abs(nextPos.x - currentPos.x) == 2 &&
             abs(nextPos.y - currentPos.y) == 1) ||
            (abs(nextPos.x - currentPos.x) == 1 &&
             abs(nextPos.y - currentPos.y) == 2))
        {
            return true;
        }
        
        return false;
    }, "knight.png");
    
    createPiece("Bishop", [](Vector2i currentPos, Vector2i nextPos) -> bool {
        // Very similar to the rook's.
        int xDiff = nextPos.x - currentPos.x;
        int yDiff = nextPos.y - currentPos.y;
        
        if (abs(xDiff) == abs(yDiff))
        {
            int xDir = getIntSign(xDiff);
            int yDir = getIntSign(yDiff);
            
            for (int x = currentPos.x + xDir, y = currentPos.y + yDir;
                 x < BOARD_SIZE && x >= 0 &&
                 y < BOARD_SIZE && y >= 0;
                 x += xDir, y += yDir)
            {
                if (x == nextPos.x &&
                    y == nextPos.y)
                {
                    return true;
                }
                
                if (pieceAtPosition({ x, y }))
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
        
        return true;
    }, "bishop.png");
    
    createPiece("Queen", [](Vector2i currentPos, Vector2i nextPos) -> bool {
        int xDiff = nextPos.x - currentPos.x;
        int yDiff = nextPos.y - currentPos.y;
        
        if (abs(xDiff) == abs(yDiff))
        {
            int xDir = getIntSign(xDiff);
            int yDir = getIntSign(yDiff);
            
            for (int x = currentPos.x + xDir, y = currentPos.y + yDir;
                 x < BOARD_SIZE && x >= 0 &&
                 y < BOARD_SIZE && y >= 0;
                 x += xDir, y += yDir)
            {
                if (x == nextPos.x &&
                    y == nextPos.y)
                {
                    return true;
                }
                
                if (pieceAtPosition({ x, y }))
                {
                    return false;
                }
            }
        }
        else
        {
            if (nextPos.x == currentPos.x)
            {
                for (int y = currentPos.y + getIntSign(nextPos.y - currentPos.y);
                     y < BOARD_SIZE && y >= 0 && y != nextPos.y;
                     y += getIntSign(nextPos.y - currentPos.y))
                {
                    if (GAME_BOARD[currentPos.x][y] != 0)
                    {
                        return false;
                    }
                }
            }
            else if (nextPos.y == currentPos.y)
            {
                for (int x = currentPos.x + getIntSign(nextPos.x - currentPos.x);
                     x < BOARD_SIZE && x >= 0 && x != nextPos.x;
                     x += getIntSign(nextPos.x - currentPos.x))
                {
                    if (GAME_BOARD[x][currentPos.y] != 0)
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }
        
        return true;
    }, "queen.png");
    
    createPiece("King", [](Vector2i currentPos, Vector2i nextPos) -> bool {
        // Castling still needs to be implemented.
        int xDiff = nextPos.x - currentPos.x;
        int yDiff = nextPos.y - currentPos.y;
        
        if (abs(xDiff) > 1 || abs(yDiff) > 1)
        {
            return false;
        }
        
        return true;
    }, "king.png");
}

static void initBoard()
{
    // Black first row.
    int row = 0;
    GAME_BOARD[0][row] = idForNameAndColor("Rook", COLOR_BLACK);
    GAME_BOARD[1][row] = idForNameAndColor("Knight", COLOR_BLACK);
    GAME_BOARD[2][row] = idForNameAndColor("Bishop", COLOR_BLACK);
    GAME_BOARD[3][row] = idForNameAndColor("Queen", COLOR_BLACK);
    GAME_BOARD[4][row] = idForNameAndColor("King", COLOR_BLACK);
    blackKingPosition = { 4, row };
    GAME_BOARD[5][row] = idForNameAndColor("Bishop", COLOR_BLACK);
    GAME_BOARD[6][row] = idForNameAndColor("Knight", COLOR_BLACK);
    GAME_BOARD[7][row] = idForNameAndColor("Rook", COLOR_BLACK);
    // Black second row.
    row = 1;
    GAME_BOARD[0][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    GAME_BOARD[1][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    GAME_BOARD[2][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    GAME_BOARD[3][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    GAME_BOARD[4][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    GAME_BOARD[5][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    GAME_BOARD[6][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    GAME_BOARD[7][row] = idForNameAndColor("Pawn", COLOR_BLACK);
    
    // White first row.
    row = BOARD_SIZE - 1;
    GAME_BOARD[0][row] = idForNameAndColor("Rook", COLOR_WHITE);
    GAME_BOARD[1][row] = idForNameAndColor("Knight", COLOR_WHITE);
    GAME_BOARD[2][row] = idForNameAndColor("Bishop", COLOR_WHITE);
    GAME_BOARD[3][row] = idForNameAndColor("Queen", COLOR_WHITE);
    GAME_BOARD[4][row] = idForNameAndColor("King", COLOR_WHITE);
    whiteKingPosition = { 4, row };
    GAME_BOARD[5][row] = idForNameAndColor("Bishop", COLOR_WHITE);
    GAME_BOARD[6][row] = idForNameAndColor("Knight", COLOR_WHITE);
    GAME_BOARD[7][row] = idForNameAndColor("Rook", COLOR_WHITE);
    // White second row.
    row = BOARD_SIZE - 2;
    GAME_BOARD[0][row] = idForNameAndColor("Pawn", COLOR_WHITE);
    GAME_BOARD[1][row] = idForNameAndColor("Pawn", COLOR_WHITE);
    GAME_BOARD[2][row] = idForNameAndColor("Pawn", COLOR_WHITE);
    GAME_BOARD[3][row] = idForNameAndColor("Pawn", COLOR_WHITE);
    GAME_BOARD[4][row] = idForNameAndColor("Pawn", COLOR_WHITE);
    GAME_BOARD[5][row] = idForNameAndColor("Pawn", COLOR_WHITE);
    GAME_BOARD[6][row] = idForNameAndColor("Pawn", COLOR_WHITE);
    GAME_BOARD[7][row] = idForNameAndColor("Pawn", COLOR_WHITE);
}

static void update()
{
    if (winner == 0)
    {
        updateMouseBox();
    }
    
    checkEndGame();
}

static void updateMouseBox()
{
    bool lastFramePressed = mouseState & SDL_BUTTON_LMASK;
    
    mouseState = SDL_GetMouseState(&mouseBox.x, &mouseBox.y);
    mouseBox.x /= TEXTURE_WIDTH;
    mouseBox.x *= TEXTURE_WIDTH;
    mouseBox.y /= TEXTURE_HEIGHT;
    mouseBox.y *= TEXTURE_HEIGHT;
    
    Vector2i mouseBoxSquarePosition = getMouseBoxSquarePosition();
    
    if (mouseState & SDL_BUTTON_LMASK &&
        !lastFramePressed)
    {
        if (pieceAtPosition(mouseBoxSquarePosition) &&
            !pieceSelected &&
            isColorsTurn(getColorAtPosition(mouseBoxSquarePosition)))
        {
            setPieceSelectedAtPosition(mouseBoxSquarePosition);
        }
        else if (pieceSelected)
        {
            setMoveSelectedAtPosition(mouseBoxSquarePosition);
            ChessPiece selected = getPieceAtPosition(selectedPiecePosition);
            
            if (!isKingInCheck(currentTurn))
            {
                if (selectedPiecePosition.x != selectedMovePosition.x ||
                    selectedPiecePosition.y != selectedMovePosition.y)
                {
                    if (pieceCanMove(selected,
                                     selectedPiecePosition,
                                     selectedMovePosition))
                    {
                        movePiece(selected,
                                  selectedPiecePosition,
                                  selectedMovePosition);
                        switchTurns();
                        
                        clearSelections();
                    }
                }
            }
            else
            {
                if (isKingInCheckMate(currentTurn))
                {
                    if (currentTurn == COLOR_WHITE)
                    {
                        winner = COLOR_BLACK;
                    }
                    else
                    {
                        winner = COLOR_WHITE;
                    }
                }
                
                if (nextMoveTakesColorOutOfCheck(currentTurn,
                                                 selectedPiecePosition,
                                                 selectedMovePosition))
                {
                    clearSelections();
                }
            }
            
            if (isKingInCheck(currentTurn) &&
                isKingInCheckMate(currentTurn))
            {
                if (currentTurn == COLOR_WHITE)
                {
                    winner = COLOR_BLACK;
                }
                else
                {
                    winner = COLOR_WHITE;
                }
            }
        }
    }
    
    if (mouseState & SDL_BUTTON_RMASK)
    {
        if (pieceAtPosition(mouseBoxSquarePosition) &&
            pieceSelected)
        {
            clearSelections();
        }
    }
}

static void render(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    
    renderBoard(renderer);
    renderPieces(renderer);
    renderMouseBox(renderer);
    
    SDL_RenderPresent(renderer);
}

static void renderMouseBox(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100);
    SDL_RenderFillRect(renderer, &mouseBox);
}

static void renderBoard(SDL_Renderer *renderer)
{
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            int rowType = ((row % 2) + (col % 2)) % 2;
            
            switch (rowType)
            {
                case 0:
                    SDL_SetRenderDrawColor(renderer, 235, 207, 143, 255);
                    break;
                    
                case 1:
                    SDL_SetRenderDrawColor(renderer, 196, 152, 49, 255);
                    break;
            }
            
            int squareWidth = WINDOW_WIDTH / BOARD_SIZE;
            
            Vector2i rectPos = {
                squareWidth * row,
                squareWidth * col
            };
            
            SDL_Rect squareRect = {
                rectPos.x,
                rectPos.y,
                squareWidth,
                squareWidth
            };
            
            SDL_RenderFillRect(renderer, &squareRect);
        }
    }
}

static void renderPieces(SDL_Renderer *renderer)
{
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            int pieceId = GAME_BOARD[row][col];
            ChessPiece piece = possiblePieces[abs(pieceId)];
            
            if (pieceId > 0)
            {
                SDL_SetTextureColorMod(piece.texture, 0, 0, 0);
            }
            else
            {
                SDL_SetTextureColorMod(piece.texture, 255, 255, 255);
            }
            
            int squareWidth = WINDOW_WIDTH / BOARD_SIZE;
            
            Vector2i rectPos = {
                squareWidth * row,
                squareWidth * col
            };
            
            SDL_Rect squareRect = {
                rectPos.x,
                rectPos.y,
                squareWidth,
                squareWidth
            };
            
            SDL_Rect srcRect = {
                0, 0,
                TEXTURE_WIDTH,
                TEXTURE_HEIGHT
            };
            
            SDL_RenderCopy(renderer, piece.texture, &srcRect, &squareRect);
            
            Vector2i renderingPosition = { row, col };
            
            if (selectedPiecePosition.x == renderingPosition.x &&
                selectedPiecePosition.y == renderingPosition.y &&
                pieceSelected)
            {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
                SDL_RenderFillRect(renderer, &squareRect);
            }
            
            if (selectedMovePosition.x == renderingPosition.x &&
                selectedMovePosition.y == renderingPosition.y &&
                moveSelected)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
                SDL_RenderFillRect(renderer, &squareRect);
            }
        }
    }
}

static int addPossiblePiece(ChessPiece piece)
{
    if (possiblePieces.size() == 0)
    {
        ChessPiece nullPiece = {
            0,
            "null",
            [](Vector2i currentPos, Vector2i nextPos) ->bool {
                return false;
            }
        };
        
        possiblePieces.push_back(nullPiece);
    }
    
    piece.id = (int)possiblePieces.size();
    possiblePieces.push_back(piece);
    
    return piece.id;
}

static void createPiece(std::string name,
                        ValidMoveFunction function,
                        std::string texturePath)
{
    addPossiblePiece({
        0,
        name,
        function,
        textureForPath(gRenderer, texturePath)
    });
}

static int idForNameAndColor(std::string name, int color)
{
    for (int pieceIndex = 0;
         pieceIndex < possiblePieces.size();
         pieceIndex++)
    {
        if (name == possiblePieces[pieceIndex].name)
        {
            return pieceIndex * color;
        }
    }
    
    return INT32_MAX;
}

static SDL_Texture *textureForPath(SDL_Renderer *renderer, std::string path)
{
    std::string prepath = "Resources/Images/";
    SDL_RWops *textureRWops = SDL_RWFromFile((prepath + path).c_str(), "rb");
    
    if (textureRWops == nullptr)
    {
        std::cout << "Unable to load " << path << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
    
    SDL_Surface *textureSurface = IMG_LoadPNG_RW(textureRWops);
    
    if (textureSurface == nullptr)
    {
        std::cout << "Unable to load " << path << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, textureSurface);
    
    if (texture == nullptr)
    {
        std::cout << "Unable to load " << path << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
    
    return texture;
}

static Vector2i getMouseBoxSquarePosition()
{
    return {
        mouseBox.x / TEXTURE_WIDTH,
        mouseBox.y / TEXTURE_HEIGHT
    };
}

static ChessPiece getPieceAtPosition(Vector2i position)
{
    int pieceId = GAME_BOARD[position.x][position.y];
    ChessPiece piece = possiblePieces[abs(pieceId)];
    
    return piece;
}

static void setPieceSelectedAtPosition(Vector2i position)
{
    selectedPiecePosition = position;
    pieceSelected = true;
}

static void setMoveSelectedAtPosition(Vector2i position)
{
    selectedMovePosition = position;
    moveSelected = true;
}

static void clearSelections()
{
    pieceSelected = false;
    moveSelected = false;
}

static bool pieceCanMove(ChessPiece piece, Vector2i position, Vector2i nextPosition)
{
    int currentPieceId = GAME_BOARD[position.x][position.y];
    int nextPieceId = GAME_BOARD[nextPosition.x][nextPosition.y];
    
    return (possiblePieces[piece.id].canMoveToPosition(position, nextPosition) &&
            (getIntSign(currentPieceId) != getIntSign(nextPieceId) ||
            nextPieceId == 0));
}

static void movePiece(ChessPiece piece, Vector2i position, Vector2i nextPosition)
{
    int pieceId = GAME_BOARD[position.x][position.y];
    GAME_BOARD[position.x][position.y] = 0;
    
    // In this case the color doesn't actually matter.
    if (abs(pieceId) == idForNameAndColor("King",
                                         COLOR_BLACK))
    {
        if (currentTurn == COLOR_WHITE)
        {
            whiteKingPosition = selectedMovePosition;
        }
        else
        {
            blackKingPosition = selectedMovePosition;
        }
    }
    
    if (GAME_BOARD[nextPosition.x][nextPosition.y] != 0)
    {
        // Give piece to current player.
        // ...
        takePiece(currentTurn, nextPosition);
    }
    
    GAME_BOARD[nextPosition.x][nextPosition.y] = pieceId;
}

static int getIntSign(int num)
{
    if (num < 0)
    {
        return -1;
    }
    
    return 1;
}

static void switchTurns()
{
    currentTurn = (currentTurn == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
    printGameState();
}

static bool isColorsTurn(int color)
{
    return (currentTurn == color);
}

static int getIdAtPosition(Vector2i position)
{
    return GAME_BOARD[position.x][position.y];
}

static int getColorAtPosition(Vector2i position)
{
    return getIntSign(getIdAtPosition(position));
}

static void takePiece(int color, Vector2i takePosition)
{
    if (isColorsTurn(color))
    {
        int takeId = GAME_BOARD[takePosition.x][takePosition.y];
        GAME_BOARD[takePosition.x][takePosition.y] = 0;
        
        switch (color)
        {
            case COLOR_WHITE:
                whitesTakenPieces.push_back(takeId);
                break;
                
            case COLOR_BLACK:
                blacksTakenPieces.push_back(takeId);
                break;
        }
    }
}

static void printGameState()
{
    std::string turnString;
    
    if (currentTurn == COLOR_WHITE)
    {
        turnString = "White";
    }
    else
    {
        turnString = "Black";
    }
    
    std::cout << "Current Turn: " << turnString << std::endl;
    
    std::cout << "White's taken pieces:" << std::endl;
    printTakenPieces(whitesTakenPieces);
    std::cout << "Black's taken pieces:" << std::endl;
    if (blacksTakenPieces.size() > 0)
    {
        printTakenPieces(blacksTakenPieces);
    }
    std::cout << std::endl;
}

static void printTakenPieces(std::vector<int> takenPieces)
{
    for (int pieceIndex = 0;
         pieceIndex < takenPieces.size();
         pieceIndex++)
    {
        std::string pieceName = getPieceNameForId(abs(takenPieces[pieceIndex]));
        std::cout << " - " << pieceName << std::endl;
    }
}

static std::string getPieceNameForId(int id)
{
    return possiblePieces[id].name;
}

// Code is from
// http://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
static inline int positiveMod(int i, int n)
{
    return (i % n + n) % n;
}

static bool positionIsVulnerable(int color, Vector2i position)
{
    if (position.x < 0 || position.x >= BOARD_SIZE ||
        position.y < 0 || position.y >= BOARD_SIZE)
    {
        return false;
    }
    
    int pieceId = getIdAtPosition(position);
    
    // Let's brute force it.  There are only 64 spaces to try each turn so this
    // isn't too bad.
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            Vector2i currentPosition = { row, col };
            int currentId = getIdAtPosition(currentPosition);
            
            if (currentId == 0 ||
                getIntSign(currentId) == color ||
                currentId == pieceId)
            {
                continue;
            }
            
            int absId = abs(currentId);
            
            if (possiblePieces[absId].canMoveToPosition(currentPosition,
                                                        position))
            {
                return true;
            }
        }
    }
    
    return false;
}

static bool isKingInCheck(int color)
{
    Vector2i kingPosition;
    
    if (color == COLOR_WHITE)
    {
        kingPosition = whiteKingPosition;
    }
    else
    {
        kingPosition = blackKingPosition;
    }
    
    return positionIsVulnerable(color, kingPosition);
}

static bool isKingInCheckMate(int color)
{
    Vector2i kPos;
    
    if (color == COLOR_WHITE)
    {
        kPos = whiteKingPosition;
    }
    else
    {
        kPos = blackKingPosition;
    }
    
    int kId = getIdAtPosition(kPos);
    GAME_BOARD[kPos.x][kPos.y] = 0;
    
    bool ans = ((positionIsVulnerable(color, kPos) ||
                 pieceAtPosition(kPos) ||
                 outOfBounds(kPos)) &&
                (positionIsVulnerable(color, { kPos.x, kPos.y + 1 }) ||
                 pieceAtPosition({ kPos.x, kPos.y + 1 }) ||
                 outOfBounds({ kPos.x, kPos.y + 1 })) &&
                (positionIsVulnerable(color, { kPos.x, kPos.y - 1 }) ||
                 pieceAtPosition({ kPos.x, kPos.y - 1 }) ||
                 outOfBounds({ kPos.x, kPos.y - 1 })) &&
                (positionIsVulnerable(color, { kPos.x - 1, kPos.y }) ||
                 pieceAtPosition({ kPos.x - 1, kPos.y }) ||
                 outOfBounds({ kPos.x - 1, kPos.y })) &&
                (positionIsVulnerable(color, { kPos.x + 1, kPos.y }) ||
                 pieceAtPosition({ kPos.x + 1, kPos.y }) ||
                 outOfBounds({ kPos.x + 1, kPos.y })) &&
                (positionIsVulnerable(color, { kPos.x + 1, kPos.y + 1 }) ||
                 pieceAtPosition({ kPos.x + 1, kPos.y + 1 }) ||
                 outOfBounds({ kPos.x + 1, kPos.y + 1 })) &&
                (positionIsVulnerable(color, { kPos.x - 1, kPos.y - 1 }) ||
                 pieceAtPosition({ kPos.x - 1, kPos.y - 1 }) ||
                 outOfBounds({ kPos.x - 1, kPos.y - 1 })) &&
                (positionIsVulnerable(color, { kPos.x + 1, kPos.y - 1 }) ||
                 pieceAtPosition({ kPos.x + 1, kPos.y - 1 }) ||
                 outOfBounds({ kPos.x + 1, kPos.y - 1 })) &&
                (positionIsVulnerable(color, { kPos.x - 1, kPos.y + 1 }) ||
                 pieceAtPosition({ kPos.x - 1, kPos.y + 1 }) ||
                 outOfBounds({ kPos.x - 1, kPos.y + 1 })) &&
                !canBeTakenOutOfCheck(color));
    
    GAME_BOARD[kPos.x][kPos.y] = kId;
    
    // 64 * 9 == 576 loops per call.
    return ans;
}

static void checkEndGame()
{
    if (winner != 0)
    {
        if (winner == COLOR_WHITE)
        {
            std::cout << "Game over, white wins" << std::endl;
        }
        else
        {
            std::cout << "Game over, black wins" << std::endl;
        }
        
        std::cout << "Press enter to restart game" << std::endl;
    }
}

static void reset()
{
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            GAME_BOARD[row][col] = 0;
        }
    }
    
    winner = 0;
    clearSelections();
    currentTurn = COLOR_WHITE;
    initBoard();
}

static bool outOfBounds(Vector2i position)
{
    return (position.x < 0 || position.x >= BOARD_SIZE ||
            position.y < 0 || position.y >= BOARD_SIZE);
}

static bool canBeTakenOutOfCheck(int color)
{
    Vector2i kPos;
    
    if (color == COLOR_WHITE)
    {
        kPos = whiteKingPosition;
    }
    else
    {
        kPos = blackKingPosition;
    }
    
    if (kPos.x < 0 || kPos.x >= BOARD_SIZE ||
        kPos.y < 0 || kPos.y >= BOARD_SIZE)
    {
        return false;
    }
    
    // Let's brute force it.  There are only 64 spaces to try each turn so this
    // isn't too bad.
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            Vector2i currentPosition = { row, col };
            int currentId = getIdAtPosition(currentPosition);
            
            if (currentId == 0 ||
                getIntSign(currentId) == color)
            {
                continue;
            }
            
            // NEED TO CHECK FOR BLOCKS GOING TO BED
            
            int absId = abs(currentId);
            
            if (possiblePieces[absId].canMoveToPosition(currentPosition,
                                                        kPos))
            {
                if (positionIsVulnerable(-color, currentPosition))
                {
                    if (nextMoveTakesColorOutOfCheck(color, kPos, currentPosition))
                    {
                        return true;
                    }
                }
                
                return false;
            }
        }
    }
    
    return true;
}

static bool nextMoveTakesColorOutOfCheck(int color,
                                         Vector2i currentPosition,
                                         Vector2i nextPosition)
{
    ChessPiece selected = getPieceAtPosition(currentPosition);
    
    if (pieceCanMove(selected,
                     currentPosition,
                     nextPosition))
    {
        movePiece(selected,
                  currentPosition,
                  nextPosition);
        
        if (isKingInCheck(color))
        {
            movePiece(selected, nextPosition, currentPosition);
            return false;
        }
        
        switchTurns();
    }
    else
    {
        return false;
    }

    return true;
}
