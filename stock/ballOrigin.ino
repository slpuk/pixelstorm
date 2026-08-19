#include <FastLED.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include "time.h"
WebServer server(80);  // Создаем сервер на порту 80
#define LED 289
CRGB leds[LED];
const int r_max = 16;
const int k_max = 16;
const int max_dots = 30;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 10800;
const int daylightOffset_sec = 0;
void printLocalTime();
bool isClock = false;
int dot = 15;
byte red, grn, blu;
byte zero[] = { B01110, B10001, B10001, B10001, B10001, B10001, B10001, B01110 };
byte one[] = { B00100, B01100, B10100, B00100, B00100, B00100, B00100, B11111 };
byte two[] = { B01110, B10001, B00001, B00010, B00100, B01000, B10000, B11111 };
byte three[] = { B01110, B10001, B00001, B01110, B00001, B00001, B10001, B01110 };
byte four[] = { B10001, B10001, B10001, B10001, B01111, B00001, B00001, B00001 };
byte five[] = { B11111, B10000, B11110, B00001, B00001, B00001, B10001, B01110 };
byte six[] = { B01110, B10001, B10000, B11110, B10001, B10001, B10001, B01110 };
byte seven[] = { B11111, B00001, B00001, B00010, B00100, B00100, B00100, B00100 };
byte eight[] = { B01110, B10001, B10001, B01110, B10001, B10001, B10001, B01110 };
byte nine[] = { B01110, B10001, B10001, B10001, B01111, B00001, B10001, B01110 };
byte spacing[] = { B00000, B00000, B00100, B00000, B00000, B00100, B00000, B00000 };
float lines [max_dots][7];
byte RED[k_max + 1][r_max + 1];
byte GRN[k_max + 1][r_max + 1];
byte BLU[k_max + 1][r_max + 1];
unsigned long timer;
unsigned long ticking;
const char* ssid = "Maus";
const char* password = "55053303";
byte bufer[k_max + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
void setup() {
  FastLED.addLeds <WS2812, 4, GRB>(leds, LED).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  FastLED.show();
  timer = millis();
  ticking = millis();
  Serial.begin(115200);
  dotGen();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (!MDNS.begin("matrix")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  Serial.println("You can now access the device at http://matrix.local");
  server.on("/", handleRoot);      // Главная страница
  server.on("/submit", handleSubmit);
  server.on("/modesubmit", modeSubmit);
  server.begin();
  Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);
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
  led();
  if (isClock == false)
    ball();
  else
    printLocalTime();
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
      symbolDraw(zero, 12, 0);
      break;
    case '1':
      symbolDraw(one, 12, 0);
      break;
    case '2':
      symbolDraw(two, 12, 0);
      break;
  }
  switch (timeHour[1]) {
    case '0':
      symbolDraw(zero, 6, 0);
      break;
    case '1':
      symbolDraw(one, 6, 0);
      break;
    case '2':
      symbolDraw(two, 6, 0);
      break;
    case '3':
      symbolDraw(three, 6, 0);
      break;
    case '4':
      symbolDraw(four, 6, 0);
      break;
    case '5':
      symbolDraw(five, 6, 0);
      break;
    case '6':
      symbolDraw(six, 6, 0);
      break;
    case '7':
      symbolDraw(seven, 6, 0);
      break;
    case '8':
      symbolDraw(eight, 6, 0);
      break;
    case '9':
      symbolDraw(nine, 6, 0);
      break;
  }
  switch (timeMinute[0]) {
    case '0':
      symbolDraw(zero, 6, 9);
      break;
    case '1':
      symbolDraw(one, 6, 9);
      break;
    case '2':
      symbolDraw(two, 6, 9);
      break;
    case '3':
      symbolDraw(three, 6, 9);
      break;
    case '4':
      symbolDraw(four, 6, 9);
      break;
    case '5':
      symbolDraw(five, 6, 9);
      break;
  }
  switch (timeMinute[1]) {
    case '0':
      symbolDraw(zero, 0, 9);
      break;
    case '1':
      symbolDraw(one, 0, 9);
      break;
    case '2':
      symbolDraw(two, 0, 9);
      break;
    case '3':
      symbolDraw(three, 0, 9);
      break;
    case '4':
      symbolDraw(four, 0, 9);
      break;
    case '5':
      symbolDraw(five, 0, 9);
      break;
    case '6':
      symbolDraw(six, 0, 9);
      break;
    case '7':
      symbolDraw(seven, 0, 9);
      break;
    case '8':
      symbolDraw(eight, 0, 9);
      break;
    case '9':
      symbolDraw(nine, 0, 9);
      break;
  }
}
int firstDigitCalc(int number) {
  int firstDigit = number / 10;
  return firstDigit;
}
int secondDigitCalc(int number) {
  int secondDigit = number % 10;
  return secondDigit;
}
void led() {
  if (millis() - timer >= 100) {
    for (int i = 0; i < LED; i++) {
      leds[i] = 0;
    }
    for (int k = 0; k <= k_max; k++) {
      for (int r = 0; r <= r_max; r++) {
        if (RED[k][r] != 0) {
          if (k % 2 == 0) {
            leds[(k_max - k) * (r_max + 1) + (r_max - r)].r = RED[k][r];
          }
          else {
            leds[(k_max - k) * (r_max + 1) + r].r = RED[k][r];
          }
        }
      }
    }
    for (int k = 0; k <= k_max; k++) {
      for (int r = 0; r <= r_max; r++) {
        if (GRN[k][r] != 0) {
          if (k % 2 == 0) {
            leds[(k_max - k) * (r_max + 1) + (r_max - r)].g = GRN[k][r];
          }
          else {
            leds[(k_max - k) * (r_max + 1) + r].g = GRN[k][r];
          }
        }
      }
    }
    for (int k = 0; k <= k_max; k++) {
      for (int r = 0; r <= r_max; r++) {
        if (BLU[k][r] != 0) {
          if (k % 2 == 0) {
            leds[(k_max - k) * (r_max + 1) + (r_max - r)].b = BLU[k][r];
          }
          else {
            leds[(k_max - k) * (r_max + 1) + r].b = BLU[k][r];
          }
        }
      }
    }
    timer = millis();
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
      //physic


      RED[(int)round(lines[i][1])][(int)round(lines[i][0])] = (byte)round(lines[i][4]);
      GRN[(int)round(lines[i][1])][(int)round(lines[i][0])] = (byte)round(lines[i][5]);
      BLU[(int)round(lines[i][1])][(int)round(lines[i][0])] = (byte)round(lines[i][6]);
    }
    for (int i = 0; i < dot; i++) {
      for (int j = i + 1; j < dot; j++) {
        if (lines[i][0] == lines[j][0] && lines[i][1] == lines[j][1]) {
          float vel;
          Serial.print("Столкновение "); Serial.print(i); Serial.print(" c "); Serial.println(j);
          for (int o = - 1; o <= 1; o++) {
            RED[(int)round(lines[i][1])][(int)round(lines[i][0]) - o] = (byte)round((lines[i][4] + lines[j][4]) / 2);
            GRN[(int)round(lines[i][1])][(int)round(lines[i][0] - o)] = (byte)round((lines[i][5] + lines[j][5]) / 2);
            BLU[(int)round(lines[i][1])][(int)round(lines[i][0] - o)] = (byte)round((lines[i][6] + lines[j][6]) / 2);
          }
          for (int o = - 1; o <= 1; o++) {
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
float randomXV(float n) {
  return (random(0, 100) - 50) / 100.0;
}
void symbolDraw(byte symbol[], byte offsetX, byte offsetY) {
  int value = 0;
  for (int section = 0; section < 8; section++) {
    for (int i = 0; i < 5; i++) {
      if (bitRead(symbol[section], i) == 1)
        value = 255;
      else
        value = 0;
      RED[section + offsetY][i + offsetX] = value;
    }
  }
}
void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>MATRIX</title>";
  html += "</head>";
  html += "<body>";
  html += "<h1>MATRIX Control</h1>";
  html += "<form action=\"/submit\" method=\"POST\">";
  html += "<label for=\"dotText\">Dots value:</label>";
  html += "<select name=\"dotsSelect\" id=\"dotsSelect\">";
  for (int i = 1; i <= 30; i++) {
    html += "<option value=\"" + String(i) + "\">" + String(i) + "</option>";
  }
  html += "</select>";
  html += "<input type=\"submit\" value=\"Set\">";
  html += "</form>";
  html += "<p>" + String(dot) + "</p>";
  html += "<form action=\"/modesubmit\" method=\"POST\">";
  html += "<button type=/modesubmit>Change mode</button>";
  html += "</form>";
  html += "</body>";
  html += "</html>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  if (server.hasArg("dotsSelect")) {
    String number = server.arg("dotsSelect");
    dot = number.toInt();
    server.sendHeader("Location", "/");
    server.send(303);
  }
  dotGen();
}
void modeSubmit() {
  memset(RED, 0, sizeof(RED));
  memset(GRN, 0, sizeof(GRN));
  memset(BLU, 0, sizeof(BLU));
  isClock = !isClock;
  server.sendHeader("Location", "/");
  server.send(303);
}