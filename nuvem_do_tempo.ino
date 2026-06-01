#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>   // BMP085 library funciona com BMP180
#include <DHT.h>
#include <SPI.h>

// --- Pinos TFT ---
#define TFT_CS   D8
#define TFT_DC   D3
#define TFT_RST  D4
#define TFT_LED  D0

// --- DHT11 ---
#define DHTPIN  D5
#define DHTTYPE DHT11

// --- Cor customizada ---
#define DARKGREY 0x4208

// --- Objetos ---
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);

// --- Estados climáticos ---
enum WeatherState {
  SUNNY,
  PARTLY_CLOUDY,
  CLOUDY,
  RAIN,
  STORM,
  FOG,
  COLD,
  HOT
};

WeatherState lastState = (WeatherState)-1;

// --- Leituras ---
float tempDHT    = 0;
float humidity   = 0;
float tempBMP    = 0;
float pressure   = 0;

// -----------------------------------------------
// Zona do ícone: x 0–110, zona da msg: x 112–160
// -----------------------------------------------

void drawIcon(WeatherState state) {
  // Limpa zona do ícone
  tft.fillRect(0, 0, 111, 128, ST77XX_BLACK);

  int cx = 55;  // centro do ícone na zona esquerda

  switch (state) {

    case SUNNY: {
      // Sol
      tft.fillCircle(cx, 64, 22, ST77XX_YELLOW);
      // Raios
      for (int a = 0; a < 360; a += 45) {
        float rad = a * 3.14159 / 180.0;
        int x1 = cx + 26 * cos(rad);
        int y1 = 64 + 26 * sin(rad);
        int x2 = cx + 35 * cos(rad);
        int y2 = 64 + 35 * sin(rad);
        tft.drawLine(x1, y1, x2, y2, ST77XX_YELLOW);
        tft.drawLine(x1+1, y1, x2+1, y2, ST77XX_YELLOW);
      }
      break;
    }

    case PARTLY_CLOUDY: {
      // Sol menor atrás
      tft.fillCircle(cx + 12, 50, 18, ST77XX_YELLOW);
      for (int a = 0; a < 360; a += 60) {
        float rad = a * 3.14159 / 180.0;
        int x1 = (cx + 12) + 21 * cos(rad);
        int y1 = 50 + 21 * sin(rad);
        int x2 = (cx + 12) + 28 * cos(rad);
        int y2 = 50 + 28 * sin(rad);
        tft.drawLine(x1, y1, x2, y2, ST77XX_YELLOW);
      }
      // Nuvem na frente
      tft.fillCircle(cx - 8, 72, 16, ST77XX_WHITE);
      tft.fillCircle(cx + 8, 68, 18, ST77XX_WHITE);
      tft.fillCircle(cx + 20, 74, 13, ST77XX_WHITE);
      tft.fillRect(cx - 8, 72, 30, 18, ST77XX_WHITE);
      break;
    }

    case CLOUDY: {
      // Nuvem grande
      tft.fillCircle(cx - 10, 60, 18, ST77XX_WHITE);
      tft.fillCircle(cx + 8,  54, 22, ST77XX_WHITE);
      tft.fillCircle(cx + 24, 62, 16, ST77XX_WHITE);
      tft.fillRect(cx - 10, 60, 36, 22, ST77XX_WHITE);
      // Segunda nuvem (cinza, atrás)
      tft.fillCircle(cx - 16, 75, 14, DARKGREY);
      tft.fillCircle(cx - 2,  70, 17, DARKGREY);
      tft.fillCircle(cx + 12, 76, 12, DARKGREY);
      tft.fillRect(cx - 16, 75, 30, 18, DARKGREY);
      break;
    }

    case RAIN: {
      // Nuvem
      tft.fillCircle(cx - 8, 48, 16, ST77XX_WHITE);
      tft.fillCircle(cx + 8, 44, 19, ST77XX_WHITE);
      tft.fillCircle(cx + 22, 50, 14, ST77XX_WHITE);
      tft.fillRect(cx - 8, 48, 32, 18, ST77XX_WHITE);
      // Gotas de chuva
      for (int i = 0; i < 5; i++) {
        int rx = cx - 16 + (i * 12);
        tft.drawLine(rx, 75, rx - 4, 90, ST77XX_CYAN);
        tft.drawLine(rx+1, 75, rx - 3, 90, ST77XX_CYAN);
      }
      break;
    }

    case STORM: {
      // Nuvem escura
      tft.fillCircle(cx - 8, 40, 16, DARKGREY);
      tft.fillCircle(cx + 8, 35, 20, DARKGREY);
      tft.fillCircle(cx + 24, 42, 14, DARKGREY);
      tft.fillRect(cx - 8, 40, 34, 20, DARKGREY);
      // Raio
      tft.fillTriangle(cx + 2, 62, cx - 8, 78, cx + 4, 78, ST77XX_YELLOW);
      tft.fillTriangle(cx + 4, 78, cx - 4, 96, cx + 10, 82, ST77XX_YELLOW);
      // Chuva
      for (int i = 0; i < 4; i++) {
        int rx = cx - 20 + (i * 14);
        if (rx == cx) continue;
        tft.drawLine(rx, 65, rx - 3, 80, ST77XX_CYAN);
      }
      break;
    }

    case FOG: {
      // Linhas de neblina
      for (int i = 0; i < 5; i++) {
        int fy = 44 + (i * 14);
        tft.drawRoundRect(cx - 36, fy, 72, 6, 3, DARKGREY);
        tft.fillRoundRect(cx - 34, fy + 1, 68, 4, 3, DARKGREY);
      }
      break;
    }

    case COLD: {
      // Floco de neve
      // Cruz central
      tft.drawFastHLine(cx - 28, 64, 56, ST77XX_CYAN);
      tft.drawFastHLine(cx - 28, 65, 56, ST77XX_CYAN);
      tft.drawFastVLine(cx, 36, 56, ST77XX_CYAN);
      tft.drawFastVLine(cx + 1, 36, 56, ST77XX_CYAN);
      // Diagonais
      for (int d = -1; d <= 1; d++) {
        tft.drawLine(cx + d, 64, cx + 18 + d, 46, ST77XX_CYAN);
        tft.drawLine(cx + d, 64, cx - 18 + d, 46, ST77XX_CYAN);
        tft.drawLine(cx + d, 64, cx + 18 + d, 82, ST77XX_CYAN);
        tft.drawLine(cx + d, 64, cx - 18 + d, 82, ST77XX_CYAN);
      }
      // Centro
      tft.fillCircle(cx, 64, 5, ST77XX_WHITE);
      // Pontas
      int tips[6][2] = {{cx, 36}, {cx, 92}, {cx - 28, 64}, {cx + 28, 64},
                        {cx - 18, 46}, {cx + 18, 46}};
      for (auto& t : tips) tft.fillCircle(t[0], t[1], 3, ST77XX_WHITE);
      break;
    }

    case HOT: {
      // Termômetro
      tft.drawRoundRect(cx - 5, 28, 10, 60, 5, ST77XX_WHITE);
      tft.fillRoundRect(cx - 3, 70, 6, 20, 3, ST77XX_RED);  // mercúrio
      tft.fillCircle(cx, 96, 10, ST77XX_RED);
      tft.fillCircle(cx, 96, 7, ST77XX_RED);
      // Traços de temperatura
      for (int t = 0; t < 4; t++) {
        tft.drawFastHLine(cx + 6, 38 + (t * 10), 8, ST77XX_WHITE);
      }
      // Raios de calor
      tft.setTextColor(ST77XX_ORANGE);
      tft.setTextSize(1);
      tft.setCursor(cx + 18, 30);
      tft.print("~~~");
      tft.setCursor(cx + 18, 42);
      tft.print("~~~");
      break;
    }
  }
}

void drawMessage(WeatherState state) {
  // Limpa zona da mensagem
  tft.fillRect(112, 0, 48, 128, ST77XX_BLACK);

  // Linha divisória
  tft.drawFastVLine(111, 0, 128, DARKGREY);

  // Temperatura e umidade no topo
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(114, 6);
  tft.print((int)tempDHT);
  tft.print("C");

  tft.setTextColor(DARKGREY);
  tft.setCursor(114, 18);
  tft.print((int)humidity);
  tft.print("%UR");

  tft.setCursor(114, 30);
  tft.print((int)(pressure / 100));
  tft.print("hPa");

  // Linha separadora interna
  tft.drawFastHLine(112, 42, 48, DARKGREY);

  // Mensagem do estado
  tft.setTextSize(1);
  tft.setCursor(114, 50);

  switch (state) {
    case SUNNY:
      tft.setTextColor(ST77XX_YELLOW);
      tft.println("Dia");
      tft.setCursor(114, 62);
      tft.println("ensola-");
      tft.setCursor(114, 74);
      tft.println("rado!");
      break;
    case PARTLY_CLOUDY:
      tft.setTextColor(ST77XX_WHITE);
      tft.println("Parcial-");
      tft.setCursor(114, 62);
      tft.println("mente");
      tft.setCursor(114, 74);
      tft.println("nublado");
      break;
    case CLOUDY:
      tft.setTextColor(DARKGREY);
      tft.println("Nublado");
      tft.setCursor(114, 62);
      tft.println("por");
      tft.setCursor(114, 74);
      tft.println("aqui");
      break;
    case RAIN:
      tft.setTextColor(ST77XX_CYAN);
      tft.println("Chuva!");
      tft.setCursor(114, 62);
      tft.println("Leva o");
      tft.setCursor(114, 74);
      tft.println("guarda-");
      tft.setCursor(114, 86);
      tft.println("chuva");
      break;
    case STORM:
      tft.setTextColor(ST77XX_RED);
      tft.println("Temp-");
      tft.setCursor(114, 62);
      tft.println("estade!");
      tft.setCursor(114, 74);
      tft.println("Fique");
      tft.setCursor(114, 86);
      tft.println("em casa");
      break;
    case FOG:
      tft.setTextColor(DARKGREY);
      tft.println("Neblina");
      tft.setCursor(114, 62);
      tft.println("Dirija");
      tft.setCursor(114, 74);
      tft.println("devagar");
      break;
    case COLD:
      tft.setTextColor(ST77XX_CYAN);
      tft.println("Frio!");
      tft.setCursor(114, 62);
      tft.println("Agasalhe");
      tft.setCursor(114, 74);
      tft.println("-se bem");
      break;
    case HOT:
      tft.setTextColor(ST77XX_RED);
      tft.println("Muito");
      tft.setCursor(114, 62);
      tft.println("calor!");
      tft.setCursor(114, 74);
      tft.println("Beba");
      tft.setCursor(114, 86);
      tft.println("agua");
      break;
  }
}

// -----------------------------------------------
WeatherState getWeatherState() {
  if (tempDHT >= 38)                         return HOT;
  if (tempDHT <= 14)                         return COLD;
  if (humidity >= 90 && pressure < 100000)   return STORM;
  if (humidity >= 85)                        return RAIN;
  if (humidity >= 70 && humidity < 85)       return CLOUDY;
  if (humidity >= 60 && humidity < 70)       return PARTLY_CLOUDY;
  if (humidity >= 80 && tempDHT < 20)        return FOG;
  return SUNNY;
}

// -----------------------------------------------
void setup() {
  Serial.begin(115200);

  // Liga backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  // Inicia TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);  // Landscape: 160x128
  tft.fillScreen(ST77XX_BLACK);

  // Mensagem de boot
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.setCursor(10, 55);
  tft.print("Nuvem do Tempo...");

  // I2C
  Wire.begin(D2, D1);

  // BMP180
  if (!bmp.begin()) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(5, 55);
    tft.print("BMP180 nao encontrado");
    while (1);
  }

  // DHT11
  dht.begin();
  delay(2000);

  Serial.println("Nuvem do Tempo iniciada!");
}

// -----------------------------------------------
void loop() {
  tempDHT  = dht.readTemperature();
  humidity = dht.readHumidity();
  tempBMP  = bmp.readTemperature();
  pressure = bmp.readPressure();  // em Pa

  if (isnan(tempDHT) || isnan(humidity)) {
    Serial.println("Erro DHT11");
    delay(2000);
    return;
  }

  Serial.printf("DHT: %.1fC  UR: %.0f%%  BMP: %.1fC  P: %.0fPa\n",
                tempDHT, humidity, tempBMP, pressure);

  WeatherState currentState = getWeatherState();

  // Só redesenha se o estado mudou
  if (currentState != lastState) {
    drawIcon(currentState);
    drawMessage(currentState);
    lastState = currentState;
  }

  delay(3000);
}
