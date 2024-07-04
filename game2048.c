#include <stdlib.h>
#include <stdbool.h>
#include "game2048.h"

void initializeRandomnessSeed(int seed)
{
    srand(seed);
}

int **createGameGrid(int size)
{
        int **g = (int **)malloc(4 * sizeof(int));
        for (int i = 0; i < 4; i++) {
                g[i] = (int *)malloc(4 * sizeof(int));
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

GameState *createGameState(int size)
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

int generateNewTileValue() { return 2 + 2 * (int)random(2); }

int getRandomCoordinate() { return random(100) % 4; }

void spawnTile()
{
        bool success = false;
        while (!success) {
                int x = getRandomCoordinate();
                int y = getRandomCoordinate();

                if (grid[x][y] == 0) {
                        grid[x][y] = generateNewTileValue();
                        success = true;
                }
        }
        occupiedTiles++;
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

void transpose()
{
        int **transposed = allocateGrid();
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        transposed[j][i] = grid[i][j];
                }
        }
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        grid[i][j] = transposed[i][j];
                }
        }
        freeGrid(transposed);
}

void merge(int direction)
{
        if (direction == UP || direction == DOWN) {
                transpose();
        }

        for (int i = 0; i < 4; i++) {
                mergeRow(i, direction);
        }

        if (direction == UP || direction == DOWN) {
                transpose();
        }
}

int getSuccessorIndex(int i, int currentIndex)
{
        int succ = currentIndex + 1;
        while (succ < 4 && grid[i][succ] == 0) {
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

void mergeRow(int i, int direction)
{
        int currentIndex = 0;
        int mergedRow[4] = {0, 0, 0, 0};
        int mergedNum = 0;

        if (direction == DOWN || direction == RIGHT) {
                reverse(grid[i]);
        }

        currentIndex = getSuccessorIndex(i, -1);

        if (currentIndex == 4) {
                // All tiles are empty.
                return;
        }

        // Now the current tile must be non-empty.
        while (currentIndex < 4) {
                int successorIndex = getSuccessorIndex(i, currentIndex);
                if (successorIndex < 4 &&
                    grid[i][currentIndex] == grid[i][successorIndex]) {
                        // Two matching tiles found, we perform a merge.
                        int sum =
                            grid[i][currentIndex] + grid[i][successorIndex];
                        score += sum;
                        occupiedTiles--;
                        mergedRow[mergedNum] = sum;
                        mergedNum++;
                        currentIndex = getSuccessorIndex(i, successorIndex);
                } else {
                        mergedRow[mergedNum] = grid[i][currentIndex];
                        mergedNum++;
                        currentIndex = successorIndex;
                }
        }

        for (int j = 0; j < 4; j++) {
                if (direction == DOWN || direction == RIGHT) {
                        grid[i][3 - j] = mergedRow[j];
                } else {
                        grid[i][j] = mergedRow[j];
                }
        }
}

void freeGrid(int **g)
{
        for (int i = 0; i < 4; i++) {
                free(g[i]);
        }
        free(g);
}
void takeTurn(int direction)
{
        int **oldGrid = allocateGrid();
        copy(grid, oldGrid);
        merge(direction);

        if (theGridChangedFrom(oldGrid)) {
                spawnTile();
        }
        freeGrid(oldGrid);
}

bool isBoardFull() { return occupiedTiles >= 16; }

bool isGameOver() { return isBoardFull() && noMovePossible(); }

bool theGridChangedFrom(int **oldGrid)
{
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        if (grid[i][j] != oldGrid[i][j]) {
                                return true;
                        }
                }
        }
        return false;
}

bool noMovePossible()
{
        // Preserve the state
        int **currentState = allocateGrid();
        copy(grid, currentState);
        int currentScore = score;
        int currentOccupied = occupiedTiles;

        boolean noMoves = true;
        for (int direction = 0; direction < 4; direction++) {
                merge(direction);
                noMoves &= !theGridChangedFrom(currentState);
                copy(currentState, grid);
        }
        freeGrid(currentState);

        // Restore the state
        score = currentScore;
        occupiedTiles = currentOccupied;
        return noMoves;
}

void copy(int **source, int **destination)
{
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        destination[i][j] = source[i][j];
                }
        }
}

int **allocateGrid()
{
        int **g = (int **)malloc(4 * sizeof(int));
        for (int i = 0; i < 4; i++) {
                g[i] = (int *)malloc(4 * sizeof(int));
        }
        return g;
}
