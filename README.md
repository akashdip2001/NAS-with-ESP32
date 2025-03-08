# NAS-with-ESP32
Network-attached storage with ESP WROOM 32

![SetUP](https://github.com/user-attachments/assets/756d28b0-82b4-49c3-b0cf-9cf150bb8596)

## Testing Code

```cpp
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
```
![Screenshot (191)](https://github.com/user-attachments/assets/5c1afd81-2e92-418c-bd81-fe010acb7cd1)

The error indicates that the Arduino IDE can’t find the ESPAsyncWebServer library. To resolve this, you need to install both the **ESPAsyncWebServer** and its dependency **AsyncTCP** libraries. Follow these steps:

---

### 1. **Download the Required Libraries**

- **ESPAsyncWebServer:**  
  - Go to the [ESPAsyncWebServer GitHub page](https://github.com/me-no-dev/ESPAsyncWebServer).
  - Click the green **Code** button and choose **Download ZIP**.

- **AsyncTCP:**  
  - Visit the [AsyncTCP GitHub page](https://github.com/me-no-dev/AsyncTCP).
  - Click the green **Code** button and select **Download ZIP**.

---

### 2. **Install the Libraries in the Arduino IDE**

- Open the Arduino IDE.
- Go to **Sketch > Include Library > Add .ZIP Library…**
- In the file dialog, locate and select the downloaded ZIP file for **AsyncTCP**.
- Repeat the process for the **ESPAsyncWebServer** ZIP file.

*Tip:* Make sure you install **AsyncTCP** **before** installing **ESPAsyncWebServer** since it’s a dependency.

---

### 3. **Verify Library Installation**

- Once installed, you can verify by going to **Sketch > Include Library** and scrolling down to see if both libraries are listed.
- If they’re correctly installed, the IDE will find the header files when compiling your project.

---

### 4. **Double-Check Board Setup**

Since you’re using an ESP32 board, make sure you’ve installed the ESP32 board definitions:
- Go to **File > Preferences** and add the following URL in the **Additional Boards Manager URLs** field:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- Then go to **Tools > Board > Boards Manager**, search for **esp32**, and install the package by **Espressif Systems**.
- Select your ESP32 board under **Tools > Board** (e.g., “ESP32 Dev Module”).

---

### 5. **Compile Your Project**

After installing the libraries and setting up the board:
- Open your project sketch.
- Verify that the pin definitions (e.g., CS, SCK, MOSI, MISO) are correctly set.
- Click the **Verify** button (✓) to compile your code.

The error should now be resolved, and your sketch should compile successfully.

---

<details>
  <summary style="opacity: 0.85;"><b>🔧 image Guide⚙️🛠️ Guide</b></summary><br>
  <div style="display: flex; align-items: center; gap: 10px;" align="center">

![Screenshot (192)](https://github.com/user-attachments/assets/cbbb0235-8173-40d9-ac9e-3af75133c6e2)
![Screenshot (193)](https://github.com/user-attachments/assets/23d51018-f80f-4c13-8764-9d8894f33a01)
![Screenshot (194)](https://github.com/user-attachments/assets/aaa35d04-af4d-4841-a031-9fbc9afd54c0)
![Screenshot (197)](https://github.com/user-attachments/assets/abb470bd-0390-4cca-8a92-99eec72db140)
![Screenshot (198)](https://github.com/user-attachments/assets/f40d094d-52b9-4bc4-9a08-15c6396c0de7)

   </div>
</details>

---

![IMG_20250308_154611](https://github.com/user-attachments/assets/37e0efe7-3013-45dc-ad0c-d28827264f8b)

# 🛠️ Explane the above Code

## 1. Separate Endpoints for Read-Only and Edit Operations

**Read-Only Endpoints:**  
- **File Listing:** Create an endpoint (for example, `/files`) that iterates over the files on the SD card and returns a list.  
- **File Downloads/Video Streaming:** Enhance your current file-serving endpoint to support larger files and video streaming (consider using chunked responses for handling large files).  

**Edit Endpoints:**  
- **File Upload:** Create an endpoint (e.g., `/upload`) where users can upload files.  
- **File Deletion:** Create another endpoint (e.g., `/delete`) that allows authorized users to delete files from the SD card.  

For these editing endpoints, you'll need to implement HTTP authentication so that only users with the correct credentials can modify the file system.

---

## 2. Implementing HTTP Authentication

ESPAsyncWebServer offers built-in methods for handling basic HTTP authentication. For example:

```cpp
nasServer.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
  if (!request->authenticate("admin", "password")) {
    return request->requestAuthentication();
  }
  request->send(200);
}, handleUpload);
```

This same pattern can be applied to your `/delete` endpoint to ensure that only authorized users can perform file deletions.

---

## 3. Handling File Uploads and Deletions

**File Upload:**  
Use the `onFileUpload` callback to handle file uploads. You can open a file on the SD card, write the received data in chunks, and then close the file once the upload is complete. Here’s a simplified example:

```cpp
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (index == 0) {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    // Open file for writing
  }
  // Write data to file here
  
  if (final) {
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
    // Close file
  }
}
```

**File Deletion:**  
For file deletion, you can extract the filename from query parameters, check if it exists on the SD card, and then call `SD.remove(filename)`:

```cpp
nasServer.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request){
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
    request->send(400, "text/plain", "Bad request");
  }
});
```

---

## 4. Running on a Separate Port or IP

While the ESP32 typically runs on a single Wi-Fi interface with one IP, you can simulate separation of concerns by running your NAS-specific endpoints on a different port. For example, you can instantiate another `AsyncWebServer` on port 8080 for NAS operations:

```cpp
AsyncWebServer nasServer(8080);
```

This way, users could access the general file server on port 80 and the NAS admin interface on port 8080. Note that if you truly need separate IPs, you’d require multiple network interfaces—which isn’t typical on the ESP32.

![Screenshot (199)](https://github.com/user-attachments/assets/4128eaf9-dc4c-4e94-bc83-e94df3f61d33)

## ❌ NAS feature not work --- Now fox the Problam
