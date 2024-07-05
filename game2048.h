typedef struct GameState {
        int **grid;
        int score;
        int occupied_tiles;
        int grid_size;
} GameState;

int **createGameGrid(int size);
GameState *initializeGameState(int gridSize);
void initializeRandomnessSeed(int seed);
bool isGameOver(GameState *gs);
void takeTurn(GameState *gs, int direction);
void spawnTile(GameState *gs);
