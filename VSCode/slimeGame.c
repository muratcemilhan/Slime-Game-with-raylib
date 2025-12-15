#include "raylib.h"
#define eYukseklik 600
#define eGenislik 1000
int main(){

    SetTargetFPS(60);
    InitWindow(eGenislik,eYukseklik,"Slime Game");
    while(!WindowShouldClose()){

        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}