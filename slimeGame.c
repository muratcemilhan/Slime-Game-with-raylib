#include "raylib.h"
#define eYukseklik 600
#define eGenislik 1000

typedef struct slime {
    Color renk;
    Vector2 konum;
    Vector2 sekil;
    Vector2 hiz;
} Slime;

typedef struct masa {
    Vector2 konum;
    Color renk;
    Vector2 boyut;
} Masa;

void slimeHareket(Slime *s) {
    s->konum.y += s->hiz.y;
    s->hiz.y += 0.5f; 
}

int main() {
    SetTargetFPS(60);
    InitWindow(eGenislik, eYukseklik, "Slime Game");

    Slime slime;
    slime.konum = (Vector2){100, eYukseklik/2};
    slime.renk = GREEN;
    slime.sekil = (Vector2){30, 30};
    slime.hiz = (Vector2){0, 0};

    Masa masa;
    masa.konum = (Vector2){0, eYukseklik/3*2};
    masa.boyut = (Vector2){eGenislik, 10};
    masa.renk = BROWN;

    while (!WindowShouldClose()) {
        slimeHareket(&slime);

        Rectangle slimeRec = {slime.konum.x, slime.konum.y, slime.sekil.x, slime.sekil.y};
        Rectangle masaRec  = {masa.konum.x, masa.konum.y, masa.boyut.x, masa.boyut.y};

    
        if (CheckCollisionRecs(slimeRec, masaRec)) {
            slime.konum.y = masa.konum.y - slime.sekil.y;
            slime.hiz.y = 0;
        }

        if (slime.konum.y + slime.sekil.y >= masa.konum.y &&
            slime.konum.y + slime.sekil.y <= masa.konum.y + 2 && 
            IsKeyDown(KEY_SPACE)) {
            slime.hiz.y = -10.0f;
        }

        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        DrawRectangleV(slime.konum, slime.sekil, slime.renk);
        DrawRectangleV(masa.konum, masa.boyut, masa.renk);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
