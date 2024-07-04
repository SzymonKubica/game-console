#include <stdlib.h>
#include <stdbool.h>
#include "game2048.h"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

static void mergeRow(GameState *gs, int i, int direction);
static void copy_grid(int **source, int **destination);

void initializeRandomnessSeed(int seed) { srand(seed); }

int **createGameGrid(int size)
{
        int **g = (int **)malloc(4 * sizeof(int));
        for (int i = 0; i < size; i++) {
                g[i] = (int *)malloc(size * sizeof(int));
        }
        return g;
}

void freeGameGrid(int **grid, int size)
{
        for (int i = 0; i < size; i++) {
                free(grid[i]);
        }
        free(grid);
}

GameState *initializeGameState(int size)
{
        GameState *gs = (GameState *)malloc(sizeof(GameState));
        gs->score = 0;
        gs->occupiedTiles = 0;
        gs->gridSize = size;
        gs->grid = createGameGrid(size);
        return gs;
}

void freeGameState(GameState *gs)
{
        freeGameGrid(gs->grid, gs->gridSize);
        free(gs);
}

int generateNewTileValue() { return 2 + 2 * (rand() % 2); }

int getRandomCoordinate() { return rand() % 4; }

void spawnTile(GameState *gs)
{
        bool success = false;
        while (!success) {
                int x = getRandomCoordinate();
                int y = getRandomCoordinate();

                if (gs->grid[x][y] == 0) {
                        gs->grid[x][y] = generateNewTileValue();
                        success = true;
                }
        }
        gs->occupiedTiles++;
}

bool isEmptyRow(int *row)
{
        for (int i = 0; i < 4; i++) {
                if (row[i] != 0) {
                        return false;
                }
        }
        return true;
}

void transpose(GameState *gs)
{
        int **transposed = createGameGrid(4);
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        transposed[j][i] = gs->grid[i][j];
                }
        }
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        gs->grid[i][j] = transposed[i][j];
                }
        }
        freeGameGrid(transposed, 4);
}

void merge(GameState *gs, int direction)
{
        if (direction == UP || direction == DOWN) {
                transpose(gs);
        }

        for (int i = 0; i < 4; i++) {
                mergeRow(gs, i, direction);
        }

        if (direction == UP || direction == DOWN) {
                transpose(gs);
        }
}

int getSuccessorIndex(GameState *gs, int i, int currentIndex)
{
        int succ = currentIndex + 1;
        while (succ < 4 && gs->grid[i][succ] == 0) {
                succ++;
        }
        return succ;
}

// Reverses a given row of four elements in place
void reverse(int *row)
{
        int temp = row[0];
        row[0] = row[3];
        row[3] = temp;

        temp = row[1];
        row[1] = row[2];
        row[2] = temp;
}

void mergeRow(GameState *gs, int i, int direction)
{
        int currentIndex = 0;
        int mergedRow[4] = {0, 0, 0, 0};
        int mergedNum = 0;

        if (direction == DOWN || direction == RIGHT) {
                reverse(gs->grid[i]);
        }

        currentIndex = getSuccessorIndex(gs, i, -1);

        if (currentIndex == 4) {
                // All tiles are empty.
                return;
        }

        // Now the current tile must be non-empty.
        while (currentIndex < 4) {
                int successorIndex = getSuccessorIndex(gs, i, currentIndex);
                if (successorIndex < 4 &&
                    gs->grid[i][currentIndex] == gs->grid[i][successorIndex]) {
                        // Two matching tiles found, we perform a merge.
                        int sum = gs->grid[i][currentIndex] +
                                  gs->grid[i][successorIndex];
                        gs->score += sum;
                        gs->occupiedTiles--;
                        mergedRow[mergedNum] = sum;
                        mergedNum++;
                        currentIndex = getSuccessorIndex(gs, i, successorIndex);
                } else {
                        mergedRow[mergedNum] = gs->grid[i][currentIndex];
                        mergedNum++;
                        currentIndex = successorIndex;
                }
        }

        for (int j = 0; j < 4; j++) {
                if (direction == DOWN || direction == RIGHT) {
                        gs->grid[i][3 - j] = mergedRow[j];
                } else {
                        gs->grid[i][j] = mergedRow[j];
                }
        }
}

bool theGridChangedFrom(GameState *gs, int **oldGrid);
void takeTurn(GameState *gs, int direction)
{
        int **oldGrid = createGameGrid(4);
        copy_grid(gs->grid, oldGrid);
        merge(gs, direction);

        if (theGridChangedFrom(gs, oldGrid)) {
                spawnTile(gs);
        }
        freeGameGrid(oldGrid, gs->gridSize);
}

bool isBoardFull(GameState *gs) { return gs->occupiedTiles >= 16; }

bool noMovePossible(GameState *gs);
bool isGameOver(GameState *gs) { return isBoardFull(gs) && noMovePossible(gs); }

bool theGridChangedFrom(GameState *gs, int **oldGrid)
{
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        if (gs->grid[i][j] != oldGrid[i][j]) {
                                return true;
                        }
                }
        }
        return false;
}

bool noMovePossible(GameState *gs)
{
        // Preserve the state
        int **currentState = createGameGrid(4);
        copy_grid(gs->grid, currentState);
        int currentScore = gs->score;
        int currentOccupied = gs->occupiedTiles;

        bool noMoves = true;
        for (int direction = 0; direction < 4; direction++) {
                merge(gs, direction);
                noMoves &= !theGridChangedFrom(gs, currentState);
                copy_grid(currentState, gs->grid);
        }
        freeGameGrid(currentState, gs->gridSize);

        // Restore the state
        gs->score = currentScore;
        gs->occupiedTiles = currentOccupied;
        return noMoves;
}

void copy_grid(int **source, int **destination)
{
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        destination[i][j] = source[i][j];
                }
        }
}
