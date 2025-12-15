#include "raylib.h"
#define eYukseklik 600
#define eGenislik 1000

int lastx = eGenislik; // global en son spawn noktası

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

typedef struct rakip {
    Vector2 konum;
    Vector2 hiz;
    Vector2 boyut;
    Color renk;
    bool aktif;
} Rakip;

void slimeHareket(Slime *s) {
    s->konum.y += s->hiz.y;
    s->hiz.y += 0.5f;
    if (s->hiz.y > 12.0f) s->hiz.y = 12.0f; // hız sınırı
}

void rakipCagir(Rakip rakip[], Masa *masa) {
    for (int i = 0; i < 5; i++) {
        if (rakip[i].aktif) {
            rakip[i].konum.x += rakip[i].hiz.x;

            // ekranın solundan çıktıysa yeniden doğur
            if (rakip[i].konum.x + rakip[i].boyut.x < 0) {
                // en uzak X'i bul
                int maxX = eGenislik;
                for (int j = 0; j < 5; j++) {
                    if (rakip[j].aktif && rakip[j].konum.x > maxX) {
                        maxX = rakip[j].konum.x;
                    }
                }

                int gap = GetRandomValue(250, 300);
                rakip[i].konum.x = maxX + gap; // en sağdakinin arkasına koy
                rakip[i].boyut.y = GetRandomValue(60, 70);
                rakip[i].konum.y = masa->konum.y - rakip[i].boyut.y;
            }
        }
    }
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

    Rakip rakip[5];
    int x = eGenislik;
    for (int i = 0; i < 5; i++) {
        int gap = GetRandomValue(250, 300);
        x += gap;
        rakip[i].boyut = (Vector2){20, GetRandomValue(50, 70)};
        rakip[i].konum = (Vector2){x, masa.konum.y - rakip[i].boyut.y};
        rakip[i].renk = BLACK;
        rakip[i].hiz = (Vector2){-3, 0};
        rakip[i].aktif = true;
    }
    lastx = x; // en sağdaki engeli referans al

    while (!WindowShouldClose()) {
        slimeHareket(&slime);
        rakipCagir(rakip, &masa);

        Rectangle slimeRec = {slime.konum.x, slime.konum.y, slime.sekil.x, slime.sekil.y};
        Rectangle masaRec  = {masa.konum.x, masa.konum.y, masa.boyut.x, masa.boyut.y};

        if (CheckCollisionRecs(slimeRec, masaRec)) {
            slime.konum.y = masa.konum.y - slime.sekil.y;
            slime.hiz.y = 0;
        }

        if (slime.konum.y + slime.sekil.y >= masa.konum.y &&
            slime.konum.y + slime.sekil.y <= masa.konum.y + 2 &&
            IsKeyPressed(KEY_SPACE)) {
            slime.hiz.y = -10.0f;
        }

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        for (int i = 0; i < 5; i++) {
            if (rakip[i].aktif) {
                DrawRectangleV(rakip[i].konum, rakip[i].boyut, rakip[i].renk);
            }
        }
        DrawRectangleV(slime.konum, slime.sekil, slime.renk);
        DrawRectangleV(masa.konum, masa.boyut, masa.renk);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
