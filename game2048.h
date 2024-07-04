typedef struct GameState {
        int **grid;
        int score;
        int occupiedTiles;
        int gridSize;
} GameState;


// Allocates a new game grid as a two-dimensional array
int **createGameGrid(int size);

GameState *initializeGameState(int gridSize);

void initializeRandomnessSeed(int seed);

int generateNewTileValue();

int getRandomCoordinate();

bool isGameOver(GameState *gs);
void takeTurn(GameState *gs, int direction);
void spawnTile(GameState *gs);

