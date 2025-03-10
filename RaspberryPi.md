<div style="display: flex; align-items: center; gap: 20px;" align="center">
  
  # NAS with Raspberry Pi Pico W
  ### 🛠️ The code is under under development, Not use it Now
</br>
  <img src="https://github.com/user-attachments/assets/cc52c438-38f9-40d1-bbde-2d655391de7b" width="48%">
  <img src="https://github.com/user-attachments/assets/501f579e-a40c-4b4f-b6f8-b55ade17408c" width="48%">
</div>

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

---

## **📥 Install Required Arduino Libraries**
In **Arduino IDE**, install these libraries via **Library Manager**:
1. **WiFi** (for Raspberry Pi Pico W)
2. **AsyncTCP**  
3. **ESPAsyncWebServer**  
4. **SD (SD.h)**  

---

## **📝 Complete Arduino Code**
This code:
- **Hosts a web server** on Pico W.
- **Allows file downloads** with correct filename & format.
- **Logs events to `/logs.txt`** on the SD card.
- **Provides a live log preview** via `/logs`.

```cpp
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <SD.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

#define SD_CS 17  // Chip Select (CS) pin for SD card (Pico GPIO17)
AsyncWebServer server(80);
AsyncEventSource events("/events");  // Event source for live logs

// Function to log messages to SD card
void logMessage(String message) {
    File logFile = SD.open("/logs.txt", FILE_APPEND);
    if (logFile) {
        logFile.println(message);
        logFile.close();
    }
}

// Handle file downloads
server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
        String filename = "/" + request->getParam("file")->value();
        if (SD.exists(filename)) {
            AsyncWebServerResponse *response = request->beginResponse(SD, filename, "application/octet-stream");
            response->addHeader("Content-Disposition", "attachment; filename=\"" + request->getParam("file")->value() + "\"");
            request->send(response);
            
            logMessage("File Downloaded: " + filename);
            events.send(("Downloaded: " + filename).c_str(), "log", millis());  // Live update
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

// Serve a live log preview
server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
        "<html><head><script>"
        "var evtSource = new EventSource('/events');"
        "evtSource.onmessage = function(event) {"
        "document.getElementById('log').innerHTML += event.data + '<br>'; };"
        "</script></head><body>"
        "<h2>Live Logs</h2><div id='log'></div></body></html>");
});

// Provide full log file download
server.on("/logs.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/logs.txt", "text/plain");
});

server.addHandler(&events);  // Attach live log handler

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize SD Card
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    Serial.println("SD Card Initialized.");

    // Start Web Server
    server.begin();
    logMessage("Server Started.");
}

void loop() {
    // Nothing needed here, async server handles everything
}
```

---

## **🖥️ How to Use**
1. **Upload the code** to your Raspberry Pi Pico W using Arduino IDE.
2. **Connect your computer/phone to the same WiFi network**.
3. Open a **web browser** and go to:
   - 📌 **Download a file**:  
     ```
     http://your-esp-ip/download?file=example.pdf
     ```
   - 📌 **Live logs (real-time preview)**:  
     ```
     http://your-esp-ip/logs
     ```
   - 📌 **Download full logs**:  
     ```
     http://your-esp-ip/logs.txt
     ```
4. **Check the serial monitor** to see the log events.

---

## **🔗 Additional Notes**
- Make sure **your SD card is formatted as FAT32**.
- **Replace `YOUR_WIFI_SSID` & `YOUR_WIFI_PASSWORD`** with actual WiFi credentials.
- **Pico W’s SPI pins are fixed** (GPIO16-MISO, GPIO17-CS, GPIO18-SCK, GPIO19-MOSI).
- You can **modify the log file path** (`/logs.txt`) as needed.

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
