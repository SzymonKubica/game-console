#ifndef GAME2048_H
#define GAME2048_H

typedef struct GameState {
        int **grid;
        int **old_grid;
        int score;
        int occupied_tiles;
        int grid_size;
        int target_max_tile;
} GameState;

GameState *initializeGameState(int gridSize);
void initializeRandomnessSeed(int seed);
bool isGameOver(GameState *gs);
bool isGameFinished(GameState *gs);
void takeTurn(GameState *gs, int direction);

#endif
