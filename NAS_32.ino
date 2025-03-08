#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>

// Define the pin for the onboard LED (adjust if necessary)
#define LED_PIN 2

// Define the SD card pins (update these if your wiring is different)
// For example, using: CS = 5, SCK = 18, MOSI = 23, MISO = 19:
#define SD_CS    5
#define SD_SCK   18
#define SD_MOSI  23
#define SD_MISO  19

// Wi-Fi credentials
const char* ssid = "spa";
const char* password = "12345678";

// Create two AsyncWebServer instances:
// - server on port 80 for general file serving
// - nasServer on port 8080 for NAS operations (file listing, upload, deletion)
AsyncWebServer server(80);
AsyncWebServer nasServer(8080);

// Global file handle for uploads
File uploadFile;

// Callback to handle file uploads via the NAS endpoint
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // On first call, open the file on the SD card for writing
  if (index == 0) {
    Serial.printf("Upload Start: %s\n", filename.c_str());
    // Open (or create) the file in write mode in the root directory
    uploadFile = SD.open("/" + filename, FILE_WRITE);
    if (!uploadFile) {
      Serial.println("Failed to open file for writing");
      return;
    }
  }
  
  // Write incoming data to the file
  if (uploadFile) {
    uploadFile.write(data, len);
  }

  // Once the upload is complete, close the file
  if (final) {
    Serial.printf("Upload End: %s, %u bytes\n", filename.c_str(), index + len);
    uploadFile.close();
  }
}

void setup() {
  Serial.begin(115200);
  // Set the LED pin as output for status indication
  pinMode(LED_PIN, OUTPUT);
  
  Serial.print("Connecting to Wi-Fi network: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    tries++;
    if (tries > 20) {
      Serial.println("\nFailed to connect to WiFi");
      return;
    }
  }
  
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Initialize the SD card (SPI pins are automatically configured by the board support package)
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  // Check if index.html exists on the SD card
  if (!SD.exists("/index.html")) {
    Serial.println("index.html missing on SD card");
    // Optionally, create a default index.html or handle the error accordingly.
  }
  
  // ----------------------
  // Setup general file server on port 80
  // ----------------------
  server.onNotFound([](AsyncWebServerRequest *request) {
    String path = request->url();
    // Default to index.html if the URL ends with '/'
    if (path.endsWith("/")) path += "index.html";
    
    // Determine the MIME type based on the file extension
    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    
    if (!SD.exists(path)) {
      request->send(404, "text/html", "<html><body><h1>File not found</h1></body></html>");
      return;
    }
    // Serve the file from the SD card
    request->send(SD, path, contentType);
  });
  server.begin();
  
  // ----------------------
  // Setup NAS endpoints on port 8080
  // ----------------------
  
  // Endpoint to list all files on the SD card
  nasServer.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileList = "";
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file) {
      fileList += String(file.name()) + "\n";
      file.close();
      file = root.openNextFile();
    }
    request->send(200, "text/plain", fileList);
  });
  
  // Protected endpoint for file uploads
  nasServer.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Use basic HTTP authentication (username: admin, password: password)
    if (!request->authenticate("admin", "password")) {
      return request->requestAuthentication();
    }
    // Respond with 200 OK after the upload is handled
    request->send(200);
  }, handleUpload);
  
  // Protected endpoint for file deletion
  nasServer.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate("admin", "password")) {
      return request->requestAuthentication();
    }
    if (request->hasParam("file")) {
      String filename = request->getParam("file")->value();
      if (SD.exists(filename)) {
        SD.remove(filename);
        request->send(200, "text/plain", "File deleted");
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      request->send(400, "text/plain", "Bad request: no file specified");
    }
  });
  nasServer.begin();
}

void loop() {
  // Blink the onboard LED as a status indicator
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
