#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <SD.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#include "camera_utils.h"

const char* ssid = "CANT INCLUDE FOR OBVIOUS REASONS";
const char* password = "CANT INCLUDE FOR OBVIOUS REASONS";

#define SD_CS   5
#define LED_PIN 2

AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();
File photoFile;

String listFiles();
void capturePhotoSaveSD();
void drawPhotoOnTFT(const String& path);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (!SPIFFS.begin(true)) Serial.println("SPIFFS mount failed");
  if (!SD.begin(SD_CS))     Serial.println("SD card mount failed");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  initCamera();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("ESP32 Photo Server", 10, 10, 2);
  tft.drawString(WiFi.localIP().toString(), 10, 30, 2);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){ r->send(SPIFFS, "/index.html", "text/html"); });
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *r){
      capturePhotoSaveSD();
      r->send(200, "text/plain", "Displayed Image");
  });
  server.on("/photos", HTTP_GET, [](AsyncWebServerRequest *r){ r->send(200,"text/plain", listFiles()); });
  server.on("/photo", HTTP_GET, [](AsyncWebServerRequest *r){
      if (r->hasParam("name")) {
        String f = "/" + r->getParam("name")->value();
        SD.exists(f) ? r->send(SD,f,"image/jpeg") : r->send(404,"text/plain","Missing");
      } else r->send(400,"text/plain","Missing 'name'");
  });
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {}

String listFiles() {
  String s="Files on SD:\n";
  File root=SD.open("/"), f=root.openNextFile();
  while(f){ s+=String(f.name())+" ("+String(f.size())+" bytes)\n"; f=root.openNextFile(); }
  return s;
}

void capturePhotoSaveSD() {
  digitalWrite(LED_PIN,HIGH);
  delay(100);
  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb){ Serial.println("Capture fail"); digitalWrite(LED_PIN,LOW); return; }

  String path = "/photo_" + String(millis()) + ".jpg";
  File file = SD.open(path, FILE_WRITE);
  if(file){ file.write(fb->buf, fb->len); file.close(); drawPhotoOnTFT(path); }
  else Serial.println("Write fail");
  esp_camera_fb_return(fb);
  digitalWrite(LED_PIN,LOW);
}

void drawPhotoOnTFT(const String& path) {
  File img = SD.open(path);
  if(!img){ Serial.println("Display: open fail"); return; }
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Latest Photo:", 10, 5, 2);
  tft.drawString(path, 10, 25, 2);
  img.close();
}
