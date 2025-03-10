<div style="display: flex; align-items: center; gap: 20px;" align="center">
  
  # NAS with Raspberry Pi Pico W
  ### 🛠️ The code is under under development, Not use it Now
</br>

  <img src="https://github.com/user-attachments/assets/cc52c438-38f9-40d1-bbde-2d655391de7b" width="48%">
  <img src="https://github.com/user-attachments/assets/501f579e-a40c-4b4f-b6f8-b55ade17408c" width="48%">
</div>

<details>
  <summary style="opacity: 0.85;">Check Your PI is OK?</b></summary><br>

### Blinking LED

```cpp
void setup() {
    pinMode(LED_BUILTIN, OUTPUT); // LED on Pico W
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH); // Turn LED ON
    delay(500);
    digitalWrite(LED_BUILTIN, LOW); // Turn LED OFF
    delay(500);
}
```
</details>

## **Raspberry Pi Pico W File Download with SD Card Logging (Arduino IDE)**
set up **file downloads, logging, and live log preview** on a **Raspberry Pi Pico W** using an **SD card module**.  

### **📌 Features**
✅ **File download support (with correct filename & format)**  
✅ **Log file downloads & errors to SD card**  
✅ **Live log preview via a web page**  
✅ **Works on Raspberry Pi Pico W with Arduino IDE**  

---

## **🛠️ Hardware & Connections (Pico W + SD Card Module)**
You'll need:  
- **Raspberry Pi Pico W**  
- **MicroSD Card Module** (SPI-based)  
- **MicroSD Card (formatted as FAT32)**  
- **Jumper wires**  

### **📝 Wiring Diagram (SPI)**
| **SD Card Module** | **Pico W (SPI Pins)** |
|-------------------|----------------------|
| VCC             | 3.3V or 5V (Pico W VBUS) |
| GND             | GND |
| MISO            | GPIO16 (SPI0 MISO) |
| MOSI            | GPIO19 (SPI0 MOSI) |
| SCK             | GPIO18 (SPI0 SCK) |
| CS (Chip Select) | GPIO17 |

</br>
  <img src="https://github.com/user-attachments/assets/da67dbeb-43d0-499c-a6a8-7c8213f99893" width="48%">
  <img src="https://github.com/user-attachments/assets/5458a463-bbd4-46cf-be75-0567643aa84e" width="48%">
</div>

</br>
</br>

<details>
  <summary style="opacity: 0.85;"></b>✅ install libraries before compiling</summary><br>

Below is a complete, revised sketch for the Raspberry Pi Pico W that includes all the requested features (Wi‑Fi AP mode, serving an index page, file listing, upload/download, and deletion) using the Pico‑friendly SDFS library and AsyncWebServer_RP2040W. Note that the SDFS library’s open() function expects the file mode as a string (e.g. "r", "w") rather than an integer flag like FILE_WRITE. Also, when opening the root directory for listing files, you must specify the mode (typically "r" for reading).

Make sure you have installed the **AsyncWebServer_RP2040W** and **SDFS** libraries before compiling.

---

![Screenshot (212)](https://github.com/user-attachments/assets/a374e3ef-6b13-4e96-865f-7545433479d8)
![Screenshot (213)](https://github.com/user-attachments/assets/3908cd1e-5ee3-4b59-8606-c28be923924e)
![Screenshot (214)](https://github.com/user-attachments/assets/205e75cd-a315-4a1b-9277-f17f006a9cfb)
![Screenshot (215)](https://github.com/user-attachments/assets/bdb8e6f9-9ec5-449b-a805-9daff7fd3bd5)

</details>
  
---

```cpp
#include <WiFi.h>                  // Wi-Fi library for Pico W
#include <SPI.h>                   // SPI library for SD card communication
#include <SDFS.h>                  // Pico‑specific SD library (implements fs::FS)
#include <AsyncWebServer_RP2040W.h>  // Asynchronous web server library

// Define SD Card pins for SPI0
#define SD_CS    17
#define SD_SCK   18
#define SD_MOSI  19
#define SD_MISO  16

// Wi-Fi Access Point credentials
const char *ap_ssid = "Space";
const char *ap_password = "Mahapatra";

// Create the AsyncWebServer object on port 80
AsyncWebServer server(80);

// Global file object for handling file uploads
File uploadFile;

// Create a filesystem reference from SDFS
// (Named "sdFS" to avoid conflict with the "fs" namespace)
fs::FS &sdFS = SDFS;

// Function to handle file uploads via HTTP POST to "/upload"
// This function is called repeatedly as chunks of the file are received.
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (index == 0) {
    // For the first chunk, build the full file path and open the file in write mode.
    // Using "w" opens (and truncates if it exists) the file for writing.
    String filePath = "/" + filename;
    uploadFile = sdFS.open(filePath.c_str(), "w");
  }
  if (uploadFile) {
    // Write the current chunk to the file.
    uploadFile.write(data, len);
  }
  if (final) {
    // When the last chunk is received, close the file and send a response.
    uploadFile.close();
    request->send(200, "text/plain", "Upload Complete");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);  // Wait for the serial connection

  // Initialize SPI0 for SD card communication using defined pins.
  SPI.setRX(SD_MISO);
  SPI.setTX(SD_MOSI);
  SPI.setSCK(SD_SCK);
  SPI.begin();

  // Initialize the SD card using SDFS. (SDFS.begin() takes no parameters.)
  if (!SDFS.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  }
  Serial.println("SD Card Initialized Successfully");

  // Set up the Pico W as a Wi-Fi Access Point
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("\nWiFi AP Started!");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Serve the index page at the root URL "/"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (sdFS.exists("/index.html")) {
      // Send the file if it exists on the SD card.
      request->send(sdFS, "/index.html", "text/html");
    } else {
      request->send(404, "text/plain", "index.html not found");
    }
  });

  // List all files on the SD card (accessed via "/files")
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileList = "<h2>File List</h2><ul>";
    // Open the root directory in read mode ("r")
    File root = sdFS.open("/", "r");
    if (root) {
      File file = root.openNextFile();
      while (file) {
        fileList += "<li><a href='/download?file=" + String(file.name()) + "'>" + String(file.name()) +
                    "</a> <button onclick='deleteFile(\"" + String(file.name()) + "\")'>Delete</button></li>";
        file = root.openNextFile();
      }
    }
    fileList += "</ul>";
    request->send(200, "text/html", fileList);
  });

  // Download a file from the SD card (accessed via "/download?file=filename")
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String filename = "/" + request->getParam("file")->value();
      if (sdFS.exists(filename)) {
        // Create a response that streams the file for download.
        AsyncWebServerResponse *response = request->beginResponse(sdFS, filename, "application/octet-stream");
        response->addHeader("Content-Disposition", "attachment; filename=\"" + request->getParam("file")->value() + "\"");
        request->send(response);
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      request->send(400, "text/plain", "Missing file parameter");
    }
  });

  // Delete a file from the SD card (accessed via "/delete?file=filename")
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String filename = "/" + request->getParam("file")->value();
      if (sdFS.exists(filename)) {
        sdFS.remove(filename);
        request->send(200, "text/plain", "File Deleted");
      } else {
        request->send(404, "text/plain", "File not found");
      }
    }
  });

  // Handle file uploads via HTTP POST to "/upload"
  // The file data is streamed to the handleUpload() function.
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    // This callback is called once the file upload is complete.
    request->send(200, "text/plain", "Upload Complete");
  }, handleUpload);

  // Start the web server
  server.begin();
}

void loop() {
  // No code is needed here because AsyncWebServer handles requests in the background.
}
```

---

### **Explanation**

1. **SD Card Initialization & SPI Setup:**  
   The SPI interface is configured with your specified pins, and the SD card is mounted using the SDFS library.  
   *If the SD card fails to mount, the sketch prints an error and halts further execution.*

2. **Wi‑Fi Access Point:**  
   The Pico W is set up as an access point using the provided SSID and password. The AP’s IP address is printed to the Serial Monitor.

3. **Web Server Endpoints:**  
   - **Root ("/")**: Serves `index.html` from the SD card if available.  
   - **"/files"**: Lists all files in the SD card’s root directory, with download and delete buttons.  
   - **"/download"**: Allows downloading a file from the SD card.  
   - **"/delete"**: Deletes a specified file from the SD card.  
   - **"/upload"**: Accepts file uploads in chunks; the `handleUpload` function writes these chunks to a new file.

4. **File Mode Strings:**  
   Instead of using numeric flags, the code now uses standard C‑string modes ("w" for writing and "r" for reading) to open files.

5. **Filesystem Reference:**  
   A filesystem reference (`sdFS`) is created from SDFS. This reference is used throughout the sketch for file operations, making it easier to switch to another filesystem in the future if needed.

---

### **Next Steps**

1. **Hardware Check:**  
   Verify that your SD card module is wired correctly to the Pico W as per your configuration.

2. **Library Installation:**  
   Ensure that the **AsyncWebServer_RP2040W** and **SDFS** libraries are installed in your Arduino IDE.

3. **Upload & Test:**  
   - Upload the sketch to your Pico W.
   - Connect a device to the Wi‑Fi AP (SSID: "Space", Password: "Mahapatra").
   - In a web browser, navigate to the Pico W’s IP (e.g., `http://192.168.4.1/`).
   - Test the file listing, upload, download, and deletion functionalities.

---

[![pico-1s](https://github.com/user-attachments/assets/5efcebda-6d8e-4b36-8bda-87485b911a84)](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#documentation)
`Click the above img to see the Documentation`

<p align="center">
  <a href="public/picow-pinout.svg">
    <img src="https://github.com/user-attachments/assets/9504f339-d0c8-451c-82a5-c8d9e9602908" width="60%" style="margin-right: 10px;"/>
  </a>
  <a href="https://github.com/akashdip2001/NAS-with-ESP32/tree/main/public">
    <img src="https://github.com/user-attachments/assets/2e3de8c8-0590-4552-b06e-2ea3b868447c" alt="Raspberry pi" width="35%" style="margin-left: 10px;"/>
  </a>
</p>

`click the img to see in HD`

---

### **Comparison: RP2350 vs RP2040**
| Feature         | **RP2350** | **RP2040** |
|---------------|-----------|-----------|
| **Processor** | Dual-core Arm Cortex-M33 | Dual-core Arm Cortex-M0+ |
| **Clock Speed** | Up to 250 MHz | Up to 133 MHz |
| **RAM** | 1MB SRAM | 264KB SRAM |
| **Flash Memory** | External QSPI Flash | External QSPI Flash |
| **Floating-Point Unit (FPU)** | Yes (with DSP instructions) | No |
| **Security Features** | TrustZone, Secure Boot, AES encryption | None |
| **GPIO Pins** | Similar to RP2040 | 30 multifunctional GPIO |
| **Analog Inputs** | 8 ADC channels | 4 ADC channels |
| **USB Support** | USB 2.0 High-Speed (480 Mbps) | USB 1.1 (12 Mbps) |
| **Connectivity** | None (requires external Wi-Fi/Bluetooth) | None (requires external module) |
| **Power Efficiency** | More efficient due to Cortex-M33 | Efficient but lower power than RP2350 |
| **Use Cases** | Advanced embedded applications, security-focused IoT, DSP processing | General-purpose embedded projects, hobbyist electronics |

---
