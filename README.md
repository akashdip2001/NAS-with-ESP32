# NAS-with-ESP32
Network-attached storage with ESP WROOM 32

## 🛜 ESP connect to WIFI wifi access point

![IMG20250308224147](https://github.com/user-attachments/assets/990f521b-1cbf-4b56-bc4b-e6813886b94f)
<details>
  <summary style="opacity: 0.85;">❌ Testing Code with Authentication</b></summary><br>

![SetUP](https://github.com/user-attachments/assets/756d28b0-82b4-49c3-b0cf-9cf150bb8596)

# Testing Code `with Authentication` (NAS_32_with_Authentication.ino)

### // I try to add Authentication when user try to UPLOAD & DELETE files
### // no need passward for Download & view the files
### // But the code have the Authentication problame
### // ❌ So, I cant'n continew the code and Re-Write a fresh code without any Authentication

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

<div style="display: flex; align-items: center; gap: 10px;" align="center">
  
### 👆 [Download the Adrino file](NAS_32_with_Authentication.ino)
*this code include all above features
</div>

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

</details>
  
<details>
  <summary style="opacity: 0.85;">✅ Download Required Libraries</b></summary><br>

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

</details>

</br>

[<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/78ad34a3-1575-4b14-a7eb-0682fe62a2e0" />](https://tcsglobal.udemy.com/course/data-engineering-101-the-beginners-guide/learn/lecture/43134006#overview)

</br>

---

<div style="display: flex; align-items: center; gap: 10px;" align="center">
  
# ⭐ NAS System ⭐
### ⚙️ ESP-32 not as a ACCESS Point ⚙️
</div>

---
⚙️ When you power on ESP32 it's try to connect to a ACCESS point (mobile hotsport or Router)

![Screenshot (202)](https://github.com/user-attachments/assets/9c4ec83e-a8ec-4040-ab89-96fe7df392c3)

✅ **ESP32 as a NAS server** (File storage, upload, delete, list)  
✅ **Drag & Drop File Upload**  
✅ **Auto-detect and serve `index.html` if present**  
✅ **Download & Delete files easily from the web interface**  
✅ **No authentication prompts** (No password required)  

---

### 📌 **Code** (NAS_32_connect_with_access_point.ino)

```cpp
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
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

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
```

<div style="display: flex; align-items: center; gap: 10px;" align="center">
  
### 👆 [Download the Adrino file](NAS_32_connect_with_access_point.ino)
*this code include all above features
</div>

---

### 📌 **Upload & File Management Web UI (`index.html`)**
Upload this file to the SD card as `/index.html`. through USB

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 NAS</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        #drop-area { border: 2px dashed #ccc; padding: 20px; margin: 20px; }
        .hidden { display: none; }
        button { padding: 10px; margin: 10px; cursor: pointer; }
    </style>
</head>
<body>
    <h1>ESP32 NAS System</h1>
    
    <div id="drop-area">
        <p>Drag & Drop Files Here</p>
        <input type="file" id="fileInput" multiple>
        <button onclick="uploadFiles()">Upload</button>
    </div>

    <h2>Available Files</h2>
    <div id="file-list">Loading...</div>

    <script>
        document.addEventListener("DOMContentLoaded", loadFiles);

        function loadFiles() {
            fetch("/files")
                .then(response => response.text())
                .then(data => document.getElementById("file-list").innerHTML = data)
                .catch(() => document.getElementById("file-list").innerHTML = "Failed to load files.");
        }

        function uploadFiles() {
            let files = document.getElementById("fileInput").files;
            if (!files.length) return alert("No file selected!");

            for (let file of files) {
                let formData = new FormData();
                formData.append("file", file, file.name);

                fetch("/upload", { method: "POST", body: formData })
                    .then(response => response.text())
                    .then(() => {
                        alert(file.name + " uploaded!");
                        loadFiles();
                    })
                    .catch(() => alert("Upload failed!"));
            }
        }

        function deleteFile(filename) {
            if (!confirm("Delete " + filename + "?")) return;

            fetch("/delete?file=" + filename)
                .then(response => response.text())
                .then(() => {
                    alert(filename + " deleted!");
                    loadFiles();
                })
                .catch(() => alert("Delete failed!"));
        }

        // Drag & Drop Upload Feature
        let dropArea = document.getElementById("drop-area");
        dropArea.addEventListener("dragover", (event) => event.preventDefault());
        dropArea.addEventListener("drop", (event) => {
            event.preventDefault();
            document.getElementById("fileInput").files = event.dataTransfer.files;
        });
    </script>
</body>
</html>
```
<div style="display: flex; align-items: center; gap: 10px;" align="center">
  
### 👆 [Download the html file](index.html)
</div>

---

</br>

https://github.com/user-attachments/assets/6a2e7d65-608a-420c-a2bd-98cd0c5588f5

</br>

### 📌 **How to Use This ESP32 NAS System**
1. **Flash the ESP32 Code** to your ESP-WROOM-32.
2. **Connect to Wi-Fi** using your **SSID & Password**.
3. **Insert an SD Card** with an `index.html` file (above) or create a blank one.
4. **Access ESP32 NAS:**  
   - Open a browser and go to **`http://ESP32-IP/`** (Find the IP from Serial Monitor).
   - The **web UI** will show up where you can **upload, download, and delete** files.

---

![IMG20250308221709](https://github.com/user-attachments/assets/b1c8eb0d-6575-43b2-9c65-146dc0eb7df7)
[![NAS System](https://github.com/user-attachments/assets/45ffbb73-7364-4d2a-aaf8-0b6f8698d352)](https://youtu.be/4c-fYHE_p90)
`click the above image ⬆ to see the YouTube demonstration video`

---
---

<div style="display: flex; align-items: center; gap: 10px;" align="center">
  
## ⭐ ESP_32 as ACCESS Point⚙️ ⭐
</div>

---

## ⚙️🛜 ESP create a WIFI access point

https://github.com/user-attachments/assets/364b583e-49a3-4190-b6eb-6cd866269ae4

- ### ✅ create own WIFI access poin.
- ### ✅ Fix IP Address

</br>

### 📌 **Code** (NAS_32_create_access_point.ino)

```cpp
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>

// Define SD Card pins
#define SD_CS    5
#define SD_SCK   18
#define SD_MOSI  23
#define SD_MISO  19

// Wi-Fi Access Point credentials
const char* ap_ssid = "Space";
const char* ap_password = "Mahapatra";

// Web server on port 80
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

  // Set up ESP32 as an Access Point (AP)
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("\nWiFi AP Started!");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

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
```

---

### ⬇️ **Fix the IP Address**
By default, the ESP32 assigns itself **`192.168.4.1`** when running as an Access Point. However, To make it explicit and ensure it **always** uses this IP, modify with **WiFi.softAPConfig()** function.

---

### 📝 **Updated Code (With Fixed IP)**
Replace **setup()** function with this updated version:
```cpp
void setup() {
  Serial.begin(115200);

  // Static IP Address for ESP32 in AP mode
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  // Set ESP32 as an Access Point
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  
  Serial.println("\nWiFi AP Started!");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  server.begin();
}
```

### **Fixes the IP to `192.168.4.1`** permanently    

---

# ✅ Download `File NAme` fix

```cpp
// Download files
server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
        String filename = "/" + request->getParam("file")->value();
        if (SD.exists(filename)) {
            AsyncWebServerResponse *response = request->beginResponse(SD, filename, "application/octet-stream");
            response->addHeader("Content-Disposition", "attachment; filename=\"" + request->getParam("file")->value() + "\"");
            request->send(response);
        } else {
            request->send(404, "text/plain", "File not found");
        }
    } else {
        request->send(400, "text/plain", "Missing file parameter");
    }
});
```

---

### also download from URl using `query parameters`

Provide the file name in the request URL.  
For example, if you want to download `"example.pdf"`, access the URL like this:  

```arduino
http://192.168.4.1/download?file=example.pdf
```
---

<div style="display: flex; align-items: center; gap: 10px;" align="center">
 
## 👆 [Just Download the Adrino UNO file and use](NAS_32_create_access_point.ino)
*all above feature include
</div>

---

# 🛠️ Updated Code for Logging & Live Preview (optional)

✅ **Stores logs in an SD card file (e.g., `/logs.txt`).**  
✅ **Serves a live preview of logs via a web page (`/logs`).**  
✅ **Updates logs dynamically using Server-Sent Events (SSE).**  

```cpp
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <SD.h>

AsyncWebServer server(80);
AsyncEventSource events("/events");  // For live updates

// Function to log messages
void logMessage(String message) {
    File logFile = SD.open("/logs.txt", FILE_APPEND);
    if (logFile) {
        logFile.println(message);
        logFile.close();
    }
}

// Handle file downloads with logging
server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
        String filename = "/" + request->getParam("file")->value();
        if (SD.exists(filename)) {
            AsyncWebServerResponse *response = request->beginResponse(SD, filename, "application/octet-stream");
            response->addHeader("Content-Disposition", "attachment; filename=\"" + request->getParam("file")->value() + "\"");
            request->send(response);
            
            // Log download event
            logMessage("File Downloaded: " + filename);
            events.send(("Downloaded: " + filename).c_str(), "log", millis());  // Send live update
        } else {
            request->send(404, "text/plain", "File not found");
            logMessage("Download Failed: " + filename + " (File not found)");
            events.send(("Download Failed: " + filename).c_str(), "log", millis());
        }
    } else {
        request->send(400, "text/plain", "Missing file parameter");
        logMessage("Download Error: Missing file parameter");
        events.send("Download Error: Missing file parameter", "log", millis());
    }
});

// Serve logs as a web page with live updates
server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
        "<html><head><script>"
        "var evtSource = new EventSource('/events');"
        "evtSource.onmessage = function(event) {"
        "document.getElementById('log').innerHTML += event.data + '<br>'; };"
        "</script></head><body>"
        "<h2>Live Logs</h2><div id='log'></div></body></html>");
});

// Provide a static file for logs (to download full logs)
server.on("/logs.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/logs.txt", "text/plain");
});

server.addHandler(&events);  // Attach event handler

void setup() {
    Serial.begin(115200);
    if (!SD.begin()) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    server.begin();
    logMessage("Server Started.");
}

void loop() {
    // Nothing needed here, async server handles everything
}
```

---

1. **Persistent Logs:** Stored on SD card.  
   **Logs Are Saved in `/logs.txt`:**  
   - Every download request (success or failure) is stored.  

2. **Live Log Preview (`/logs`):** using Server-Sent Events (SSE) 
   - Opens a web page that dynamically updates logs in real-time.  

3. **Full Log File Download (`/logs.txt`):**  
   - Allows downloading complete log history.  

---

📌 **To download a file:**  
```
http://192.168.4.1/download?file=example.pdf
```
📌 **To see live logs:**  
```
http://192.168.4.1/logs
```
📌 **To download full logs:**  
```
http://192.168.4.1/logs.txt
```
---

<img src="connection.jpg">

---

<div style="display: flex; align-items: center; gap: 10px;" align="center">
  
# ⭐ want to [Try in Raspberry-Pi](RaspberryPi.md) ? ⭐
# or, update `NAS 2.O`
</div>

### NAS 2.O

- Update the Web interface
- Mount like a Drive in PC

![Screenshot (605)](https://github.com/user-attachments/assets/ab01625b-59b9-4237-b696-44f3057f231a)


---
