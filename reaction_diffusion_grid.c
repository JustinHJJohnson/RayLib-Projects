// This program runs a visualisation of a reaction diffusion algorithm
// Based on this tutorial http://karlsims.com/rd.html

#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CAMERA_CONTROLS true

/// A stucture used to store the data for each pixel in the simulation
typedef struct Cell Cell;

struct Cell{
    double *a;
    double *b;
    int x;
    int y;
    Cell **adjacent;
    Cell **diagonal;
    int lastFrameChecked;
};

// TODO store centre points and propagate out from there recursively based on number of frames rendered to reduce checks

typedef struct {
    Cell **cells;
} Grid;

/// Calculate the average of all neighbours with weights depending position
/// @param cell The cell to run the convolution on
/// @param a Whether to run this operation on a cell's a chemical or b chemical
/// @param oldIndex Which index in the a or b array is from the previous frame
/// @return The weighted average of this cell's neighbouring cells
double convolution(Cell *cell, bool a, int oldIndex)
{
    double convolution = 0.0;

    if (a)
    {
        // Calculate adjacent neightbours at a weight of 0.2
        for (int i = 0; i < 4; i++)
        {
            convolution += cell->adjacent[i]->a[oldIndex] * 0.2;
        }

        // Calculate diagonal neightbours at a weight of 0.05
        for (int i = 0; i < 4; i++)
        {
            convolution += cell->diagonal[i]->a[oldIndex] * 0.05;
        }

        convolution -= cell->a[oldIndex];
    }
    else
    {
        // Calculate adjacent neightbours at a weight of 0.2
        for (int i = 0; i < 4; i++)
        {
            convolution += cell->adjacent[i]->b[oldIndex] * 0.2;
        }

        // Calculate diagonal neightbours at a weight of 0.05
        for (int i = 0; i < 4; i++)
        {
            convolution += cell->diagonal[i]->b[oldIndex] * 0.05;
        }

        convolution -= cell->b[oldIndex];
    }

    return convolution;
}

/// initiailse a grid with cells set to a = 1, b = 0 with an input sized square of cells 
/// randomly placed with a = 1, b = 1
/// @param grid The grid to initialise
/// @param rows The number of vertical pixels in the simulation
/// @param columns The number of horizontal pixels in the simulation
/// @param bSquareSize The size of the square of pixels used to start the simulation
void initialiseGrid(Grid* grid, int rows, int columns, int bSquareSize)
{
    // Malloc space for the 2D array of cells
    grid->cells = (Cell **) malloc(rows * sizeof(Cell *));
    for (int i = 0; i < rows; i++)
    {
        grid->cells[i] = (Cell *) malloc(columns * sizeof(Cell));
        for (int j = 0; j < columns; j++)
        {
            grid->cells[i][j].a = (double *) malloc(2 * sizeof(double));
            grid->cells[i][j].b = (double *) malloc(2 * sizeof(double));
            grid->cells[i][j].a[0] = 1;
            grid->cells[i][j].b[0] = 0;
            grid->cells[i][j].a[1] = 0;
            grid->cells[i][j].b[1] = 0;
            grid->cells[i][j].adjacent = (Cell *) malloc(4 * sizeof(Cell));
            grid->cells[i][j].diagonal = (Cell *) malloc(4 * sizeof(Cell));
            grid->cells[i][j].lastFrameChecked = 0;
        } 
    }

    
    // Get a random location for the square of b cells
    //int bX = GetRandomValue(bSquareSize, rows - bSquareSize);
    //int bY = GetRandomValue(bSquareSize, columns - bSquareSize);

    // Initiase a small square were b = 1
    /* for (int s = 1; s <= 3; s++)
    {
        for (int i = -bSquareSize; i < bSquareSize; i++)
        {
            for (int j = -bSquareSize; j < bSquareSize; j++)
            {
                grid->cells[(rows * s/4) + i][(columns * s/4) + j].a[0] = 1;
                grid->cells[(rows * s/4) + i][(columns * s/4) + j].b[0] = 1;
            }
        }
    }

    for (int i = -bSquareSize; i < bSquareSize; i++)
    {
        for (int j = -bSquareSize; j < bSquareSize; j++)
        {
            grid->cells[(rows * 3/4) + i][(columns * 1/4) + j].a[0] = 1;
            grid->cells[(rows * 3/4) + i][(columns * 1/4) + j].b[0] = 1;
        }
    }

    for (int i = -bSquareSize; i < bSquareSize; i++)
    {
        for (int j = -bSquareSize; j < bSquareSize; j++)
        {
            grid->cells[(rows * 1/4) + i][(columns * 3/4) + j].a[0] = 1;
            grid->cells[(rows * 1/4) + i][(columns * 3/4) + j].b[0] = 1;
        }
    } */

    // Setup up the starting square of pixels
    for (int i = -bSquareSize; i < bSquareSize; i++)
    {
        for (int j = -bSquareSize; j < bSquareSize; j++)
        {
            grid->cells[rows / 2 + i][columns / 2 + j].a[0] = 1;
            grid->cells[rows / 2 + i][columns / 2 + j].b[0] = 1;
        }
    }

    // Store a pointer to each of a cells neighbours
    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < columns - 1; j++)
        {
            grid->cells[i][j].adjacent[0] = &grid->cells[i + 1][j];
            grid->cells[i][j].adjacent[1] = &grid->cells[i - 1][j];
            grid->cells[i][j].adjacent[2] = &grid->cells[i][j + 1];
            grid->cells[i][j].adjacent[3] = &grid->cells[i][j - 1];

            grid->cells[i][j].diagonal[0] = &grid->cells[i + 1][j + 1];
            grid->cells[i][j].diagonal[1] = &grid->cells[i + 1][j - 1];
            grid->cells[i][j].diagonal[2] = &grid->cells[i - 1][j + 1];
            grid->cells[i][j].diagonal[3] = &grid->cells[i - 1][j - 1];
        } 
    }
}

int main(void)
{
    // Initialisation
    //-----------------------------------------------------------------------------------
    const int screenWidth = 200;
    const int screenHeight = 200;

    InitWindow(screenWidth, screenHeight, "Reaction Diffusion by Justin Johnson");

    // Initialise the grid
    Grid grid;
    initialiseGrid(&grid, screenWidth, screenHeight, 5);

    // Setup the camera
    Camera2D camera = { 0 };
    //camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(144);
    //-----------------------------------------------------------------------------------
    
    //int cellSize = 50;
    int newGridIndex = 1;
    int oldGridIndex = 0;
    
    // Setup values for the function to calculate new values of a cells a and b properties
    double feedRate = 0.055;
    double killRate = 0.062;
    double dA = 1;
    double dB = 0.5;

    // The current frame being rendered, used to check if a cell has already been calculated this frame
    int frame = 1;
    
    // Main sim loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //-------------------------------------------------------------------------------

        #if CAMERA_CONTROLS
        // Camera zoom controls
        camera.zoom += ((float)GetMouseWheelMove()*0.05f);
        camera.offset = (Vector2)
        {
            (float) GetMouseWheelMove() * 0.05f,
            (float) GetMouseWheelMove() * 0.05f
        };

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            camera.rotation = 0.0f;
        }
        //-------------------------------------------------------------------------------
        #endif

        // Draw
        //-------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
                for (int i = 1; i < screenWidth - 1; i++)
                {
                    for (int j = 1; j < screenHeight - 1; j++)
                    {
                        double *oldA = &grid.cells[i][j].a[oldGridIndex];
                        double *oldB = &grid.cells[i][j].b[oldGridIndex];
                        double *newA = &grid.cells[i][j].a[newGridIndex];
                        double *newB = &grid.cells[i][j].b[newGridIndex];
                        
                        double aConvolution = convolution(&grid.cells[i][j], true, oldGridIndex);
                        double bConvolution = convolution(&grid.cells[i][j], false, oldGridIndex);
                        
                        *newA = *oldA + (dA * aConvolution - *oldA * (*oldB * *oldB) + feedRate * (1 - *oldA));
                        *newB = *oldB + (dB * bConvolution + *oldA * (*oldB * *oldB) - (killRate + feedRate) * *oldB);
                        
                        double abDiff = *newA - *newB;
                        Color colour = {abDiff * 255, abDiff * 255, abDiff * 255, 255};
                        DrawPixel(i, j, colour);
                    }
                }

                newGridIndex = (newGridIndex + 1) % 2;
                oldGridIndex = (oldGridIndex + 1) % 2;
                frame++;
            EndMode2D();
            DrawFPS(0, 0);
        EndDrawing();
        //-------------------------------------------------------------------------------
    }
    // De-Initialisation
    //-----------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //-----------------------------------------------------------------------------------

    return 0;
}
