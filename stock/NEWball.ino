#include <FastLED.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include "time.h"
#include <Arduino.h>
#define LED 289
#define EEPROM_SIZE 128
#define SSID_ADDR 0
#define PASS_ADDR 64
WebServer server(80);
DNSServer dnsServer;
CRGB leds[LED];
SemaphoreHandle_t xMutex = NULL;
SemaphoreHandle_t xLinesMutex = NULL;
const int r_max = 16;
const int k_max = 16;
const int max_dots = 30;
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 10800;
const int daylightOffset_sec = 0;
const byte DNS_PORT = 53;
bool isSetup = false;
bool ledsNeedUpdate = false;
int mode = 0;
bool hasCredentials = false;
int dot = 15;
int clockColorR = 200;
int clockColorG = 255;
int clockColorB = 255;
int textColorR = 200;
int textColorG = 255;
int textColorB = 255;
float lines[max_dots][7];
String clockColorHex = "C8FFFF";
String textColorHex = "C8FFFF";
byte red, grn, blu;
int scrollOffset = 0;
unsigned long scrollTimer = 0;
String currentText = "";
String runningText = "";
int textPosition = 0;
int charColumn = 0;
byte symbols[157][8] = {
  { B01110, B10001, B10001, B10001, B11111, B10001, B10001, B10001 },  //A
  { B11110, B10001, B10001, B11110, B10001, B10001, B10001, B11110 },  //B
  { B01110, B10001, B10000, B10000, B10000, B10000, B10001, B01110 },  //C
  { B11110, B10001, B10001, B10001, B10001, B10001, B10001, B11110 },  //D
  { B11111, B10000, B10000, B11110, B10000, B10000, B10000, B11111 },  //E
  { B11111, B10000, B10000, B11110, B10000, B10000, B10000, B10000 },  //F
  { B01110, B10001, B10000, B10111, B10001, B10001, B10001, B01110 },  //G
  { B10001, B10001, B10001, B11111, B10001, B10001, B10001, B10001 },  //H
  { B11111, B00100, B00100, B00100, B00100, B00100, B00100, B11111 },  //I
  { B00001, B00001, B00001, B00001, B00001, B00001, B10001, B01110 },  //J
  { B10001, B10010, B10100, B11000, B11000, B10100, B10010, B10001 },  //K
  { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B11111 },  //L
  { B10001, B11011, B11011, B10101, B10101, B10001, B10001, B10001 },  //M
  { B10001, B11001, B11001, B10101, B10101, B10011, B10011, B10001 },  //N
  { B01110, B10001, B10001, B10001, B10001, B10001, B10001, B01110 },  //O
  { B11110, B10001, B10001, B10001, B11110, B10000, B10000, B10000 },  //P
  { B01110, B10001, B10001, B10001, B10001, B10001, B10010, B01101 },  //Q
  { B11110, B10001, B10001, B10001, B11110, B10001, B10001, B10001 },  //R
  { B01110, B10001, B10000, B01110, B00001, B00001, B10001, B01110 },  //S
  { B11111, B00100, B00100, B00100, B00100, B00100, B00100, B00100 },  //T
  { B10001, B10001, B10001, B10001, B10001, B10001, B10001, B01110 },  //U
  { B10001, B10001, B10001, B10001, B10001, B11011, B01110, B00100 },  //V
  { B10001, B10001, B10001, B10101, B10101, B11011, B11011, B10001 },  //W
  { B10001, B10001, B01010, B00100, B00100, B01010, B10001, B10001 },  //X
  { B10001, B10001, B10001, B01010, B00100, B00100, B00100, B00100 },  //Y
  { B11111, B00001, B00011, B00110, B01100, B11000, B10000, B11111 },  //Z
  { B00000, B00000, B00000, B01110, B00001, B01111, B10001, B01111 },  //a
  { B10000, B10000, B10000, B11110, B10001, B10001, B10001, B11110 },  //b
  { B00000, B00000, B00000, B01110, B10001, B10000, B10001, B01110 },  //c
  { B00001, B00001, B00001, B01111, B10001, B10001, B10001, B01111 },  //d
  { B00000, B00000, B00000, B01110, B10001, B11111, B10000, B01110 },  //e
  { B00111, B01000, B01000, B11111, B01000, B01000, B01000, B01000 },  //f
  { B01111, B10001, B10001, B10001, B01111, B00001, B10001, B01110 },  //g(3px down)
  { B10000, B10000, B10000, B11110, B10001, B10001, B10001, B10001 },  //h
  { B00100, B00000, B11100, B00100, B00100, B00100, B00100, B11111 },  //i
  { B00001, B00000, B00011, B00001, B00001, B00001, B10001, B01110 },  //j(3px down)
  { B10000, B10000, B10010, B10100, B11000, B10100, B10010, B10001 },  //k
  { B11100, B00100, B00100, B00100, B00100, B00100, B00100, B11111 },  //l
  { B00000, B00000, B00000, B11110, B10101, B10101, B10101, B10101 },  //m
  { B00000, B00000, B00000, B11110, B10001, B10001, B10001, B10001 },  //n
  { B00000, B00000, B00000, B01110, B10001, B10001, B10001, B01110 },  //o
  { B11110, B10001, B10001, B10001, B11110, B10000, B10000, B10000 },  //p(3px down)
  { B01111, B10001, B10001, B10001, B01111, B00001, B00001, B00001 },  //q(3px down)
  { B00000, B00000, B00000, B10110, B11001, B10000, B10000, B10000 },  //r
  { B00000, B00000, B00000, B01111, B10000, B01110, B00001, B11110 },  //s
  { B00000, B00000, B00000, B01000, B11110, B01000, B01001, B00110 },  //t
  { B00000, B00000, B00000, B10001, B10001, B10001, B10011, B01101 },  //u
  { B00000, B00000, B00000, B10001, B10001, B10001, B01010, B00100 },  //v
  { B00000, B00000, B00000, B10001, B10001, B10101, B11011, B10001 },  //w
  { B00000, B00000, B00000, B10001, B01010, B00100, B01010, B10001 },  //x
  { B10001, B10001, B10001, B10001, B01111, B00001, B10001, B01110 },  //y(3px down)
  { B00000, B00000, B00000, B11111, B00001, B01110, B10000, B11111 },  //z
  { B01110, B10001, B10001, B10001, B11111, B10001, B10001, B10001 },  //A
  { B11111, B10000, B10000, B11110, B10001, B10001, B10001, B11110 },  //Б
  { B11110, B10001, B10001, B11110, B10001, B10001, B10001, B11110 },  //B
  { B11111, B10000, B10000, B10000, B10000, B10000, B10000, B10000 },  //Г
  { B00110, B01010, B01010, B01010, B01010, B11111, B10001, B10001 },  //Д
  { B11111, B10000, B10000, B11110, B10000, B10000, B10000, B11111 },  //E
  { B01010, B00000, B11111, B10000, B11110, B10000, B10000, B11111 },  //Ё
  { B10101, B10101, B10101, B01110, B10101, B10101, B10101, B10101 },  //Ж
  { B01110, B10001, B00001, B01110, B00001, B00001, B10001, B01110 },  //З
  { B10001, B10011, B10011, B10101, B10101, B11001, B11001, B10001 },  //И
  { B01010, B00100, B10001, B10011, B10101, B10101, B11001, B10001 },  //Й
  { B10001, B10010, B10100, B11000, B11000, B10100, B10010, B10001 },  //К
  { B00111, B01001, B01001, B01001, B01001, B01001, B01001, B10001 },  //Л
  { B10001, B11011, B11011, B10101, B10101, B10001, B10001, B10001 },  //М
  { B10001, B10001, B10001, B11111, B10001, B10001, B10001, B10001 },  //Н
  { B01110, B10001, B10001, B10001, B10001, B10001, B10001, B01110 },  //О
  { B11111, B10001, B10001, B10001, B10001, B10001, B10001, B10001 },  //П
  { B11110, B10001, B10001, B10001, B11110, B10000, B10000, B10000 },  //Р
  { B01110, B10001, B10000, B10000, B10000, B10000, B10001, B01110 },  //С
  { B11111, B00100, B00100, B00100, B00100, B00100, B00100, B00100 },  //Т
  { B10001, B10001, B10001, B10001, B01111, B00001, B10001, B01110 },  //У
  { B00100, B01110, B10101, B10101, B10101, B01110, B00100, B00100 },  //Ф
  { B10001, B10001, B01010, B00100, B00100, B01010, B10001, B10001 },  //Х
  { B10010, B10010, B10010, B10010, B10010, B10010, B11111, B00001 },  //Ц
  { B10001, B10001, B10001, B10001, B01111, B00001, B00001, B00001 },  //Ч
  { B10101, B10101, B10101, B10101, B10101, B10101, B10101, B11111 },  //Ш
  { B10101, B10101, B10101, B10101, B10101, B10101, B11111, B00001 },  //Щ
  { B11000, B01000, B01000, B01110, B01001, B01001, B01001, B01110 },  //Ъ
  { B10001, B10001, B10001, B11101, B10101, B10101, B10101, B11101 },  //Ы
  { B10000, B10000, B10000, B11110, B10001, B10001, B10001, B11110 },  //Ь
  { B01110, B10001, B00001, B01111, B00001, B00001, B10001, B01110 },  //Э
  { B10111, B10101, B10101, B11101, B10101, B10101, B10101, B10111 },  //Ю
  { B01111, B10001, B10001, B10001, B01111, B10001, B10001, B10001 },  //Я
  { B00000, B00000, B00000, B01110, B00001, B01111, B10001, B01111 },  //a
  { B01110, B10000, B10000, B11110, B10001, B10001, B10001, B01110 },  //б
  { B00000, B00000, B00000, B11100, B10100, B11110, B10010, B11110 },  //в(4px weight)
  { B00000, B00000, B00000, B11110, B10000, B10000, B10000, B10000 },  //г(4px weight)
  { B00000, B00000, B00110, B01010, B01010, B01010, B11111, B10001 },  //д
  { B00000, B00000, B00000, B01110, B10001, B11111, B10000, B01110 },  //е
  { B00000, B01010, B00000, B01110, B10001, B11111, B10000, B01110 },  //ё
  { B00000, B00000, B00000, B10101, B10101, B01110, B10101, B10101 },  //ж
  { B00000, B00000, B00000, B01110, B10001, B00110, B10001, B01110 },  //з
  { B00000, B00000, B00000, B10001, B10011, B10101, B11001, B10001 },  //и
  { B00000, B01010, B00100, B10001, B10011, B10101, B11001, B10001 },  //й
  { B00000, B00000, B00000, B10010, B10100, B11000, B10100, B10010 },  //к(4px weight)
  { B00000, B00000, B00000, B01111, B01001, B01001, B01001, B10001 },  //л
  { B00000, B00000, B00000, B10001, B11011, B10101, B10001, B10001 },  //м
  { B00000, B00000, B00000, B10010, B10010, B11110, B10010, B10010 },  //н(4px weight)
  { B00000, B00000, B00000, B01110, B10001, B10001, B10001, B01110 },  //o
  { B00000, B00000, B00000, B11110, B10010, B10010, B10010, B10010 },  //п(4px weight)
  { B11110, B10001, B10001, B10001, B11110, B10000, B10000, B10000 },  //р(3px down)
  { B00000, B00000, B00000, B01110, B10001, B10000, B10001, B01110 },  //c
  { B00000, B00000, B00000, B11111, B00100, B00100, B00100, B00100 },  //т
  { B10001, B10001, B10001, B10001, B01111, B00001, B10001, B01110 },  //у(3px down)
  { B00100, B01110, B10101, B10101, B10101, B01110, B00100, B00100 },  //ф(2px down)
  { B00000, B00000, B00000, B10001, B01010, B00100, B01010, B10001 },  //х
  { B00000, B00000, B10010, B10010, B10010, B10010, B11111, B00001 },  //ц(1px down)
  { B00000, B00000, B00000, B10010, B10010, B10010, B01110, B00010 },  //ч(4px weight)
  { B00000, B00000, B00000, B10101, B10101, B10101, B10101, B11111 },  //ш
  { B00000, B00000, B10101, B10101, B10101, B10101, B11111, B00001 },  //щ(1px down)
  { B00000, B00000, B00000, B11000, B01000, B01110, B01001, B01110 },  //ъ
  { B00000, B00000, B00000, B10001, B10001, B11101, B10101, B11101 },  //ы
  { B00000, B00000, B00000, B10000, B10000, B11100, B10010, B11100 },  //ь(4px weight)
  { B00000, B00000, B00000, B01110, B10001, B00111, B10001, B01110 },  //э
  { B00000, B00000, B00000, B10111, B10101, B11101, B10101, B10111 },  //ю
  { B00000, B00000, B00000, B01110, B10010, B01110, B10010, B10010 },  //я(4px weight)
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
  { B00000, B01010, B11111, B01010, B01010, B11111, B01010, B00000 },  //#
  { B00000, B00000, B00000, B11111, B00000, B00000, B00000, B00000 },  //-
  { B00000, B00100, B00100, B11111, B00100, B00100, B00000, B00000 },  //+
  { B00000, B00000, B01010, B00100, B01010, B00000, B00000, B00000 },  //*
  { B00000, B00000, B11111, B00000, B11111, B00000, B00000, B00000 },  //=
  { B00001, B00010, B00010, B00100, B00100, B01000, B01000, B10000 },  //'/'
  { B10000, B01000, B01000, B00100, B00100, B00010, B00010, B00001 },  //'\'
  { B00100, B01010, B10001, B00000, B00000, B00000, B00000, B00000 },  //^
  { B01100, B10010, B10010, B01100, B10100, B10011, B10010, B01101 },  //&
  { B00000, B00000, B00000, B01010, B10101, B00000, B00000, B00000 },  //~
  { B10100, B10100, B00000, B00000, B00000, B00000, B00000, B00000 },  //"
  { B10000, B10000, B00000, B00000, B00000, B00000, B00000, B00000 },  //'
  { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000 }   //space
};
byte RED[k_max + 1][r_max + 1] = { 0 };
byte GRN[k_max + 1][r_max + 1] = { 0 };
byte BLU[k_max + 1][r_max + 1] = { 0 };
byte bufer[k_max + 1] = { 0 };
byte brPercent = 50;
unsigned long timer;
unsigned long ticking;
char savedSSID[64];
char savedPassword[64];
void printLocalTime(int r, int g, int b);
void serverTask(void *pvParameters);
void mainLogicTask(void *pvParameters);
void dotGen();
void clr(int clear_step);
void ball(int currentDot);
void led();
void startAPMode();
float randomXV(float n);
void symbolDraw(byte symbol[], byte offsetX, byte offsetY, int r, int g, int b);
void moveToLeft();
void textScroll(String text, int speed, int startPosY, int r, int g, int b);
byte percentToRange(int percent);
void loadCredentials();
void saveCredentials(String ssid, String password);
void handleDotSubmit();
void handleBrightenssSubmit();
void handleModeSubmit();
void handleColorSubmit();
void handleRoot();
void handleAP();
void handleWifiSave();
void handleNotFound();
void restart();
void handleTextSubmit();
byte getSymbolIndex(char c);
void setup() {
  Serial.begin(115200);
  xMutex = xSemaphoreCreateMutex();
  xLinesMutex = xSemaphoreCreateMutex();
  if (xMutex == NULL || xLinesMutex == NULL) {
    Serial.println("Ошибка создания мьютексов!");
    while (1) delay(1000);
  }
  EEPROM.begin(EEPROM_SIZE);
  FastLED.addLeds<WS2812, 4, GRB>(leds, LED).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(percentToRange(brPercent));
  WiFi.softAPdisconnect(true);
  loadCredentials();
  timer = millis();
  ticking = millis();
  if (hasCredentials) {
    unsigned long startTime = millis();
    WiFi.begin(savedSSID, savedPassword);
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
      if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
        RED[0][16] = random(0, 255);
        GRN[0][16] = random(0, 255);
        BLU[0][16] = random(0, 255);
        updateLedsDirect();
        xSemaphoreGive(xLinesMutex);
      }
      delay(250);
      if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
        memset(RED, 0, sizeof(RED));
        memset(GRN, 0, sizeof(GRN));
        memset(BLU, 0, sizeof(BLU));
        updateLedsDirect();
        xSemaphoreGive(xLinesMutex);
      }
      Serial.print(".");
      delay(250);
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Не удалось подключиться к WiFi!");
      Serial.println("Включение AP режима...");
      WiFi.disconnect(true);
      startAPMode();
    } else {
      if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
        memset(RED, 0, sizeof(RED));
        memset(GRN, 0, sizeof(GRN));
        memset(BLU, 0, sizeof(BLU));
        xSemaphoreGive(xLinesMutex);
      }
      Serial.println("\nWiFi connected!");
      Serial.println("IP: " + WiFi.localIP().toString());
      WiFi.softAPdisconnect(true);
      dnsServer.stop();
      WiFi.mode(WIFI_STA);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      if (!MDNS.begin("matrix")) {
        Serial.println("Error setting up MDNS responder!");
        while (1) delay(1000);
      }
      Serial.println("mDNS responder started");
      dotGen();
      server.on("/", handleRoot);
      server.on("/dotSubmit", handleDotSubmit);
      server.on("/modeSubmit", handleModeSubmit);
      server.on("/colorSubmit", handleColorSubmit);
      server.on("/brightnessSubmit", handleBrightenssSubmit);
      server.on("/textSubmit", handleTextSubmit);
      server.on("/textColorSubmit", handleTextColorSubmit);
      server.begin();
      MDNS.addService("http", "tcp", 80);
    }
  } else {
    startAPMode();
  }
  xTaskCreatePinnedToCore(serverTask, "ServerTask", 15000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(mainLogicTask, "MainLogicTask", 15000, NULL, 1, NULL, 1);
  vTaskDelete(NULL);
}
void updateLedsDirect() {
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

  FastLED.show();
}
void serverTask(void *pvParameters) {
  for (;;) {
    if (isSetup) {
      dnsServer.processNextRequest();
    }
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
void mainLogicTask(void *pvParameters) {
  while (1) {
    bool currentIsSetup;
    int currentDot, currentClockColorR, currentClockColorG, currentClockColorB, currentMode, currentTextColorR, currentTextColorG, currentTextColorB;
    String currentRunningText;
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      currentIsSetup = isSetup;
      currentDot = dot;
      currentClockColorR = clockColorR;
      currentClockColorG = clockColorG;
      currentClockColorB = clockColorB;
      currentTextColorR = textColorR;
      currentTextColorG = textColorG;
      currentTextColorB = textColorB;
      currentRunningText = runningText;
      currentMode = mode;
      xSemaphoreGive(xMutex);
    }
    if (currentIsSetup) {
      if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
        symbolDraw(symbols[0], 9, 5, 255, 0, 0);
        symbolDraw(symbols[15], 3, 5, 255, 0, 0);
        xSemaphoreGive(xLinesMutex);
      }
    }
    switch (currentMode) {
      case 0:
        ball(currentDot);
        break;
      case 1:
        if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
          printLocalTime(currentClockColorR, currentClockColorG, currentClockColorB);
          xSemaphoreGive(xLinesMutex);
        }
        break;
      case 2:
        textScroll(currentRunningText + "   ", 75, 4, currentTextColorR, currentTextColorG, currentTextColorB);
        break;
    }
    led();
    if (ledsNeedUpdate) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
        FastLED.show();
        ledsNeedUpdate = false;
        xSemaphoreGive(xMutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
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
void printLocalTime(int r, int g, int b) {
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
      symbolDraw(symbols[118], 12, 0, r, g, b);
      break;
    case '1':
      symbolDraw(symbols[119], 12, 0, r, g, b);
      break;
    case '2':
      symbolDraw(symbols[120], 12, 0, r, g, b);
      break;
  }
  switch (timeHour[1]) {
    case '0':
      symbolDraw(symbols[118], 6, 0, r, g, b);
      break;
    case '1':
      symbolDraw(symbols[119], 6, 0, r, g, b);
      break;
    case '2':
      symbolDraw(symbols[120], 6, 0, r, g, b);
      break;
    case '3':
      symbolDraw(symbols[121], 6, 0, r, g, b);
      break;
    case '4':
      symbolDraw(symbols[122], 6, 0, r, g, b);
      break;
    case '5':
      symbolDraw(symbols[123], 6, 0, r, g, b);
      break;
    case '6':
      symbolDraw(symbols[124], 6, 0, r, g, b);
      break;
    case '7':
      symbolDraw(symbols[125], 6, 0, r, g, b);
      break;
    case '8':
      symbolDraw(symbols[126], 6, 0, r, g, b);
      break;
    case '9':
      symbolDraw(symbols[127], 6, 0, r, g, b);
      break;
  }
  switch (timeMinute[0]) {
    case '0':
      symbolDraw(symbols[118], 6, 9, r, g, b);
      break;
    case '1':
      symbolDraw(symbols[119], 6, 9, r, g, b);
      break;
    case '2':
      symbolDraw(symbols[120], 6, 9, r, g, b);
      break;
    case '3':
      symbolDraw(symbols[121], 6, 9, r, g, b);
      break;
    case '4':
      symbolDraw(symbols[122], 6, 9, r, g, b);
      break;
    case '5':
      symbolDraw(symbols[123], 6, 9, r, g, b);
      break;
  }
  switch (timeMinute[1]) {
    case '0':
      symbolDraw(symbols[118], 0, 9, r, g, b);
      break;
    case '1':
      symbolDraw(symbols[119], 0, 9, r, g, b);
      break;
    case '2':
      symbolDraw(symbols[120], 0, 9, r, g, b);
      break;
    case '3':
      symbolDraw(symbols[121], 0, 9, r, g, b);
      break;
    case '4':
      symbolDraw(symbols[122], 0, 9, r, g, b);
      break;
    case '5':
      symbolDraw(symbols[123], 0, 9, r, g, b);
      break;
    case '6':
      symbolDraw(symbols[124], 0, 9, r, g, b);
      break;
    case '7':
      symbolDraw(symbols[125], 0, 9, r, g, b);
      break;
    case '8':
      symbolDraw(symbols[126], 0, 9, r, g, b);
      break;
    case '9':
      symbolDraw(symbols[127], 0, 9, r, g, b);
      break;
  }
}
void ball(int currentDot) {
  if (millis() - ticking >= 100) {
    if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
      clr(50);
      for (int i = 0; i < currentDot; i++) {
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
      for (int i = 0; i < currentDot; i++) {
        for (int j = i + 1; j < currentDot; j++) {
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
      xSemaphoreGive(xLinesMutex);
    }
    ticking = millis();
  }
}
void led() {
  if (millis() - timer >= 100) {
    if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
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
      ledsNeedUpdate = true;
      timer = millis();
      xSemaphoreGive(xLinesMutex);
    }
  }
}
void startAPMode() {
  if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
    isSetup = true;
    xSemaphoreGive(xMutex);
  }
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
void symbolDraw(byte symbol[], byte offsetX, byte offsetY, int r, int g, int b) {
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
          valueR = r;
          valueG = g;
          valueB = b;
          break;
      }
      RED[section + offsetY][i + offsetX] = valueR;
      GRN[section + offsetY][i + offsetX] = valueG;
      BLU[section + offsetY][i + offsetX] = valueB;
    }
  }
}
void moveToLeft() {
  //RED
  for (int i = 0; i < 17; i++) {
    bufer[i] = RED[i][16];
  }
  for (int i = 15; i >= 0; i--) {
    for (int j = 0; j < 17; j++) {
      RED[j][i + 1] = RED[j][i];
    }
  }
  for (int i = 0; i < 17; i++) {
    RED[i][0] = bufer[i];
  }
  //GRN
  for (int i = 0; i < 17; i++) {
    bufer[i] = GRN[i][16];
  }
  for (int i = 15; i >= 0; i--) {
    for (int j = 0; j < 17; j++) {
      GRN[j][i + 1] = GRN[j][i];
    }
  }
  for (int i = 0; i < 17; i++) {
    GRN[i][0] = bufer[i];
  }
  //BLU
  for (int i = 0; i < 17; i++) {
    bufer[i] = BLU[i][16];
  }
  for (int i = 15; i >= 0; i--) {
    for (int j = 0; j < 17; j++) {
      BLU[j][i + 1] = BLU[j][i];
    }
  }
  for (int i = 0; i < 17; i++) {
    BLU[i][0] = bufer[i];
  }
}
void textScroll(String text, int speed, int startPosY, int r, int g, int b) {
  if (text.length() == 0 || text == " ") return;
  if (millis() - scrollTimer < speed) return;
  scrollTimer = millis();
  String localCurrentText;
  if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
    localCurrentText = currentText;
    xSemaphoreGive(xMutex);
  }
  if (text != localCurrentText) {
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      currentText = text;
      textPosition = 0;
      charColumn = 0;
      scrollOffset = 0;
      xSemaphoreGive(xMutex);
    }
  }
  moveToLeft();
  for (int y = 0; y <= k_max; y++) {
    RED[y][r_max] = 0;
    GRN[y][r_max] = 0;
    BLU[y][r_max] = 0;
  }
  if (textPosition < text.length()) {
    char c = text[textPosition];
    byte symbolIndex = getSymbolIndex(c);
    int yOffset = 0;
    if (c == 'g' || c == 'j' || c == 'p' || c == 'q' || c == 'y')
      yOffset = 3;
    else if (c == ',')
      yOffset = 2;
    int charWidth = 5;
    if (c == '"')
      charWidth = 3;
    else if (c == '.' || c == ',' || c == '\'')
      charWidth = 1;
    if (charColumn < charWidth) {
      for (int row = 0; row < 8; row++) {
        int y = startPosY + row + yOffset;
        if (y < 0 || y > k_max) continue;
        if (bitRead(symbols[symbolIndex][row], 4 - charColumn)) {
          RED[y][r_max] = r;
          GRN[y][r_max] = g;
          BLU[y][r_max] = b;
        }
      }
      charColumn++;
    } else {
      charColumn = 0;
      textPosition++;
    }
  } else {
    textPosition = 0;
    charColumn = 0;
  }
}
byte getSymbolIndex(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return 26 + (c - 'a');
  if (c >= '0' && c <= '9') return 118 + (c - '0');
  switch (c) {
    case '!': return 128;
    case '?': return 129;
    case '.': return 130;
    case ':': return 131;
    case ',': return 132;
    case ';': return 133;
    case '(': return 134;
    case ')': return 135;
    case '[': return 136;
    case ']': return 137;
    case '<': return 138;
    case '>': return 139;
    case '{': return 140;
    case '}': return 141;
    case '%': return 142;
    case '@': return 143;
    case '#': return 144;
    case '-': return 145;
    case '+': return 146;
    case '*': return 147;
    case '=': return 148;
    case '/': return 149;
    case '\\': return 150;
    case '^': return 151;
    case '&': return 152;
    case '~': return 154;
    case '"': return 155;
    case '\'': return 156;
    case ' ': return 157;
    default: return 129;  // пробел для неизвестных символов
  }
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
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      dot = number.toInt();
      xSemaphoreGive(xMutex);
    }

    if (xSemaphoreTake(xLinesMutex, portMAX_DELAY) == pdTRUE) {
      dotGen();
      xSemaphoreGive(xLinesMutex);
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleBrightenssSubmit() {
  if (server.hasArg("brightnessRange")) {
    String number = server.arg("brightnessRange");
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      brPercent = number.toInt();
      FastLED.setBrightness(percentToRange(brPercent));
      xSemaphoreGive(xMutex);
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleModeSubmit() {
  mode = (mode + 1) % 3;
  memset(RED, 0, sizeof(RED));
  memset(GRN, 0, sizeof(GRN));
  memset(BLU, 0, sizeof(BLU));
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
void handleTextColorSubmit() {
  if (server.hasArg("textColor")) {
    String colorHex = server.arg("textColor");
    if (colorHex.startsWith("#")) {
      colorHex = colorHex.substring(1);
    }
    if (colorHex.length() >= 6) {
      long colorValue = strtol(colorHex.c_str(), NULL, 16);
      textColorR = (colorValue >> 16) & 0xFF;
      textColorG = (colorValue >> 8) & 0xFF;
      textColorB = colorValue & 0xFF;
      textColorHex = colorHex;
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleRoot() {
  String buttonText;
  if (mode == 0) buttonText = "Режим часов";
  else if (mode == 1) buttonText = "Режим текста";
  else buttonText = "Режим точек";

  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>PixelStorm control</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; }";
  html += "h1 { color: #333; }";
  html += "form { margin: 20px 0; }";
  html += "select, button, input[type='text'] { padding: 8px; margin: 5px 0; width: 100%; box-sizing: border-box; }";
  html += "</style>";
  html += "<script>";
  html += "function updateSliderValue(sliderId, valueId) {";
  html += "document.getElementById(valueId).textContent = document.getElementById(sliderId).value + '%';";
  html += "}";
  html += "</script>";
  html += "</head>";
  html += "<body>";
  html += "<h1>PixelStorm control</h1>";

  if (mode == 0) {
    html += "<form action=\"/dotSubmit\" method=\"POST\">";
    html += "<label for=\"dotText\">Количество точек:</label>";
    html += "<select name=\"dotsSelect\" id=\"dotsSelect\">";
    for (int i = 1; i <= 30; i++) {
      html += "<option value=\"" + String(i) + "\"" + (i == dot ? " selected" : "") + ">" + String(i) + "</option>";
    }
    html += "</select>";
    html += "<input type=\"submit\" value=\"Применить\">";
    html += "</form>";
  } else if (mode == 1) {
    html += "<form action=\"/colorSubmit\" method=\"POST\">";
    html += "<label for=\"clockColor\">Цвет часов:</label>";
    html += "<input type=\"color\" id=\"clockColor\" name=\"clockColor\" value=\"#";
    html += clockColorHex;
    html += "\">";
    html += "<input type=\"submit\" value=\"Применить\">";
    html += "</form>";
  } else if (mode == 2) {
    html += "<form action=\"/textSubmit\" method=\"POST\">";
    html += "<label for=\"textInput\">Текст для прокрутки:</label>";
    html += "<input type=\"text\" id=\"textInput\" name=\"textInput\" value=\"" + runningText + "\">";
    html += "<input type=\"submit\" value=\"Применить\">";
    html += "</form>";
    html += "<form action=\"/textColorSubmit\" method=\"POST\">";
    html += "<label for=\"textColor\">Цвет текста:</label>";
    html += "<input type=\"color\" id=\"textColor\" name=\"textColor\" value=\"#";
    html += textColorHex;
    html += "\">";
    html += "<input type=\"submit\" value=\"Применить\">";
    html += "</form>";
  }

  html += "<form action=\"/brightnessSubmit\" method=\"POST\">";
  html += "<label for=\"brightnessRange\">Яркость:</label>";
  html += "<div class=\"slider-container\">";
  html += "<input type=\"range\" id=\"brightnessRange\" name=\"brightnessRange\" min=\"1\" max=\"100\" value=\"" + String(brPercent) + "\" oninput=\"updateSliderValue('brightnessRange', 'brValue')\">";
  html += "<span id=\"brValue\" class=\"slider-value\">" + String(brPercent) + "%</span>";
  html += "</div>";
  html += "<input type=\"submit\" value=\"Применить\">";
  html += "</form>";

  html += "<form action=\"/modeSubmit\" method=\"POST\">";
  html += "<button type=\"submit\">" + buttonText + "</button>";
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
  <h2>При успешном подключении веб-конфигуратор будет доступен на <a href="matrix.local">matrix.local</a></h2>
</body>
</html>
)=====";
    server.send(200, "text/html; charset=utf-8", html);
    delay(3000);
    WiFi.softAPdisconnect(true);
    ESP.restart();
    vTaskDelete(NULL);
  } else {
    server.send(400, "text/html; charset=utf-8", "<h1>Ошибка: SSID не может быть пустым!</h1>");
  }
}
void handleNotFound() {
  server.sendHeader("Location", "http://" + server.client().localIP().toString(), true);
  server.send(302, "text/plain", "");
}
void handleTextSubmit() {
  memset(RED, 0, sizeof(RED));
  memset(GRN, 0, sizeof(GRN));
  memset(BLU, 0, sizeof(BLU));
  if (server.hasArg("textInput")) {
    String newText = server.arg("textInput");
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      runningText = newText;
      currentText = "";
      textPosition = 0;
      charColumn = 0;
      xSemaphoreGive(xMutex);
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
void loop() {}