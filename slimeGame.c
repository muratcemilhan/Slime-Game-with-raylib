#include "raylib.h"
#include <stdio.h> 

// --- SABİTLER ---
#define EKRAN_YUKSEKLIK 600
#define EKRAN_GENISLIK 1000
#define MAX_PARTIKUL 100
#define SKOR_KAYIT_SAYISI 5 // Liderlik tablosunda kaç kişi olacak?

// --- YAPI (STRUCT) TANIMLARI ---
typedef struct {
    Vector2 konum;
    Vector2 hiz;
    float omur;     
    float boyut;
    bool aktif;     
} Partikul;

typedef struct {
    Color renk;      
    Vector2 konum;   
    Vector2 sekil;   
    Vector2 hiz;     
    bool yerdeMi;    
} Slime;

typedef struct {
    Vector2 konum;   
    Color renk;      
    Vector2 boyut;   
} Masa;

typedef struct {
    Vector2 konum;   
    Vector2 boyut;   
    Color renk;      
    bool aktif;      
    bool gecildi;
    int tip;         // 0 = Kütük, 1 = Kuş
    int yon;         // 1 = Aşağı, -1 = Yukarı
} Rakip;

// --- GLOBAL DEĞİŞKENLER ---
Slime slime;
Masa masa;
Rakip rakipler[5]; 
Partikul partikuller[MAX_PARTIKUL];     

int score = 0;          
int liderlikTablosu[SKOR_KAYIT_SAYISI] = {0}; // YENİ: Top 5 skor dizisi

float oyunHizi = -4.0f; 
float arkaplanX = 0.0f;
float zeminX = 0.0f; 

bool oyunAktif = false; 
bool oyunBitti = false; 
bool oyunDuraklatildi = false; 
bool muzikAcik = true; 

Sound ziplama;
Music oyunmuzigi;
Sound yuksekSkor; 
Sound kaybetme;

// Yerde koşarken efekt sıklığını kontrol etmek için sayaç
int kosmaSayaci = 0;

// --- YARDIMCI FONKSİYONLAR  ---

void LiderlikTablosunuYukle() {
    FILE *dosya = fopen("liderlik_tablosu.txt", "r");
    if (dosya != NULL) {
        for (int i = 0; i < SKOR_KAYIT_SAYISI; i++) {
            if (fscanf(dosya, "%d", &liderlikTablosu[i]) != 1) {
                liderlikTablosu[i] = 0; // Okuma hatası olursa 0 yap
            }
        }
        fclose(dosya);
    }
}

void LiderlikTablosunuKaydet() {
    FILE *dosya = fopen("liderlik_tablosu.txt", "w");
    if (dosya != NULL) {
        for (int i = 0; i < SKOR_KAYIT_SAYISI; i++) {
            fprintf(dosya, "%d\n", liderlikTablosu[i]);
        }
        fclose(dosya);
    }
}

// Skoru listeye ekleyip kaydırma yapan fonksiyon
void SkoruTabloyaIsle(int yeniSkor) {
    for (int i = 0; i < SKOR_KAYIT_SAYISI; i++) {
        // Eğer yeni skor, bu sıradaki skordan büyükse araya girer
        if (yeniSkor > liderlikTablosu[i]) {
            
            // 1. Aşağıdakileri kaydır (En sondan başlayarak)
            for (int k = SKOR_KAYIT_SAYISI - 1; k > i; k--) {
                liderlikTablosu[k] = liderlikTablosu[k - 1];
            }
            
            // 2. Yeni skoru yerleştir
            liderlikTablosu[i] = yeniSkor;
            
            // 3. Kaydet ve çık
            LiderlikTablosunuKaydet();
            break; 
        }
    }
}

// --- PARTİKÜL SİSTEMİ (AYNI) ---
void PartikulSifirla() {
    for(int i = 0; i < MAX_PARTIKUL; i++) {
        partikuller[i].aktif = false;
    }
}

void TozEfektiOlustur(float x, float y, int miktar) {
    int sayac = 0;
    for (int i = 0; i < MAX_PARTIKUL; i++) {
        if (!partikuller[i].aktif) {
            partikuller[i].aktif = true;
            partikuller[i].konum = (Vector2){x + GetRandomValue(-5, 10), y}; 
            
            if(miktar == 1) {
                 partikuller[i].hiz.x = (float)GetRandomValue(-30, -10) / 10.0f; 
                 partikuller[i].hiz.y = (float)GetRandomValue(-10, 0) / 10.0f;   
            } else {
                 partikuller[i].hiz.x = (float)GetRandomValue(-20, 20) / 10.0f;
                 partikuller[i].hiz.y = (float)GetRandomValue(-30, -10) / 10.0f;
            }
            
            partikuller[i].omur = 1.0f;
            partikuller[i].boyut = (float)GetRandomValue(3, 8);
            
            sayac++;
            if (sayac >= miktar) break; 
        }
    }
}

void PartikulGuncelle() {
    for (int i = 0; i < MAX_PARTIKUL; i++) {
        if (partikuller[i].aktif) {
            partikuller[i].konum.x += partikuller[i].hiz.x + oyunHizi; 
            partikuller[i].konum.y += partikuller[i].hiz.y;
            
            partikuller[i].omur -= 0.04f; 
            partikuller[i].boyut -= 0.1f; 

            if (partikuller[i].omur <= 0 || partikuller[i].boyut <= 0) {
                partikuller[i].aktif = false;
            }
        }
    }
}



// --- OYUN MANTIĞI ---
void EngelOlustur(int index, float xKonumu) {
    int rastgele = GetRandomValue(0, 100);
    
    if (rastgele < 30) { // KUŞ
        rakipler[index].tip = 1; 
        rakipler[index].boyut = (Vector2){40, 30}; 
        rakipler[index].konum.y = masa.konum.y - GetRandomValue(10,100); 
        rakipler[index].yon = 1; 
        rakipler[index].renk = RED; 
    } else { // KÜTÜK
        rakipler[index].tip = 0;
        rakipler[index].boyut = (Vector2){GetRandomValue(30,40), (float)GetRandomValue(90, 110)};
        rakipler[index].konum.y = masa.konum.y - rakipler[index].boyut.y;
        rakipler[index].renk = RAYWHITE;
    }

    rakipler[index].konum.x = xKonumu;
    rakipler[index].aktif = true;
    rakipler[index].gecildi = false;
}

void OyunuSifirla() {
    masa.konum = (Vector2){0, EKRAN_YUKSEKLIK / 3.0f * 2.0f}; 
    masa.boyut = (Vector2){EKRAN_GENISLIK, 20}; 
    masa.renk = BROWN;

    slime.renk = GREEN;
    slime.sekil = (Vector2){40, 40}; 
    slime.konum = (Vector2){100, masa.konum.y - slime.sekil.y}; 
    slime.hiz = (Vector2){0, 0};
    slime.yerdeMi = true; 

    oyunHizi = -4.0f;       
    int x = EKRAN_GENISLIK; 
    
    for (int i = 0; i < 5; i++) {
        int gap = GetRandomValue(300, 450); 
        x += gap; 
        EngelOlustur(i, (float)x); 
    }
    
    StopMusicStream(oyunmuzigi);
    PartikulSifirla(); 

    score = 0;           
    oyunBitti = false;
    oyunDuraklatildi = false; 
    arkaplanX = 0.0f;
    zeminX = 0.0f;
}

void SlimeHareket() {
    slime.konum.y += slime.hiz.y;
    slime.hiz.y += 0.6f; 

    // Yere Çarpma Kontrolü
    if (slime.konum.y + slime.sekil.y >= masa.konum.y) {
        
        if (slime.yerdeMi == false) {
            TozEfektiOlustur(slime.konum.x, masa.konum.y, 15);
        }
          
        slime.konum.y = masa.konum.y - slime.sekil.y; 
        slime.hiz.y = 0; 
        slime.yerdeMi = true; 

        kosmaSayaci++;
        if(kosmaSayaci >= 5) {
             TozEfektiOlustur(slime.konum.x, masa.konum.y, 2);
             kosmaSayaci = 0;
        }

    } else {
        slime.yerdeMi = false; 
        kosmaSayaci = 0;
    }

    if (IsKeyPressed(KEY_SPACE) && slime.yerdeMi) {
        slime.hiz.y = -12.5f; 
        PlaySound(ziplama); 
        TozEfektiOlustur(slime.konum.x + 10, masa.konum.y, 10);
        slime.yerdeMi = false;
    }
}

void RakipYonetimi() {
    for (int i = 0; i < 5; i++) {
        if (rakipler[i].aktif) {
            rakipler[i].konum.x += oyunHizi;
            
            if (rakipler[i].tip == 1) { 
                rakipler[i].konum.y += rakipler[i].yon * 2.0f;
                if (rakipler[i].konum.y < masa.konum.y - 140) rakipler[i].yon = 1;
                if (rakipler[i].konum.y > masa.konum.y - 50) rakipler[i].yon = -1;
            }

            if (rakipler[i].konum.x + rakipler[i].boyut.x < 0) {
                float maxX = 0;
                for (int j = 0; j < 5; j++) {
                    if (rakipler[j].konum.x > maxX) maxX = rakipler[j].konum.x;
                }
                int seviye = score / 20; 
                int ekMesafe = seviye * 30; 
                int gap = GetRandomValue(300 + ekMesafe, 500 + ekMesafe); 
                EngelOlustur(i, maxX + gap);
            }
        }
    }
}

// --- ANA PROGRAM ---
int main() {
    InitWindow(EKRAN_GENISLIK, EKRAN_YUKSEKLIK, "Slime Runner");
    InitAudioDevice();
   
    ziplama = LoadSound("ziplama.mp3");
    oyunmuzigi = LoadMusicStream("oyunmuzigi.mp3");
    yuksekSkor = LoadSound("yuksekskor.mp3"); 
    kaybetme = LoadSound("kaybetme.mp3");
    
    SetTargetFPS(60); 
    
    LiderlikTablosunuYukle(); // Başlangıçta listeyi yükle

    Texture2D arkaplan = LoadTexture("arkaplan.jpeg");
    Texture2D toprak = LoadTexture("toprak.jpeg");
    Texture2D slimeT = LoadTexture("slime.png");
    Texture2D kutuk = LoadTexture("kutuk.png");
    Texture2D kusT = LoadTexture("kus.png"); 
    
    OyunuSifirla(); 
   
    float slimeYaricap = slime.sekil.x / 2.0f - 2.0f;
    
    while (!WindowShouldClose()) {
        UpdateMusicStream(oyunmuzigi); 

        if(IsKeyPressed(KEY_V)){
            if(muzikAcik == true){
                PauseMusicStream(oyunmuzigi);
                muzikAcik = false;
            }
            else { 
                ResumeMusicStream(oyunmuzigi);
                muzikAcik = true;
            }
        }

        if (oyunAktif && !oyunBitti) {
            if (IsKeyPressed(KEY_P)) {
                oyunDuraklatildi = !oyunDuraklatildi;
                if (oyunDuraklatildi) PauseMusicStream(oyunmuzigi); 
                else if(muzikAcik) ResumeMusicStream(oyunmuzigi);
            }
        }

        // --- OYUN DÖNGÜSÜ ---
        if (oyunAktif && !oyunBitti && !oyunDuraklatildi) {
            SlimeHareket();   
            PartikulGuncelle(); 
            RakipYonetimi();  

            arkaplanX += oyunHizi * 0.2f; 
            if (arkaplanX <= -1000) arkaplanX = 0.0f;

            zeminX += oyunHizi;
            if (zeminX <= -1000) zeminX = 0.0f;
            
            // Çarpışma Kontrolleri
            for (int i = 0; i < 5; i++) {
                Vector2 slimeMerkez = { 
                    slime.konum.x + slime.sekil.x / 2.0f,  
                    slime.konum.y + slime.sekil.y / 2.0f   
                };
                Rectangle rakipRec = {rakipler[i].konum.x, rakipler[i].konum.y, rakipler[i].boyut.x, rakipler[i].boyut.y};

                if (CheckCollisionCircleRec(slimeMerkez, slimeYaricap, rakipRec)) {
                    oyunBitti = true; 
                    StopMusicStream(oyunmuzigi); 
                    
                    // --- YENİ KAYIT SİSTEMİ ---
                    // Eğer mevcut skor, listedeki 5. (en kötü) skordan büyükse listeye girer
                    if (score > liderlikTablosu[SKOR_KAYIT_SAYISI - 1]) {
                        SkoruTabloyaIsle(score);
                        PlaySound(yuksekSkor); 
                    } else {
                        PlaySound(kaybetme);
                    }
                }
                
                if (!rakipler[i].gecildi && slime.konum.x > rakipler[i].konum.x + rakipler[i].boyut.x) {
                    score++;
                    rakipler[i].gecildi = true;
                    if (score % 10 == 0) oyunHizi -= 0.5f;
                }
            }
        }
        else {
            if (!oyunDuraklatildi) {
                if (!oyunBitti && IsKeyPressed(KEY_SPACE)) {
                    oyunAktif = true;
                    if(muzikAcik) PlayMusicStream(oyunmuzigi);
                }
                if (oyunBitti && IsKeyPressed(KEY_R)) {
                    OyunuSifirla();     
                    oyunAktif = false;  
                    arkaplanX = 0;
                }
            }
        }

        // --- ÇİZİM ---
        BeginDrawing();
        ClearBackground(RAYWHITE); 
        
        // Arkaplan
        Rectangle boyutArkaplan= {0, 0,(float)arkaplan.width,(float)arkaplan.height};
        Rectangle konumArkaplan1= {arkaplanX, 0,1000,400};
        DrawTexturePro(arkaplan,boyutArkaplan,konumArkaplan1,(Vector2){0,0},0.0f,RAYWHITE);
        Rectangle konumArkaplan2= {arkaplanX+1000, 0,1000,400};
        DrawTexturePro(arkaplan,boyutArkaplan,konumArkaplan2,(Vector2){0,0},0.0f,RAYWHITE);
        
        DrawRectangleV(masa.konum, masa.boyut, masa.renk);
        
        // Zemin
        Rectangle boyutToprak={0,0,toprak.width,toprak.height};
        Rectangle konumToprak1={zeminX, 400, 1000, 200};
        DrawTexturePro(toprak,boyutToprak,konumToprak1,(Vector2){0,0},0.0f,RAYWHITE);
        Rectangle konumToprak2={zeminX+1000, 400, 1000, 200};
        DrawTexturePro(toprak,boyutToprak,konumToprak2,(Vector2){0,0},0.0f,RAYWHITE);

        // Engeller
        for (int i = 0; i < 5; i++) {
            if (rakipler[i].tip == 0) {
                Rectangle boyutKutuk ={0,0,kutuk.width,kutuk.height};
                Rectangle konumKutuk ={rakipler[i].konum.x,rakipler[i].konum.y,30,rakipler[i].boyut.y};
                DrawTexturePro(kutuk,boyutKutuk,konumKutuk,(Vector2){0,0},0.0f,RAYWHITE);
            } else {
                Rectangle boyutKusKaynak = {0, 0, kusT.width, kusT.height};
                Rectangle konumKus = {rakipler[i].konum.x, rakipler[i].konum.y, rakipler[i].boyut.x, rakipler[i].boyut.y};
                DrawTexturePro(kusT, boyutKusKaynak, konumKus, (Vector2){0,0}, 0.0f, RAYWHITE);
            }
        }

        //partikül
    for (int i = 0; i < MAX_PARTIKUL; i++) {
        if (partikuller[i].aktif) {
            DrawRectangle((int)partikuller[i].konum.x, (int)partikuller[i].konum.y, (int)partikuller[i].boyut, (int)partikuller[i].boyut, Fade(LIME, partikuller[i].omur)); 
        }
    }


        // Karakter
        Rectangle boyutSlime={0,0,slimeT.width,slimeT.height};
        if(slime.yerdeMi){
            Rectangle KonumSlime={slime.konum.x,slime.konum.y,slime.sekil.x,slime.sekil.y};
            DrawTexturePro(slimeT,boyutSlime,KonumSlime,(Vector2){0.0f},0.0f,RAYWHITE);
        }
        else{
            Rectangle KonumSlime={slime.konum.x + 2.5f, slime.konum.y - 10.0f, slime.sekil.x-5.0f, slime.sekil.y+10.0f};
            DrawTexturePro(slimeT,boyutSlime,KonumSlime,(Vector2){0.0f},0.0f,RAYWHITE);
        }

        // Arayüz
        int kutuGenislik = 140;
        int kutuYukseklik = 60;
        int kutuX = EKRAN_GENISLIK - kutuGenislik - 10; 
        int kutuY = 10; 
        DrawRectangle(kutuX, kutuY, kutuGenislik, kutuYukseklik, Fade(RAYWHITE, 0.9f));
        DrawRectangleLines(kutuX, kutuY, kutuGenislik, kutuYukseklik, LIME); 
        DrawText(TextFormat("Skor: %d", score), kutuX + 10, kutuY + 10, 20, BLUE);
        // Ekranda her zaman EN İYİ (1.) skoru gösterelim
        DrawText(TextFormat("Rekor: %d", liderlikTablosu[0]), kutuX + 10, kutuY + 35, 15, DARKGRAY);

        DrawText("SPACE: Zipla", 10, EKRAN_YUKSEKLIK - 40, 20, WHITE);
        DrawText("P: Durdur", 10, EKRAN_YUKSEKLIK - 20, 20, WHITE);
        
        if(muzikAcik) DrawText("Muzik: ACIK (V)", 10, EKRAN_YUKSEKLIK - 60, 20, GREEN);
        else DrawText("Muzik: KAPALI (V)", 10, EKRAN_YUKSEKLIK - 60, 20, RED);

        // Menüler
        if (oyunDuraklatildi) {
            DrawRectangle(0, 0, EKRAN_GENISLIK, EKRAN_YUKSEKLIK, Fade(BLACK, 0.4f));
            int pPanelGen = 300;
            int pPanelYuk = 100;
            int pX = EKRAN_GENISLIK/2 - pPanelGen/2;
            int pY = EKRAN_YUKSEKLIK/2 - pPanelYuk/2;
            DrawRectangle(pX, pY, pPanelGen, pPanelYuk, Fade(RAYWHITE, 0.9f));
            DrawRectangleLines(pX, pY, pPanelGen, pPanelYuk, DARKGRAY);
            DrawText("DURAKLATILDI", EKRAN_GENISLIK/2 - MeasureText("DURAKLATILDI", 30)/2, pY + 20, 30, DARKGRAY);
            DrawText("'P' ile Devam Et", EKRAN_GENISLIK/2 - MeasureText("'P' ile Devam Et", 20)/2, pY + 60, 20, GRAY);
        }
        else if (!oyunAktif && !oyunBitti) {
            int panelGenislik = 450;
            int panelYukseklik = 140;
            int panelX = EKRAN_GENISLIK / 2 - panelGenislik / 2;
            int panelY = EKRAN_YUKSEKLIK / 2 - panelYukseklik / 2;
            DrawRectangle(panelX, panelY, panelGenislik, panelYukseklik, Fade(RAYWHITE, 0.85f));
            DrawRectangleLines(panelX, panelY, panelGenislik, panelYukseklik, GRAY);
            const char* baslik = "SLIME RUNNER";
            const char* basla = "BASLAMAK ICIN 'SPACE' TUSUNA BASIN";
            DrawText(baslik, EKRAN_GENISLIK/2 - MeasureText(baslik, 40)/2, panelY + 30, 40, DARKGREEN);
            DrawText(basla, EKRAN_GENISLIK/2 - MeasureText(basla, 20)/2, panelY + 90, 20, DARKGRAY);
        }   
        else if (oyunBitti) {
            // --- YENİ LİDERLİK TABLOSU EKRANI ---
            int panelGenislik = 400;
            int panelYukseklik = 350; 
            int panelX = EKRAN_GENISLIK / 2 - panelGenislik / 2;
            int panelY = EKRAN_YUKSEKLIK / 2 - panelYukseklik / 2;

            DrawRectangle(panelX, panelY, panelGenislik, panelYukseklik, Fade(RAYWHITE, 0.95f));
            DrawRectangleLines(panelX, panelY, panelGenislik, panelYukseklik, GRAY); 
            
            DrawText("OYUN BITTI", panelX + 110, panelY + 20, 30, RED);
            DrawText(TextFormat("Senin Skorun: %d", score), panelX + 120, panelY + 60, 20, BLACK);
            
            DrawText("- LIDERLIK TABLOSU -", panelX + 100, panelY + 100, 20, DARKGREEN);
            
            // Tabloyu Çizdir
            for (int i = 0; i < SKOR_KAYIT_SAYISI; i++) {
                Color yaziRengi = DARKGRAY;
                // Şu anki skorumuz listedeyse onu altın rengi yapalım
                if (liderlikTablosu[i] == score && score != 0) yaziRengi = GOLD; 

                DrawText(TextFormat("%d.  .........  %d", i + 1, liderlikTablosu[i]), 
                         panelX + 120, 
                         panelY + 140 + (i * 30), // Her satırda 30px aşağı in
                         20, 
                         yaziRengi);
            }

            DrawText("Basa donmek icin 'R'", panelX + 100, panelY + 310, 20, LIGHTGRAY);
        }

        EndDrawing(); 
    }

    // --- TEMİZLİK ---
    UnloadTexture(arkaplan);
    UnloadTexture(toprak);
    UnloadTexture(slimeT);
    UnloadTexture(kutuk);
    UnloadTexture(kusT); 
    UnloadSound(ziplama);
    UnloadSound(yuksekSkor); 
    UnloadSound(kaybetme); 
    UnloadMusicStream(oyunmuzigi);

    CloseAudioDevice();
    CloseWindow(); 
    return 0;
}