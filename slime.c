#include "raylib.h"

// Maksimum partikül sayısı (Havuzun boyutu)
#define MAX_PARTIKUL 200

// 1. ADIM: Partikülün neye benzediğini tanımlayalım
typedef struct {
    Vector2 konum;   // Nerede?
    Vector2 hiz;     // Nereye gidiyor?
    float boyut;     // Ne kadar büyük?
    float omur;      // Ne kadar yaşayacak? (1.0 = tam dolu, 0.0 = öldü)
    Color renk;      // Rengi ne?
    bool aktif;      // Şu an ekranda mı?
} Partikul;

// Global partikül havuzumuz
Partikul partikuller[MAX_PARTIKUL];

// 2. ADIM: Yeni bir partikül oluşturma fonksiyonu
void PartikulOlustur(Vector2 baslangicYeri) {
    // Havuzu tara ve boş (aktif olmayan) bir partikül bul
    for (int i = 0; i < MAX_PARTIKUL; i++) {
        if (!partikuller[i].aktif) {
            
            // Bulduk! Şimdi özelliklerini verelim:
            partikuller[i].aktif = true;
            partikuller[i].konum = baslangicYeri;
            
            // Rastgele bir yöne fırlasın (-5 ile +5 arası hız)
            partikuller[i].hiz.x = (float)GetRandomValue(-50, 50) / 10.0f; 
            partikuller[i].hiz.y = (float)GetRandomValue(-50, 50) / 10.0f;
            
            // Rastgele boyut ve renk
            partikuller[i].boyut = (float)GetRandomValue(5, 15);
            partikuller[i].omur = 1.0f; // Tam ömürle başla
            
            // Rastgele renk seçimi (Kırmızı, Turuncu veya Sarı)
            int renkSecim = GetRandomValue(0, 2);
            if (renkSecim == 0) partikuller[i].renk = MAROON;
            else if (renkSecim == 1) partikuller[i].renk = ORANGE;
            else partikuller[i].renk = GOLD;

            break; // Bir tane oluşturduk, döngüden çık.
                   // (Çoklu oluşturmak istersen buradaki break'i kaldırıp dışarıya sayaç koyarsın)
        }
    }
}

int main() {
    InitWindow(800, 600, "Partikül Sistemi Öğrenme - Raylib");
    SetTargetFPS(60);

    // Başlangıçta tüm partikülleri "ölü" yapalım
    for (int i = 0; i < MAX_PARTIKUL; i++) {
        partikuller[i].aktif = false;
    }

    while (!WindowShouldClose()) {
        
        // --- GİRİŞ (INPUT) ---
        
        // Sol tık ile patlama (Tek seferde 20 tane partikül fırlat)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for(int i=0; i<20; i++) {
                PartikulOlustur(GetMousePosition());
            }
        }

        // Sağ tık basılı tutulursa sürekli akış (Alev gibi)
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            PartikulOlustur(GetMousePosition());
            PartikulOlustur(GetMousePosition()); // Daha yoğun olsun diye 2 tane
        }

        // --- GÜNCELLEME (UPDATE) ---
        
        for (int i = 0; i < MAX_PARTIKUL; i++) {
            if (partikuller[i].aktif) {
                // 1. Hareket ettir
                partikuller[i].konum.x += partikuller[i].hiz.x;
                partikuller[i].konum.y += partikuller[i].hiz.y;

                // 2. Yer çekimi ekle (İsteğe bağlı - aşağı düşmesi için)
                partikuller[i].hiz.y += 0.1f; 

                // 3. Ömrünü azalt
                partikuller[i].omur -= 0.02f; // Her karede %2 canı gitsin
                
                // 4. Boyutunu küçült (Giderek yok olsun)
                partikuller[i].boyut -= 0.1f;

                // 5. Öldü mü kontrol et
                if (partikuller[i].omur <= 0.0f || partikuller[i].boyut <= 0.0f) {
                    partikuller[i].aktif = false; // Havuza geri döndü
                }
            }
        }

        // --- ÇİZİM (DRAW) ---
        
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Sol Tik: PATLAMA | Sag Tik (Basili Tut): ALEV", 10, 10, 20, WHITE);
        DrawText("Mantigi: Havuz Sistemi (Object Pooling)", 10, 40, 15, GRAY);

        for (int i = 0; i < MAX_PARTIKUL; i++) {
            if (partikuller[i].aktif) {
                // Fade fonksiyonu renge şeffaflık (alpha) katar. 
                // Ömür azaldıkça rengi soluklaşır.
                DrawCircleV(partikuller[i].konum, partikuller[i].boyut, Fade(partikuller[i].renk, partikuller[i].omur));
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}