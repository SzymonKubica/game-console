#include <stdlib.h>
#include <stdbool.h>
#include "game2048.h"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

static void copy_grid(int **source, int **destination);

void initializeRandomnessSeed(int seed) { srand(seed); }

int **createGameGrid(int size)
{
        int **g = (int **)malloc(4 * sizeof(int));
        for (int i = 0; i < size; i++) {
                g[i] = (int *)calloc(size, sizeof(int));
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
        gs->occupied_tiles = 0;
        gs->grid_size = size;
        gs->grid = createGameGrid(size);
        return gs;
}

void freeGameState(GameState *gs)
{
        freeGameGrid(gs->grid, gs->grid_size);
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
        gs->occupied_tiles++;
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
        for (int i = 0; i < gs->grid_size; i++) {
                for (int j = i; j < gs->grid_size; j++) {
                        int temp = gs->grid[i][j];
                        gs->grid[i][j] = gs->grid[j][i];
                        gs->grid[j][i] = temp;
                }
        }
}

/*******************************************************************************
  Tile Merging Logic
*******************************************************************************/

/* Helper functions for tile merging */

/// Merges the i-th row of tiles in the given direction (left/right).
static void mergeRow(GameState *gs, int i, int direction);

/// Reverses a given row of `row_size` elements in place.
void reverse(int *row, int row_size);

/// Returns the index of the next non-empty tile after the `current_index` in
/// the i-th row in of the grid.
int getSuccessorIndex(GameState *gs, int i, int current_index);

// We only implement merging tiles left or right (row-wise), in order to merge
// the tiles in a vertical direction we first transpose the grid, merge and then
// transpose back.
void merge(GameState *gs, int direction)
{
        if (direction == UP || direction == DOWN) {
                transpose(gs);
        }

        for (int i = 0; i < gs->grid_size; i++) {
                mergeRow(gs, i, direction);
        }

        if (direction == UP || direction == DOWN) {
                transpose(gs);
        }
}

void mergeRow(GameState *gs, int i, int direction)
{
        int current_index = 0;
        int *merged_row = (int *)calloc(gs->grid_size, sizeof(int));
        int merged_num = 0;

        // We always merge to the left (or up if we have previously transposed
        // the grid), in other cases we reverse the row, merge and reverse back.
        if (direction == DOWN || direction == RIGHT) {
                reverse(gs->grid[i], gs->grid_size);
        }

        current_index = getSuccessorIndex(gs, i, -1);

        if (current_index == gs->grid_size) {
                // All tiles are empty.
                return;
        }

        // Now the current tile must be non-empty.
        while (current_index < gs->grid_size) {
                int successorIndex = getSuccessorIndex(gs, i, current_index);
                if (successorIndex < gs->grid_size &&
                    gs->grid[i][current_index] == gs->grid[i][successorIndex]) {
                        // Two matching tiles found, we perform a merge.
                        int sum = gs->grid[i][current_index] +
                                  gs->grid[i][successorIndex];
                        gs->score += sum;
                        gs->occupied_tiles--;
                        merged_row[merged_num] = sum;
                        merged_num++;
                        current_index =
                            getSuccessorIndex(gs, i, successorIndex);
                } else {
                        merged_row[merged_num] = gs->grid[i][current_index];
                        merged_num++;
                        current_index = successorIndex;
                }
        }

        for (int j = 0; j < gs->grid_size; j++) {
                if (direction == DOWN || direction == RIGHT) {
                        gs->grid[i][gs->grid_size - 1 - j] = merged_row[j];
                } else {
                        gs->grid[i][j] = merged_row[j];
                }
        }
}

// Reverses a given row of four elements in place
void reverse(int *row, int row_size)
{
        for (int i = 0; i < row_size / 2; i++) {
                int temp = row[i];
                row[i] = row[row_size - i - 1];
                row[row_size - i - 1] = temp;
        }
}

int getSuccessorIndex(GameState *gs, int i, int current_index)
{
        int succ = current_index + 1;
        while (succ < gs->grid_size && gs->grid[i][succ] == 0) {
                succ++;
        }
        return succ;
}

/*******************************************************************************
  Game Loop Logic
*******************************************************************************/

bool theGridChangedFrom(GameState *gs, int **oldGrid);
void takeTurn(GameState *gs, int direction)
{
        int **oldGrid = createGameGrid(4);
        copy_grid(gs->grid, oldGrid);
        merge(gs, direction);

        if (theGridChangedFrom(gs, oldGrid)) {
                spawnTile(gs);
        }
        freeGameGrid(oldGrid, gs->grid_size);
}

bool isBoardFull(GameState *gs) { return gs->occupied_tiles >= 16; }

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
        int currentOccupied = gs->occupied_tiles;

        bool noMoves = true;
        for (int direction = 0; direction < 4; direction++) {
                merge(gs, direction);
                noMoves &= !theGridChangedFrom(gs, currentState);
                copy_grid(currentState, gs->grid);
        }
        freeGameGrid(currentState, gs->grid_size);

        // Restore the state
        gs->score = currentScore;
        gs->occupied_tiles = currentOccupied;
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
