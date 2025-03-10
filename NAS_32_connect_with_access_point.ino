// this is a NAS code for ESP32 --> TO CONNECT YOUR HOTSPOT (mobile aur home router) but this could not create its own access point.
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>

// Define SD Card pins
#define SD_CS    5
#define SD_SCK   18
#define SD_MOSI  23
#define SD_MISO  19

// Wi-Fi credentials
const char* ssid = "spa";
const char* password = "12345678";

// Web server on port 80 (for files & UI)
AsyncWebServer server(80);

// Handle File Upload
File uploadFile;

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (index == 0) {
    uploadFile = SD.open("/" + filename, FILE_WRITE);
  }
  if (uploadFile) {
    uploadFile.write(data, len);
  }
  if (final) {
    uploadFile.close();
    request->send(200, "text/plain", "Upload Complete");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  // Serve index.html if it exists
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (SD.exists("/index.html")) {
      request->send(SD, "/index.html", "text/html");
    } else {
      request->send(404, "text/plain", "index.html not found");
    }
  });

  // Serve files
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileList = "<h2>File List</h2><ul>";
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file) {
      fileList += "<li><a href='/download?file=" + String(file.name()) + "'>" + String(file.name()) + "</a> <button onclick='deleteFile(\"" + String(file.name()) + "\")'>Delete</button></li>";
      file = root.openNextFile();
    }
    fileList += "</ul>";
    request->send(200, "text/html", fileList);
  });

  // Download files
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String filename = "/" + request->getParam("file")->value();
      if (SD.exists(filename)) {
        request->send(SD, filename, "application/octet-stream");
      } else {
        request->send(404, "text/plain", "File not found");
      }
    }
  });

  // Delete files
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String filename = "/" + request->getParam("file")->value();
      if (SD.exists(filename)) {
        SD.remove(filename);
        request->send(200, "text/plain", "File Deleted");
      } else {
        request->send(404, "text/plain", "File not found");
      }
    }
  });

  // Handle file uploads
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Upload Complete");
  }, handleUpload);

  server.begin();
}

void loop() {}
