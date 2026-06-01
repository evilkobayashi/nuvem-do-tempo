# 🌦 Nuvem do Tempo

Estação meteorológica embarcada com ícones climáticos desenhados e mensagens em tempo real num display TFT 1.8".

---

## Sobre o projeto

A Nuvem do Tempo combina leituras de temperatura, umidade e pressão atmosférica para determinar o estado climático atual e exibi-lo visualmente. O display é dividido em duas zonas: o ícone do clima à esquerda e os dados + mensagem à direita. O sistema reconhece 8 condições climáticas distintas e só atualiza o display quando o estado muda.

---

## Demonstração

| Estado | Condição detectada | Ícone |
|--------|-------------------|-------|
| ☀️ Sunny | Umidade < 60%, temp normal | Sol com raios |
| ⛅ Partly Cloudy | Umidade 60–70% | Sol + nuvem |
| ☁️ Cloudy | Umidade 70–85% | Nuvens sobrepostas |
| 🌧 Rain | Umidade ≥ 85% | Nuvem com gotas |
| ⛈ Storm | Umidade ≥ 90% e pressão baixa | Nuvem escura + raio |
| 🌫 Fog | Umidade ≥ 80% e temp < 20°C | Linhas de neblina |
| ❄️ Cold | Temperatura ≤ 14°C | Floco de neve |
| 🌡 Hot | Temperatura ≥ 38°C | Termômetro vermelho |

---

## Hardware

| Componente | Função | Pino |
|---|---|---|
| NodeMCU ESP8266 | Microcontrolador | — |
| TFT 1.8" ST7735 (8 pinos) | Display | CS=D8, DC=D3, RST=D4, LED=D0 |
| DHT11 | Temperatura e umidade do ar | D5 |
| BMP180 | Pressão atmosférica e temperatura | I2C: SDA=D2, SCL=D1 |

---

## Esquema de ligação

```
NodeMCU         TFT 1.8"
D8      ──────  CS
D3      ──────  DC (A0)
D4      ──────  RST
D0      ──────  LED (backlight)
D7      ──────  SDA (MOSI)
D5      ──────  SCK (clock do SPI — ver nota abaixo)
3.3V    ──────  VCC
GND     ──────  GND

NodeMCU         DHT11
D5      ──────  DATA
3.3V    ──────  VCC
GND     ──────  GND

NodeMCU         BMP180 (I2C)
D2      ──────  SDA
D1      ──────  SCL
3.3V    ──────  VCC
GND     ──────  GND
```

> **Atenção:** se o DHT11 estiver no D5 e conflitar com o SCK do SPI, mova o DHT11 para D6.

---

## Bibliotecas necessárias

Instale pela Arduino IDE (Sketch → Include Library → Manage Libraries):

```
Adafruit ST7735 and ST7789 Library
Adafruit GFX Library
Adafruit BMP085 Unified           ← use esta para o BMP180
DHT sensor library (by Adafruit)
```

> O BMP180 é totalmente compatível com a biblioteca `Adafruit BMP085`. Não é necessária uma biblioteca específica para BMP180.

---

## Layout do display

```
┌─────────────────────────────────┐  128px
│                        │  28°C  │
│                        │  72%UR │
│      ÍCONE             │ 1013hPa│
│    (0 – 110px)         │────────│
│                        │ Chuva! │
│                        │ Leva o │
│                        │ guarda │
└────────────────────────┴────────┘
         160px
```

---

## Lógica de estados

```cpp
HOT           →  temp >= 38°C
COLD          →  temp <= 14°C
STORM         →  umidade >= 90% e pressão < 100000 Pa
RAIN          →  umidade >= 85%
CLOUDY        →  umidade >= 70%
PARTLY_CLOUDY →  umidade >= 60%
FOG           →  umidade >= 80% e temp < 20°C
SUNNY         →  demais casos
```

---

## Detalhes técnicos importantes

```cpp
// A biblioteca ST7735 não possui ST77XX_DARKGREY — defina manualmente:
#define DARKGREY 0x4208

// O backlight precisa ser ativado antes do initR():
pinMode(TFT_LED, OUTPUT);
digitalWrite(TFT_LED, HIGH);

// Orientação landscape (160x128px):
tft.setRotation(1);

// BMP180 usa a biblioteca do BMP085 — funciona normalmente:
Adafruit_BMP085 bmp;
bmp.begin();

// Pressão retornada em Pascal — converta para hPa dividindo por 100:
float pressureHpa = bmp.readPressure() / 100.0;
```

---

## Como usar

1. Clone este repositório
2. Abra `nuvem_do_tempo.ino` na Arduino IDE
3. Instale as bibliotecas listadas acima
4. Selecione a placa **NodeMCU 1.0 (ESP-12E Module)**
5. Faça as ligações conforme o esquema
6. Faça o upload e abra o Serial Monitor (115200 baud) para acompanhar as leituras

---

## Stack

![skills](https://skillicons.dev/icons?i=cpp,arduino&theme=dark)

`ESP8266` `C++` `I2C` `SPI` `DHT11` `BMP180` `Embedded Systems`

---

## Autor

**André Veiga** — [@evilkobayashi](https://github.com/evilkobayashi)  
Professor de robótica e desenvolvimento em Queimados, RJ.  
📧 andreveiga.eng@gmail.com
