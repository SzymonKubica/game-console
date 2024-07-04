typedef struct GameState {
        int **grid;
        int score = 0;
        int occupiedTiles = 0;
        int gridSize;
} GameState;


// Allocates a new game grid as a two-dimensional array
int **createGameGrid(int size);

GameState *initializeGameState(int gridSize);

void initializeRandomnessSeed(int seed);


