/*******************************************************************************************
*
*   raylib [core] example - 2d camera
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CAMERA_CONTROLS true
#define __USE_MINGW_ANSI_STDIO 1

typedef struct {
    Vector2 centre;
    float innerRadius;
    float outerRadius;
    float startAngle;
    float endAngle;
    int segments;
    Color colour;
} Ring;

typedef struct node {
    Ring ring;
    struct node *next;
} node_t;


node_t *head = NULL;

void addRing(Ring ring)
{
    //printf("Adding a ring\n");
    
    node_t *node = (node_t *)malloc(sizeof(node_t));
    node_t *current = head;

    node->next = NULL;
    node->ring = ring;

    if (head == NULL) 
    {
        //printf("Making head node in add function\n");
        head = node;
        return;
    }

    while (current->next != NULL) current = current->next;

    current->next = node;
}

void drawRingFromStruct(Ring ring)
{
    DrawRing(
        ring.centre,
        ring.innerRadius,
        ring.outerRadius,
        ring.startAngle,
        ring.endAngle,
        ring.segments,
        ring.colour
    );
    
}

void drawRingList()
{
    //printf("Started drawing list\n");
    node_t *current = head;
    int numRingsDrawn = 0;

    while (current != NULL) 
    {
        //printf("Drawing a ring from the list\n");
        drawRingFromStruct(current->ring);
        current = current->next;
        numRingsDrawn++;
    }
}

int main(void)
{
    // Initialization
    //-----------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Reaction Diffusion by Justin Johnson");

    // Setup the camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(30);
    //-----------------------------------------------------------------------------------

    int count = 1;
    int increment = 25;
    
    // Main sim loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //-------------------------------------------------------------------------------

        double radius =  ((count + increment) - count) / 2;
        double centreX = count + increment / 2;
        Ring ring = {(Vector2){centreX, screenHeight / 2}, radius, radius + 1, 90, 270, 100, BLACK};
        addRing(ring);

        // Draw
        //-------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode2D(camera);
                drawRingList();
                count += increment;
                //camera.offset = (Vector2){GetScreenWidth() - ((ring.outerRadius * 2) * ((count + increment) / increment)), 0.0f};
                camera.offset = (Vector2){screenWidth - (increment * ((count + increment) / increment)), 0.0f};                
                //printf("Camera offset is %f %f\n", camera.offset.x, camera.offset.y);
                //camera.zoom = screenWidth / count;
            EndMode2D();
            DrawFPS(0, 0);
            char *text = TextFormat("Number of rings drawn %d", (count + increment) / increment);
            DrawText(text, screenWidth - 280, 0, 20, BLACK);
        EndDrawing();
        //-------------------------------------------------------------------------------
    }
    // De-Initialization
    //-----------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //-----------------------------------------------------------------------------------

    return 0;
}
