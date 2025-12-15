#include "raylib.h"

// Sabitler: Ekran boyutlarını burada belirliyoruz.
#define EKRAN_YUKSEKLIK 600
#define EKRAN_GENISLIK 1000

// --- YAPI (STRUCT) TANIMLARI ---

// Slime (Karakter) özelliklerini tutan yapı
typedef struct {
    Color renk;      // Karakterin rengi
    Vector2 konum;   // Ekrandaki X ve Y koordinatı
    Vector2 sekil;   // Genişlik ve Yükseklik bilgisi
    Vector2 hiz;     // Hareket hızı (Yatay ve Dikey)
    bool yerdeMi;    // Karakter yere basıyor mu? (Zıplama kontrolü için)
} Slime;

// Zemin (Masa) özelliklerini tutan yapı
typedef struct {
    Vector2 konum;   // Masanın başladığı yer
    Color renk;      // Masanın rengi
    Vector2 boyut;   // Masanın genişliği ve yüksekliği
} Masa;

// Düşman (Rakip/Engel) özelliklerini tutan yapı
typedef struct {
    Vector2 konum;   // Engelin konumu
    Vector2 boyut;   // Engelin boyutu
    Color renk;      // Engelin rengi
    bool aktif;      // Engel şu an ekranda aktif mi?
    bool gecildi;    // Oyuncu bu engeli aştı mı? (Skor kontrolü)
} Rakip;

// --- GLOBAL DEĞİŞKENLER ---
// Bu değişkenlere programın her yerinden erişilebilir.
Slime slime;
Masa masa;
Rakip rakipler[5];      // Aynı anda ekranda olabilecek maksimum engel sayısı
int score = 0;          // Oyuncunun puanı
float oyunHizi = -3.0f; // Oyunun kayma hızı (Sola doğru gittiği için negatif)
bool oyunAktif = false; // Oyunun oynanıp oynanmadığını kontrol eder
bool oyunBitti = false; // Karakterin yanıp yanmadığını kontrol eder

// --- OYUNU BAŞLATMA VE SIFIRLAMA ---
// Oyun ilk açıldığında veya 'R' tuşuna basıldığında bu fonksiyon çalışır.
void OyunuSifirla() {
    // 1. Masanın (Zeminin) ayarlanması
    masa.konum = (Vector2){0, EKRAN_YUKSEKLIK / 3.0f * 2.0f}; // Ekranın alt kısmına yerleştir
    masa.boyut = (Vector2){EKRAN_GENISLIK, 10}; // Ekran boyunca uzanan ince bir çizgi
    masa.renk = BROWN;

    // 2. Slime (Karakter) ayarları
    slime.renk = GREEN;
    slime.sekil = (Vector2){30, 30}; // 30x30 piksel boyutunda
    // Slime'ın konumu: Masanın Y konumundan slime'ın boyunu çıkararak tam üstüne oturtuyoruz.
    slime.konum = (Vector2){100, masa.konum.y - slime.sekil.y}; 
    slime.hiz = (Vector2){0, 0};
    slime.yerdeMi = true; // Yerde başlıyor olarak işaretle

    // 3. Rakiplerin (Engellerin) hazırlanması
    oyunHizi = -3.0f;       // Hızı varsayılan değere döndür
    int x = EKRAN_GENISLIK; // Engeller ekranın sağından gelmeye başlasın
    
    for (int i = 0; i < 5; i++) {
        int gap = GetRandomValue(300, 450); // Engeller arası rastgele mesafe belirle
        x += gap; // Bir sonraki engelin konumunu hesapla
        
        // Engelin boyutunu ve yerini ayarla
        rakipler[i].boyut = (Vector2){20, (float)GetRandomValue(50, 80)};
        // Engeli zemine (masaya) yapışık olacak şekilde konumlandır
        rakipler[i].konum = (Vector2){(float)x, masa.konum.y - rakipler[i].boyut.y}; 
        rakipler[i].renk = BLACK;
        rakipler[i].aktif = true;
        rakipler[i].gecildi = false;
    }

    score = 0;          // Skoru sıfırla
    oyunBitti = false;  // Yanma durumunu kaldır
}

// --- KARAKTER HAREKET MANTIĞI ---
void SlimeHareket() {
    // Yerçekimi uygulaması: Slime sürekli aşağı doğru hızlanır
    slime.konum.y += slime.hiz.y;
    slime.hiz.y += 0.6f; 

    // Zemin ile çarpışma kontrolü
    // Eğer slime masanın seviyesinin altına indiyse...
    if (slime.konum.y + slime.sekil.y >= masa.konum.y) {
        slime.konum.y = masa.konum.y - slime.sekil.y; // Onu tekrar masanın üstüne koy
        slime.hiz.y = 0; // Düşme hızını sıfırla
        slime.yerdeMi = true; // Yere bastığını belirt
    } else {
        slime.yerdeMi = false; // Havada olduğunu belirt
    }

    // Zıplama kontrolü
    // Space tuşuna basıldıysa VE karakter yerdeyse zıpla
    if (IsKeyPressed(KEY_SPACE) && slime.yerdeMi) {
        slime.hiz.y = -12.0f; // Yukarı doğru negatif hız ver
        slime.yerdeMi = false;
    }
}

// --- ENGEL YÖNETİMİ ---
void RakipYonetimi() {
    for (int i = 0; i < 5; i++) {
        if (rakipler[i].aktif) {
            // Engeli sola doğru kaydır
            rakipler[i].konum.x += oyunHizi;

            // Eğer engel ekranın solundan tamamen çıktıysa...
            if (rakipler[i].konum.x + rakipler[i].boyut.x < 0) {
                // Ekrandaki en sağdaki (en uzaktaki) engeli bul
                float maxX = 0;
                for (int j = 0; j < 5; j++) {
                    if (rakipler[j].konum.x > maxX) {
                        maxX = rakipler[j].konum.x;
                    }
                }

                // Ekrandan çıkan bu engeli alıp en arkaya (sağa) taşı
                int gap = GetRandomValue(300, 500); // Rastgele yeni bir aralık ver
                rakipler[i].konum.x = maxX + gap;
                
                // Engel her döndüğünde boyunu rastgele değiştir
                rakipler[i].boyut.y = GetRandomValue(50, 80);
                rakipler[i].konum.y = masa.konum.y - rakipler[i].boyut.y;
                rakipler[i].gecildi = false; // Tekrar puan kazanılabilir hale getir
            }
        }
    }
}

// --- ANA PROGRAM ---
int main() {
    // Raylib penceresini başlat
    InitWindow(EKRAN_GENISLIK, EKRAN_YUKSEKLIK, "Slime Runner - Raylib");
    SetTargetFPS(60); // Oyunu 60 FPS'e sabitle (Akıcılık için)

    OyunuSifirla(); // Değişkenleri ilk değerlerine ayarla

    // Pencere kapatılmadığı sürece döngüye gir
    while (!WindowShouldClose()) {
        
        // --- GÜNCELLEME (UPDATE) ---
        // Oyun aktifse ve bitmemişse oyun mantığını çalıştır
        if (oyunAktif && !oyunBitti) {
            SlimeHareket();   // Karakter fiziği
            RakipYonetimi();  // Engel hareketi

            // Çarpışma kontrolü için dikdörtgenleri oluştur
            Rectangle slimeRec = {slime.konum.x, slime.konum.y, slime.sekil.x, slime.sekil.y};
            
            for (int i = 0; i < 5; i++) {
                Rectangle rakipRec = {rakipler[i].konum.x, rakipler[i].konum.y, rakipler[i].boyut.x, rakipler[i].boyut.y};

                // Slime bir engele çarptı mı?
                if (CheckCollisionRecs(slimeRec, rakipRec)) {
                    oyunBitti = true; // Oyunu bitir
                }

                // Slime engeli başarıyla geçti mi?
                if (!rakipler[i].gecildi && slime.konum.x > rakipler[i].konum.x + rakipler[i].boyut.x) {
                    score++; // Puanı artır
                    rakipler[i].gecildi = true; // Bu engeli işaretle (tekrar puan vermesin)
                    
                    // Zorluk ayarı: Her 10 puanda bir oyunu hızlandır
                    if (score % 10 == 0) oyunHizi -= 1.0f;
                }
            }
        } 
        else {
            // Oyun bekleme modundaysa veya bitmişse tuşları kontrol et
            
            // Başlamak için SPACE tuşu kontrolü
            if (!oyunBitti && IsKeyPressed(KEY_SPACE)) {
                oyunAktif = true;
            }
            
            // Kaybettikten sonra R tuşuna basılırsa ana menüye dön
            if (oyunBitti && IsKeyPressed(KEY_R)) {
                OyunuSifirla();     // Her şeyi sıfırla
                oyunAktif = false;  // Oyunu hemen başlatma, menüde beklet
            }
        }

        // --- ÇİZİM (DRAW) ---
        BeginDrawing();
        ClearBackground(RAYWHITE); // Arka planı temizle

        // Zemini çiz
        DrawRectangleV(masa.konum, masa.boyut, masa.renk);

        // Tüm engelleri çiz
        for (int i = 0; i < 5; i++) {
            DrawRectangleV(rakipler[i].konum, rakipler[i].boyut, rakipler[i].renk);
        }

        // Karakteri çiz
        DrawRectangleV(slime.konum, slime.sekil, slime.renk);
        
        // Skoru ekrana yaz
        DrawText(TextFormat("Skor: %d", score), 20, 20, 20, BLUE);

        // Başlangıç Ekranı Yazısı
        if (!oyunAktif && !oyunBitti) {
            DrawText("BASLAMAK ICIN 'SPACE' TUSUNA BASIN", EKRAN_GENISLIK/2 - 200, EKRAN_YUKSEKLIK/2 - 50, 20, DARKGRAY);
        }
        // Oyun Bitti Ekranı Yazıları
        else if (oyunBitti) {
            DrawText("KAYBETTINIZ!", EKRAN_GENISLIK/2 - 80, EKRAN_YUKSEKLIK/2 - 60, 30, RED);
            DrawText(TextFormat("Son Skor: %d", score), EKRAN_GENISLIK/2 - 60, EKRAN_YUKSEKLIK/2 - 20, 20, BLACK);
            DrawText("Basa donmek icin 'R' tusuna basin", EKRAN_GENISLIK/2 - 180, EKRAN_YUKSEKLIK/2 + 20, 20, DARKGRAY);
        }

        EndDrawing(); // Çizimi bitir
    }

    CloseWindow(); // Pencereyi kapat ve kaynakları serbest bırak
    return 0;
}