#include "raylib.h"
#define eYukseklik 600
#define eGenislik 1000
int main(){

        typedef struct slime
        {
        Color renk;
        Vector2 konum;
        Vector2 sekil;
        Vector2 hiz;
        }Slime;
        
    SetTargetFPS(60);
    InitWindow(eGenislik,eYukseklik,"Slime Game");


        Slime slime;
        slime.konum=(Vector2){70,eYukseklik/3};
        slime.renk=(GREEN);
        

    while(!WindowShouldClose()){

        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}