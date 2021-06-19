// This program runs a visualisation of a reaction diffusion algorithm

#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CAMERA_CONTROLS true

typedef struct {
    double a;
    double b;
} Cell;

typedef struct {
    Cell** cells;
} Grid;

/// calculate the average of all neighbours with weights depending position
double convolution(Grid *oldGrid, Grid *newGrid, int cellX, int cellY, bool a)
{
    double convolution = 0.0;
    
    if (a)
    {
        // Calculate adjacent neightbours at a weight of 0.2
        convolution += oldGrid->cells[cellX][cellY + 1].a * 0.2;
        convolution += oldGrid->cells[cellX + 1][cellY].a * 0.2;
        convolution += oldGrid->cells[cellX - 1][cellY].a * 0.2;
        convolution += oldGrid->cells[cellX][cellY - 1].a * 0.2;

        // Calculate diagonal neightbours at a weight of 0.05
        convolution += oldGrid->cells[cellX + 1][cellY + 1].a * 0.05;
        convolution += oldGrid->cells[cellX + 1][cellY - 1].a * 0.05;
        convolution += oldGrid->cells[cellX - 1][cellY + 1].a * 0.05;
        convolution += oldGrid->cells[cellX - 1][cellY - 1].a * 0.05;

        convolution -= oldGrid->cells[cellX][cellY].a;
    }
    else
    {
        // Calculate adjacent neightbours at a weight of 0.2
        convolution += oldGrid->cells[cellX][cellY + 1].b * 0.2;
        convolution += oldGrid->cells[cellX + 1][cellY].b * 0.2;
        convolution += oldGrid->cells[cellX - 1][cellY].b * 0.2;
        convolution += oldGrid->cells[cellX][cellY - 1].b * 0.2;

        // Calculate diagonal neightbours at a weight of 0.05
        convolution += oldGrid->cells[cellX + 1][cellY + 1].b * 0.05;
        convolution += oldGrid->cells[cellX + 1][cellY - 1].b * 0.05;
        convolution += oldGrid->cells[cellX - 1][cellY + 1].b * 0.05;
        convolution += oldGrid->cells[cellX - 1][cellY - 1].b * 0.05;

        convolution -= oldGrid->cells[cellX][cellY].b;
    }

    return convolution;
}

/// initiailse a oldGrid with cells set to a = 1, b = 0 with an input sized square of cells
/// randomly placed with a = 1, b = 1
void initialiseStartGrid(Grid* oldGrid, Grid* newGrid, int rows, int columns, int bSquareSize)
{
    // Malloc space for the 2D array of cells
    oldGrid->cells = (Cell **) malloc(rows * sizeof(Cell *));
    for (int i = 0; i < rows; i++)
    {
        oldGrid->cells[i] = (Cell *) malloc(columns * sizeof(Cell));
        for (int j = 0; j < columns; j++)
        {
            oldGrid->cells[i][j].a = 1;
            oldGrid->cells[i][j].b = 0;
        } 
    }

    
    // Get a random location for the square of b cells
    //int bX = GetRandomValue(bSquareSize, rows - bSquareSize);
    //int bY = GetRandomValue(bSquareSize, columns - bSquareSize);

    // Initiase a small square were b = 1
    for (int s = 1; s <= 3; s++)
    {
        for (int i = -bSquareSize; i < bSquareSize; i++)
        {
            for (int j = -bSquareSize; j < bSquareSize; j++)
            {
                oldGrid->cells[(rows * s/4) + i][(columns * s/4) + j].a = 1;
                oldGrid->cells[(rows * s/4) + i][(columns * s/4) + j].b = 1;
            }
        }
    }

    for (int i = -bSquareSize; i < bSquareSize; i++)
    {
        for (int j = -bSquareSize; j < bSquareSize; j++)
        {
            oldGrid->cells[(rows * 3/4) + i][(columns * 1/4) + j].a = 1;
            oldGrid->cells[(rows * 3/4) + i][(columns * 1/4) + j].b = 1;
        }
    }

    for (int i = -bSquareSize; i < bSquareSize; i++)
    {
        for (int j = -bSquareSize; j < bSquareSize; j++)
        {
            oldGrid->cells[(rows * 1/4) + i][(columns * 3/4) + j].a = 1;
            oldGrid->cells[(rows * 1/4) + i][(columns * 3/4) + j].b = 1;
        }
    }
}

void initialiseSwapGrid(Grid* grid, int rows, int columns)
{
    // Malloc space for the 2D array of cells
    grid->cells = (Cell **) malloc(rows * sizeof(Cell *));
    for (int i = 0; i < rows; i++)
    {
        grid->cells[i] = (Cell *) malloc(columns * sizeof(Cell));
        for (int j = 0; j < columns; j++)
        {
            grid->cells[i][j].a = 1;
            grid->cells[i][j].b = 0;
        } 
    }
}

int main(void)
{
    // Initialization
    //-----------------------------------------------------------------------------------
    const int screenWidth = 200;
    const int screenHeight = 200;

    InitWindow(screenWidth, screenHeight, "Reaction Diffusion by Justin Johnson");

    // Two grids used so the new oldGrid can be calculated without changing the old one
    Grid grids[2];
    initialiseSwapGrid(&grids[1], screenWidth, screenHeight);
    initialiseStartGrid(&grids[0], &grids[1], screenWidth, screenHeight, 5);

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
    
    // Main sim loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //-------------------------------------------------------------------------------

        #if CAMERA_CONTROLS
        // Camera zoom controls
        camera.zoom += ((float)GetMouseWheelMove()*0.05f);
        camera.offset = (Vector2){(float)GetMouseWheelMove()*0.05f, (float)GetMouseWheelMove()*0.05f};

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

        Grid *oldGrid = &grids[oldGridIndex];
        Grid *newGrid = &grids[newGridIndex];

        // Draw
        //-------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
                for (int i = 1; i < screenWidth - 1; i++)
                {
                    for (int j = 1; j < screenHeight - 1; j++)
                    {
                        Cell *oldCell = &oldGrid->cells[i][j];
                        Cell *newCell = &newGrid->cells[i][j];
                        double a = oldCell->a;
                        double b = oldCell->b;
                        double aConvolution = convolution(oldGrid, newGrid, i, j, true);
                        double bConvolution = convolution(oldGrid, newGrid, i, j, false);
                        
                        newCell->a = a + (dA * aConvolution - a * (b * b) + feedRate * (1 - a));
                        newCell->b = b + (dB * bConvolution + a * (b * b) - (killRate + feedRate) * b);
                        
                        double abDiff = newCell->a - newCell->b;
                        Color colour = {abDiff * 255, abDiff * 255, abDiff * 255, 255};
                        DrawPixel(i, j, colour);
                    }
                }
                newGridIndex = (newGridIndex + 1) % 2;
                oldGridIndex = (oldGridIndex + 1) % 2;
            EndMode2D();
            DrawFPS(0, 0);
        EndDrawing();
        //-------------------------------------------------------------------------------
    }
    // De-Initialization
    //-----------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //-----------------------------------------------------------------------------------

    return 0;
}
