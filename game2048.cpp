#include <stdlib.h>
#include <stdbool.h>
#include "game2048.h"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

static void copy_grid(int **source, int **destination, int size);

void initializeRandomnessSeed(int seed) { srand(seed); }

/*******************************************************************************
  Initialization Code
*******************************************************************************/
static void spawnTile(GameState *gs);

int **createGameGrid(int size);
GameState *initializeGameState(int size)
{
        GameState *gs = (GameState *)malloc(sizeof(GameState));
        gs->score = 0;
        gs->occupied_tiles = 0;
        gs->grid_size = size;
        gs->grid = createGameGrid(size);

        spawnTile(gs);
        return gs;
}

void freeGameGrid(int **grid, int size);
void freeGameState(GameState *gs)
{
        freeGameGrid(gs->grid, gs->grid_size);
        free(gs);
}

// Allocates a new game grid as a two-dimensional array
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

/*******************************************************************************
  Tile Spawning
*******************************************************************************/

static int generateNewTileValue() { return 2 + 2 * (rand() % 2); }
static int getRandomCoordinate() { return rand() % 4; }

static void spawnTile(GameState *gs)
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

/*******************************************************************************
  Tile Merging Logic
*******************************************************************************/

/* Helper functions for tile merging */

/// Merges the i-th row of tiles in the given direction (left/right).
static void mergeRow(GameState *gs, int i, int direction);

/// Reverses a given row of `row_size` elements in place.
static void reverse(int *row, int row_size);

/// Transposes the game trid in place to allow for merging vertically.
static void transpose(GameState *gs);

/// Returns the index of the next non-empty tile after the `current_index` in
/// the i-th row in of the grid.
static int getSuccessorIndex(GameState *gs, int i, int current_index);

// We only implement merging tiles left or right (row-wise), in order to merge
// the tiles in a vertical direction we first transpose the grid, merge and then
// transpose back.
static void merge(GameState *gs, int direction)
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

static void mergeRow(GameState *gs, int i, int direction)
{
        int curr = 0;
        int *merged_row = (int *)calloc(gs->grid_size, sizeof(int));
        int merged_num = 0;

        int size = gs->grid_size;

        // We always merge to the left (or up if we have previously transposed
        // the grid), in other cases we reverse the row, merge and reverse back.
        if (direction == DOWN || direction == RIGHT) {
                reverse(gs->grid[i], size);
        }

        // We find the first non-empty tile.
        curr = getSuccessorIndex(gs, i, -1);

        if (curr == size) {
                // All tiles are empty.
                return;
        }

        // Now the current tile must be non-empty.
        while (curr < size) {
                int succ = getSuccessorIndex(gs, i, curr);
                // Two matching tiles found, we perform a merge.
                int curr_val = gs->grid[i][curr];
                if (succ < size && curr_val == gs->grid[i][succ]) {
                        int sum = curr_val + gs->grid[i][succ];
                        gs->score += sum;
                        gs->occupied_tiles--;
                        merged_row[merged_num] = sum;
                        curr = getSuccessorIndex(gs, i, succ);
                } else {
                        merged_row[merged_num] = curr_val;
                        curr = succ;
                }
                merged_num++;
        }

        for (int j = 0; j < size; j++) {
                if (direction == DOWN || direction == RIGHT) {
                        gs->grid[i][gs->grid_size - 1 - j] = merged_row[j];
                } else {
                        gs->grid[i][j] = merged_row[j];
                }
        }
        free(merged_row);
}

// Reverses a given row of four elements in place
static void reverse(int *row, int row_size)
{
        for (int i = 0; i < row_size / 2; i++) {
                int temp = row[i];
                row[i] = row[row_size - i - 1];
                row[row_size - i - 1] = temp;
        }
}

static int getSuccessorIndex(GameState *gs, int i, int current_index)
{
        int succ = current_index + 1;
        while (succ < gs->grid_size && gs->grid[i][succ] == 0) {
                succ++;
        }
        return succ;
}

static void transpose(GameState *gs)
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
  Game Loop Logic
*******************************************************************************/

// Helper functions for the game loop
static bool theGridChangedFrom(GameState *gs, int **oldGrid);
static bool isBoardFull(GameState *gs);
static bool noMovePossible(GameState *gs);

bool isGameOver(GameState *gs) { return isBoardFull(gs) && noMovePossible(gs); }

static bool isBoardFull(GameState *gs)
{
        return gs->occupied_tiles >= gs->occupied_tiles * gs->occupied_tiles;
}

void takeTurn(GameState *gs, int direction)
{
        int **oldGrid = createGameGrid(gs->grid_size);
        copy_grid(gs->grid, oldGrid, gs->grid_size);
        merge(gs, direction);

        if (theGridChangedFrom(gs, oldGrid)) {
                spawnTile(gs);
        }
        freeGameGrid(oldGrid, gs->grid_size);
}

static bool theGridChangedFrom(GameState *gs, int **oldGrid)
{
        for (int i = 0; i < gs->grid_size; i++) {
                for (int j = 0; j < gs->grid_size; j++) {
                        if (gs->grid[i][j] != oldGrid[i][j]) {
                                return true;
                        }
                }
        }
        return false;
}

static bool noMovePossible(GameState *gs)
{
        // A move is always possible if not all tiles are occupied.
        if (gs->occupied_tiles < gs->grid_size * gs->grid_size) {
                return false;
        }

        // When the grid is full a move is possible as long as there exist some
        // adjacent tiles that have the same number
        for (int i = 0; i < gs->grid_size; i++) {
                for (int j = 0; j < gs->grid_size - 1; j++) {
                        // row-wise adjacency
                        if (gs->grid[i][j] == gs->grid[i][j + 1]) {
                                return false;
                        }
                        // colunm-wise adjacency
                        if (gs->grid[j][i] == gs->grid[j + 1][i]) {
                                return false;
                        }
                }
        }
        return true;
}

static void copy_grid(int **source, int **destination, int size)
{
        for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                        destination[i][j] = source[i][j];
                }
        }
}
