#include <FastLED.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include "time.h"
#define LED 289
#define EEPROM_SIZE 128
#define SSID_ADDR 0
#define PASS_ADDR 64
WebServer server(80);
DNSServer dnsServer;
CRGB leds[LED];
const int r_max = 16;
const int k_max = 16;
const int max_dots = 30;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 10800;
const int daylightOffset_sec = 0;
const byte DNS_PORT = 53;
bool isSetup = false;
bool isClock = false;
bool hasCredentials = false;
int dot = 15;
int clockColorR = 200;
int clockColorG = 255;
int clockColorB = 255;
float lines[max_dots][7];
String clockColorHex = "C8FFFF";
byte red, grn, blu;
byte symbolsEN[51][8] = {
  { B01110, B10001, B10001, B10001, B11111, B10001, B10001, B10001 },  //A
  { B00000, B00000, B00000, B01110, B00001, B01111, B10001, B01111 },  //a
  { B11110, B10001, B10001, B11110, B10001, B10001, B10001, B11110 },  //B
  { B10000, B10000, B10000, B11110, B10001, B10001, B10001, B11110 },  //b
  { B01110, B10001, B10000, B10000, B10000, B10000, B10001, B01110 },  //C
  { B00000, B00000, B00000, B01110, B10001, B10000, B10001, B01110 },  //c
  { B11110, B10001, B10001, B10001, B10001, B10001, B10001, B11110 },  //D
  { B00001, B00001, B00001, B01111, B10001, B10001, B10001, B01111 },  //d
  { B11111, B10000, B10000, B11110, B10000, B10000, B10000, B11111 },  //E
  { B00000, B00000, B00000, B01110, B10001, B11111, B10000, B01110 },  //e
  { B11111, B10000, B10000, B11110, B10000, B10000, B10000, B10000 },  //F
  { B00111, B01000, B01000, B11111, B01000, B01000, B01000, B01000 },  //f
  { B01110, B10001, B10000, B10111, B10001, B10001, B10001, B01110 },  //G
  { B01111, B10001, B10001, B10001, B01111, B00001, B10001, B01110 },  //g(3px down)
  { B10001, B10001, B10001, B11111, B10001, B10001, B10001, B10001 },  //H
  { B10000, B10000, B10000, B11110, B10001, B10001, B10001, B10001 },  //h
  { B11111, B00100, B00100, B00100, B00100, B00100, B00100, B11111 },  //I
  { B00100, B00000, B11100, B00100, B00100, B00100, B00100, B11111 },  //i
  { B00001, B00001, B00001, B00001, B00001, B00001, B10001, B01110 },  //J
  { B00001, B00000, B00011, B00001, B00001, B00001, B10001, B01110 },  //j(3px down)
  { B10001, B10010, B10100, B11000, B11000, B10100, B10010, B10001 },  //K
  { B10000, B10000, B10010, B10100, B11000, B10100, B10010, B10001 },  //k
  { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B11111 },  //L
  { B11100, B00100, B00100, B00100, B00100, B00100, B00100, B11111 },  //l
  { B10001, B11011, B11011, B10101, B10101, B10001, B10001, B10001 },  //M
  { B00000, B00000, B00000, B11110, B10101, B10101, B10101, B10101 },  //m
  { B10001, B11001, B11001, B10101, B10101, B10011, B10011, B10001 },  //N
  { B00000, B00000, B00000, B11110, B10001, B10001, B10001, B10001 },  //n
  { B01110, B10001, B10001, B10001, B10001, B10001, B10001, B01110 },  //O
  { B00000, B00000, B00000, B01110, B10001, B10001, B10001, B01110 },  //o
  { B11110, B10001, B10001, B10001, B11110, B10000, B10000, B10000 },  //P/p(3px down)
  { B01110, B10001, B10001, B10001, B10001, B10001, B10010, B01101 },  //Q
  { B01111, B10001, B10001, B10001, B01111, B00001, B00001, B00001 },  //q(3px down)
  { B11110, B10001, B10001, B10001, B11110, B10100, B10010, B10001 },  //R
  { B00000, B00000, B00000, B10110, B11001, B10000, B10000, B10000 },  //r
  { B01110, B10001, B10000, B01110, B00001, B00001, B10001, B01110 },  //S
  { B00000, B00000, B00000, B01111, B10000, B01110, B00001, B11110 },  //s
  { B11111, B00100, B00100, B00100, B00100, B00100, B00100, B00100 },  //T
  { B00000, B00000, B00000, B01000, B11110, B01000, B01001, B00110 },  //t
  { B10001, B10001, B10001, B10001, B10001, B10001, B10001, B01110 },  //U
  { B00000, B00000, B00000, B10001, B10001, B10001, B10011, B01101 },  //u
  { B10001, B10001, B10001, B10001, B10001, B11011, B01110, B00100 },  //V
  { B00000, B00000, B00000, B10001, B10001, B10001, B01010, B00100 },  //v
  { B10001, B10001, B10001, B10101, B10101, B11011, B11011, B10001 },  //W
  { B00000, B00000, B00000, B10001, B10001, B10101, B11011, B10001 },  //w
  { B10001, B10001, B01010, B00100, B00100, B01010, B10001, B10001 },  //X
  { B00000, B00000, B00000, B10001, B01010, B00100, B01010, B10001 },  //x
  { B10001, B10001, B10001, B01010, B00100, B00100, B00100, B00100 },  //Y
  { B10001, B10001, B10001, B10001, B01111, B00001, B10001, B01110 },  //y(3px down)
  { B11111, B00001, B00011, B00110, B01100, B11000, B10000, B11111 },  //Z
  { B00000, B00000, B00000, B11111, B00001, B01110, B10000, B11111 }   //z
};
byte symbolsSP[38][8]{
  { B01110, B10001, B10001, B10001, B10001, B10001, B10001, B01110 },  //zero
  { B00100, B01100, B10100, B00100, B00100, B00100, B00100, B11111 },  //one
  { B01110, B10001, B00001, B00010, B00100, B01000, B10000, B11111 },  //two
  { B01110, B10001, B00001, B01110, B00001, B00001, B10001, B01110 },  //three
  { B10001, B10001, B10001, B10001, B01111, B00001, B00001, B00001 },  //four
  { B11111, B10000, B11110, B00001, B00001, B00001, B10001, B01110 },  //five
  { B01110, B10001, B10000, B11110, B10001, B10001, B10001, B01110 },  //six
  { B11111, B00001, B00001, B00010, B00100, B00100, B00100, B00100 },  //seven
  { B01110, B10001, B10001, B01110, B10001, B10001, B10001, B01110 },  //eight
  { B01110, B10001, B10001, B10001, B01111, B00001, B10001, B01110 },  //nine
  { B10000, B10000, B10000, B10000, B10000, B10000, B00000, B10000 },  //!
  { B01110, B10001, B00001, B00010, B00100, B00100, B00000, B00100 },  //?
  { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000 },  //space
  { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B10000 },  //.
  { B00000, B00000, B00100, B00000, B00000, B00100, B00000, B00000 },  //:
  { B00000, B00000, B00000, B00000, B00000, B01000, B01000, B10000 },  //,
  { B00000, B00000, B00100, B00000, B00000, B00100, B00100, B01000 },  //;
  { B01100, B10000, B10000, B10000, B10000, B10000, B10000, B01100 },  //(
  { B00110, B00001, B00001, B00001, B00001, B00001, B00001, B00110 },  //)
  { B11100, B10000, B10000, B10000, B10000, B10000, B10000, B11100 },  //[
  { B00111, B00001, B00001, B00001, B00001, B00001, B00001, B00111 },  //]
  { B00000, B00100, B01000, B10000, B10000, B01000, B00100, B00000 },  //<
  { B00000, B00100, B00010, B00001, B00001, B00010, B00100, B00000 },  //>
  { B00100, B01000, B01000, B10000, B10000, B01000, B01000, B00100 },  //{
  { B00100, B00010, B00010, B00001, B00001, B00010, B00010, B00100 },  //}
  { B11001, B11010, B00010, B00100, B00100, B01000, B01011, B10011 },  //%
  { B01110, B10001, B10111, B10101, B10101, B10011, B10000, B01111 },  //@
  { B00000, B00000, B00000, B11111, B00000, B00000, B00000, B00000 },  //-
  { B00000, B00100, B00100, B11111, B00100, B00100, B00000, B00000 },  //+
  { B00000, B00000, B01010, B00100, B01010, B00000, B00000, B00000 },  //*
  { B00001, B00010, B00010, B00100, B00100, B01000, B01000, B10000 },  //"/"
  { B10000, B01000, B01000, B00100, B00100, B00010, B00010, B00001 },  //"\"
  { B00100, B01010, B10001, B00000, B00000, B00000, B00000, B00000 },  //^
  { B00000, B01010, B11111, B01010, B01010, B11111, B01010, B00000 },  //#
  { B01100, B10010, B10010, B01100, B10100, B10011, B10010, B01101 },  //&
  { B00000, B00000, B00000, B01010, B10101, B00000, B00000, B00000 },  //~
  { B00000, B00000, B11111, B00000, B11111, B00000, B00000, B00000 },  //=
  { B01010, B01010, B00000, B00000, B00000, B00000, B00000, B00000 }   //"
};
byte RED[k_max + 1][r_max + 1];
byte GRN[k_max + 1][r_max + 1];
byte BLU[k_max + 1][r_max + 1];
byte bufer[k_max + 1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
byte brPercent = 50;
unsigned long timer;
unsigned long ticking;
char savedSSID[64];
char savedPassword[64];
void printLocalTime();
void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  FastLED.addLeds<WS2812, 4, GRB>(leds, LED).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(percentToRange(brPercent));
  WiFi.softAPdisconnect(true);
  FastLED.show();
  loadCredentials();
  timer = millis();
  ticking = millis();
  if (hasCredentials) {
    unsigned long startTime = millis();
    WiFi.begin(savedSSID, savedPassword);
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
      RED[0][16] = random(0, 255);
      GRN[0][16] = random(0, 255);
      BLU[0][16] = random(0, 255);
      led();
      FastLED.show();
      delay(250);
      memset(RED, 0, sizeof(RED));
      memset(GRN, 0, sizeof(GRN));
      memset(BLU, 0, sizeof(BLU));
      led();
      FastLED.show();
      Serial.print(".");
      delay(250);
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Не удалось подключиться к WiFi!");
      Serial.println("Включение AP режима...");
      startAPMode();
      return;
    }
    memset(RED, 0, sizeof(RED));
    memset(GRN, 0, sizeof(GRN));
    memset(BLU, 0, sizeof(BLU));
    Serial.println("\nWiFi connected!");
    Serial.println("IP: " + WiFi.localIP().toString());
    WiFi.softAPdisconnect(true);
    dnsServer.stop();
    WiFi.mode(WIFI_STA);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if (!MDNS.begin("matrix")) {
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
    dotGen();
    server.on("/", handleRoot);
    server.on("/dotSubmit", handleDotSubmit);
    server.on("/modeSubmit", handleModeSubmit);
    server.on("/colorSubmit", handleColorSubmit);
    server.on("/brightnessSubmit", handleBrightenssSubmit);
    server.begin();
    MDNS.addService("http", "tcp", 80);
  } else {
    startAPMode();
  }
}
void dotGen() {
  for (int i = 0; i < dot; i++) {
    lines[i][0] = random(0, 16);
    lines[i][1] = random(0, 16);
    lines[i][2] = 1;
    lines[i][3] = 1;
    lines[i][4] = random(0, 255);
    lines[i][5] = random(0, 255);
    lines[i][6] = random(0, 255);
  }
}

void loop() {
  if (isSetup) {
    symbolDraw(symbolsEN[0], 9, 5);
    symbolDraw(symbolsEN[30], 3, 5);
  }
  switch (isClock) {
    case false:
      ball();
      break;
    case true:
      printLocalTime();
      break;
  }
  led();
  FastLED.show();
  server.handleClient();
}
void clr(int clear_step = 25) {
  // Serial.println();
  for (int i = 0; i < 17; i++) {
    //Serial.println();
    for (int j = 0; j < 17; j++) {
      if (RED[j][i] < clear_step)
        RED[j][i] = 0;
      else
        RED[j][i] -= clear_step;
      if (GRN[j][i] < clear_step)
        GRN[j][i] = 0;
      else
        GRN[j][i] -= clear_step;
      if (BLU[j][i] < clear_step)
        BLU[j][i] = 0;
      else
        BLU[j][i] -= clear_step;
      //Serial.print(RED[j][i]);
      // Serial.print(" ");
    }
  }
}
void printLocalTime() {
  struct tm timeinfo;
  char timeHour[3];
  char timeMinute[3];
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Не удалось получить время с NTP сервера.");
    return;
  }
  strftime(timeHour, sizeof(timeHour), "%H", &timeinfo);
  strftime(timeMinute, sizeof(timeMinute), "%M", &timeinfo);
  switch (timeHour[0]) {
    case '0':
      symbolDraw(symbolsSP[0], 12, 0);
      break;
    case '1':
      symbolDraw(symbolsSP[1], 12, 0);
      break;
    case '2':
      symbolDraw(symbolsSP[2], 12, 0);
      break;
  }
  switch (timeHour[1]) {
    case '0':
      symbolDraw(symbolsSP[0], 6, 0);
      break;
    case '1':
      symbolDraw(symbolsSP[1], 6, 0);
      break;
    case '2':
      symbolDraw(symbolsSP[2], 6, 0);
      break;
    case '3':
      symbolDraw(symbolsSP[3], 6, 0);
      break;
    case '4':
      symbolDraw(symbolsSP[4], 6, 0);
      break;
    case '5':
      symbolDraw(symbolsSP[5], 6, 0);
      break;
    case '6':
      symbolDraw(symbolsSP[6], 6, 0);
      break;
    case '7':
      symbolDraw(symbolsSP[7], 6, 0);
      break;
    case '8':
      symbolDraw(symbolsSP[8], 6, 0);
      break;
    case '9':
      symbolDraw(symbolsSP[9], 6, 0);
      break;
  }
  switch (timeMinute[0]) {
    case '0':
      symbolDraw(symbolsSP[0], 6, 9);
      break;
    case '1':
      symbolDraw(symbolsSP[1], 6, 9);
      break;
    case '2':
      symbolDraw(symbolsSP[2], 6, 9);
      break;
    case '3':
      symbolDraw(symbolsSP[3], 6, 9);
      break;
    case '4':
      symbolDraw(symbolsSP[4], 6, 9);
      break;
    case '5':
      symbolDraw(symbolsSP[5], 6, 9);
      break;
  }
  switch (timeMinute[1]) {
    case '0':
      symbolDraw(symbolsSP[0], 0, 9);
      break;
    case '1':
      symbolDraw(symbolsSP[1], 0, 9);
      break;
    case '2':
      symbolDraw(symbolsSP[2], 0, 9);
      break;
    case '3':
      symbolDraw(symbolsSP[3], 0, 9);
      break;
    case '4':
      symbolDraw(symbolsSP[4], 0, 9);
      break;
    case '5':
      symbolDraw(symbolsSP[5], 0, 9);
      break;
    case '6':
      symbolDraw(symbolsSP[6], 0, 9);
      break;
    case '7':
      symbolDraw(symbolsSP[7], 0, 9);
      break;
    case '8':
      symbolDraw(symbolsSP[8], 0, 9);
      break;
    case '9':
      symbolDraw(symbolsSP[9], 0, 9);
      break;
  }
}
void ball() {
  if (millis() - ticking >= 100) {
    clr(50);
    for (int i = 0; i < dot; i++) {
      lines[i][0] = lines[i][0] + lines[i][2];
      lines[i][1] = lines[i][1] + lines[i][3];
      //Serial.print(i); Serial.print(": "); Serial.print(lines[i][0]); Serial.print(","); Serial.println(lines[i][1]);
      if (lines[i][0] >= 16) {
        lines[i][2] = -1;
        lines[i][2] += randomXV(lines[i][2]);
      }
      if (lines[i][1] >= 16) {
        lines[i][3] = -lines[i][3];
        lines[i][2] += randomXV(lines[i][2]);
      }
      if (lines[i][0] <= 0) {
        lines[i][2] = 1;
        lines[i][2] += randomXV(lines[i][2]);
      }
      if (lines[i][1] <= 0) {
        lines[i][3] = fabs(lines[i][3]);
        lines[i][2] += randomXV(lines[i][2]);
      }
      RED[(int)round(lines[i][1])][(int)round(lines[i][0])] = (byte)round(lines[i][4]);
      GRN[(int)round(lines[i][1])][(int)round(lines[i][0])] = (byte)round(lines[i][5]);
      BLU[(int)round(lines[i][1])][(int)round(lines[i][0])] = (byte)round(lines[i][6]);
    }
    for (int i = 0; i < dot; i++) {
      for (int j = i + 1; j < dot; j++) {
        if (lines[i][0] == lines[j][0] && lines[i][1] == lines[j][1]) {
          float vel;
          /*
          Serial.print("Столкновение ");
          Serial.print(i);
          Serial.print(" c ");
          Serial.println(j);
          */
          for (int o = -1; o <= 1; o++) {
            RED[(int)round(lines[i][1])][(int)round(lines[i][0]) - o] = (byte)round((lines[i][4] + lines[j][4]) / 2);
            GRN[(int)round(lines[i][1])][(int)round(lines[i][0] - o)] = (byte)round((lines[i][5] + lines[j][5]) / 2);
            BLU[(int)round(lines[i][1])][(int)round(lines[i][0] - o)] = (byte)round((lines[i][6] + lines[j][6]) / 2);
          }
          for (int o = -1; o <= 1; o++) {
            RED[(int)round(lines[i][1]) - o][(int)round(lines[i][0])] = (byte)round((lines[i][4] + lines[j][4]) / 2);
            GRN[(int)round(lines[i][1]) - o][(int)round(lines[i][0])] = (byte)round((lines[i][5] + lines[j][5]) / 2);
            BLU[(int)round(lines[i][1]) - o][(int)round(lines[i][0])] = (byte)round((lines[i][6] + lines[j][6]) / 2);
          }
          /*
            Serial.print(lines[i][2]);
            Serial.print(",");
            Serial.print(lines[i][3]);
            Serial.print(" ");
            Serial.print(lines[j][2]);
            Serial.print(",");
            Serial.print(lines[j][3]);
            Serial.print(" | ");
          */
          vel = lines[i][2];
          lines[i][2] = lines[j][2];
          lines[j][2] = vel;
          vel = lines[i][3];
          lines[i][3] = lines[j][3];
          lines[j][3] = vel;
          /*
            Serial.print(lines[i][2]);
            Serial.print(",");
            Serial.print(lines[i][3]);
            Serial.print(" ");
            Serial.print(lines[j][2]);
            Serial.print(",");
            Serial.println(lines[j][3]);
          */
        }
      }
    }
    ticking = millis();
  }
}
void led() {
  if (millis() - timer >= 100) {
    for (int i = 0; i < LED; i++) {
      leds[i] = 0;
    }
    for (int k = 0; k <= k_max; k++) {
      for (int r = 0; r <= r_max; r++) {
        if (RED[k][r] != 0) {
          int rotated_k = r;
          int rotated_r = k_max - k;
          if (rotated_k % 2 == 0) {
            leds[(k_max - rotated_k) * (r_max + 1) + (r_max - rotated_r)].r = RED[k][r];
          } else {
            leds[(k_max - rotated_k) * (r_max + 1) + rotated_r].r = RED[k][r];
          }
        }
      }
    }
    for (int k = 0; k <= k_max; k++) {
      for (int r = 0; r <= r_max; r++) {
        if (GRN[k][r] != 0) {
          int rotated_k = r;
          int rotated_r = k_max - k;
          if (rotated_k % 2 == 0) {
            leds[(k_max - rotated_k) * (r_max + 1) + (r_max - rotated_r)].g = GRN[k][r];
          } else {
            leds[(k_max - rotated_k) * (r_max + 1) + rotated_r].g = GRN[k][r];
          }
        }
      }
    }
    for (int k = 0; k <= k_max; k++) {
      for (int r = 0; r <= r_max; r++) {
        if (BLU[k][r] != 0) {
          int rotated_k = r;
          int rotated_r = k_max - k;
          if (rotated_k % 2 == 0) {
            leds[(k_max - rotated_k) * (r_max + 1) + (r_max - rotated_r)].b = BLU[k][r];
          } else {
            leds[(k_max - rotated_k) * (r_max + 1) + rotated_r].b = BLU[k][r];
          }
        }
      }
    }
    timer = millis();
  }
}
void startAPMode() {
  isSetup = true;
  WiFi.softAP("PixelStorm", "11111111");
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  server.on("/", HTTP_GET, handleAP);
  server.on("/wifi", HTTP_POST, handleWifiSave);
  server.onNotFound(handleNotFound);
  server.begin();
  clockColorR = 255;
  clockColorG = 0;
  clockColorB = 0;
  Serial.println("AP режим запущен!");
}
float randomXV(float n) {
  return (random(0, 100) - 50) / 100.0;
}
void symbolDraw(byte symbol[], byte offsetX, byte offsetY) {
  int valueR, valueG, valueB;
  for (int section = 0; section < 8; section++) {
    for (int i = 0; i < 5; i++) {
      switch (bitRead(symbol[section], i)) {
        case 0:
          valueR = 0;
          valueG = 0;
          valueB = 0;
          break;
        case 1:
          valueR = clockColorR;
          valueG = clockColorG;
          valueB = clockColorB;
          break;
      }
      RED[section + offsetY][i + offsetX] = valueR;
      GRN[section + offsetY][i + offsetX] = valueG;
      BLU[section + offsetY][i + offsetX] = valueB;
    }
  }
}
void textScroll(String text, int startPosX, int startPosY, float vx) {
  //H5 W0
}
byte percentToRange(int percent) {
  return map(percent, 0, 100, 5, 255);
}
void loadCredentials() {
  String ssid = EEPROM.readString(SSID_ADDR);
  String password = EEPROM.readString(PASS_ADDR);

  if (ssid.length() > 0 && ssid.length() < sizeof(savedSSID)) {
    isSetup = false;
    ssid.toCharArray(savedSSID, sizeof(savedSSID));
    password.toCharArray(savedPassword, sizeof(savedPassword));
    hasCredentials = true;
  } else {
    hasCredentials = false;
    isSetup = true;
    startAPMode();
  }
}
void saveCredentials(String ssid, String password) {
  EEPROM.writeString(SSID_ADDR, ssid);
  EEPROM.writeString(PASS_ADDR, password);
  EEPROM.commit();
}
void handleDotSubmit() {
  if (server.hasArg("dotsSelect")) {
    String number = server.arg("dotsSelect");
    dot = number.toInt();
    server.sendHeader("Location", "/");
    server.send(303);
  }
  dotGen();
}
void handleBrightenssSubmit() {
  if (server.hasArg("brightnessRange")) {
    String number = server.arg("brightnessRange");
    brPercent = number.toInt();
    server.sendHeader("Location", "/");
    server.send(303);
  }
  FastLED.setBrightness(percentToRange(brPercent));
}
void handleModeSubmit() {
  memset(RED, 0, sizeof(RED));
  memset(GRN, 0, sizeof(GRN));
  memset(BLU, 0, sizeof(BLU));
  isClock = !isClock;
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleColorSubmit() {
  if (server.hasArg("clockColor")) {
    String colorHex = server.arg("clockColor");
    if (colorHex.startsWith("#")) {
      colorHex = colorHex.substring(1);
    }
    if (colorHex.length() >= 6) {
      long colorValue = strtol(colorHex.c_str(), NULL, 16);
      clockColorR = (colorValue >> 16) & 0xFF;
      clockColorG = (colorValue >> 8) & 0xFF;
      clockColorB = colorValue & 0xFF;
      clockColorHex = colorHex;
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>PixelStorm control</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; }";
  html += "h1 { color: #333; }";
  html += "form { margin: 20px 0; }";
  html += "select, button { padding: 8px; margin: 5px 0; }";
  html += "</style>";
  html += "<script>";
  html += "function updateSliderValue(sliderId, valueId) {";
  html += "document.getElementById(valueId).textContent = document.getElementById(sliderId).value + '%';";
  html += "}";
  html += "</script>";
  html += "</head>";
  html += "<body>";
  html += "<h1>PixelStorm control</h1>";
  if (isClock == false) {
    html += "<form action=\"/dotSubmit\" method=\"POST\">";
    html += "<label for=\"dotText\">Количество точек:</label>";
    html += "<select name=\"dotsSelect\" id=\"dotsSelect\">";
    for (int i = 1; i <= 30; i++) {
      html += "<option value=\"" + String(i) + "\"" + (i == dot ? " selected" : "") + ">" + String(i) + "</option>";
    }
    html += "</select>";
    html += "<input type=\"submit\" value=\"ok\">";
    html += "</form>";
  } else {
    html += "<form action=\"/colorSubmit\" method=\"POST\">";
    html += "<label for=\"clockColor\">Цвет часов:</label>";
    html += "<input type=\"color\" id=\"clockColor\" name=\"clockColor\" value=\"#";
    if (clockColorHex.length() >= 6) {
      html += clockColorHex;
    } else {
      html += "C8FFFF";
    }
    html += "\">";
    html += "<input type=\"submit\" value=\"ok\">";
    html += "</form>";
  }
  html += "<form action=\"/brightnessSubmit\" method=\"POST\">";
  html += "<label for=\"dotsSlider\">Яркость:</label>";
  html += "<div class=\"slider-container\">";
  html += "<input type=\"range\" id=\"brightnessRange\" name=\"brightnessRange\" min=\"1\" max=\"100\" value=\"" + String(brPercent) + "\" oninput=\"updateSliderValue('brightnessRange', 'brValue')\">";
  html += "<span id=\"brValue\" class=\"slider-value\">" + String(brPercent) + "%</span>";
  html += "</div>";
  html += "<input type=\"submit\" value=\"ok\">";
  html += "</form>";
  html += "<form action=\"/modeSubmit\" method=\"POST\">";
  html += "<button type=/submit>";
  html += isClock ? "Режим точек" : "Режим часов";
  html += "</button>";
  html += "</form>";
  html += "</body>";
  html += "</html>";
  server.send(200, "text/html; charset=utf-8", html);
}
void handleAP() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>WiFi Setup</title>
  <style>
  body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; }
  h1 { color: #333; }
  form { margin: 20px 0; }
  select, button { padding: 8px; margin: 5px 0; }
  </style>
</head>
<body>
  <h1>Настройка WiFi</h1>
  <form method="POST" action="/wifi">
    <input type="text" name="ssid" placeholder="SSID" required><br>
    <input type="password" name="pass" placeholder="Password"><br>
    <button type="submit">Сохранить</button>
  </form>
</body>
</html>
)=====";

  server.send(200, "text/html; charset=utf-8", html);
}

void handleWifiSave() {
  String newSsid = server.arg("ssid");
  String newPass = server.arg("pass");
  if (newSsid.length() > 0) {
    saveCredentials(newSsid, newPass);
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="refresh" content="5;url=/">
  <title>WiFi Setup</title>
  <style>
  body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; }
  h1 { color: #333; }
  form { margin: 20px 0; }
  select, button { padding: 8px; margin: 5px 0; }
  </style>
</head>
<body>
  <h1>Настройки сохранены! Перезагрузка...</h1>
</body>
</html>
)=====";
    server.send(200, "text/html; charset=utf-8", html);
    delay(1000);
    ESP.restart();
    WiFi.softAPdisconnect(true);
  } else {
    server.send(400, "text/html; charset=utf-8", "<h1>Ошибка: SSID не может быть пустым!</h1>");
  }
}

void handleNotFound() {
  server.sendHeader("Location", "http://" + server.client().localIP().toString(), true);
  server.send(302, "text/plain", "");
}