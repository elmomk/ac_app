/*
 * Echo Server for Heltec LoRa 32 v3
 * 
 * This sketch creates a simple TCP server that echoes received data
 * to the built-in OLED display.
 * 
 * Connect to the server using netcat or similar tool:
 *   nc <board-ip> 8080
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <U8g2lib.h>
#include <Wire.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server settings
const int serverPort = 80;
WiFiServer server(serverPort);

// Initialize OLED display (SSD1306)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Display buffer
String displayText[4];  // 4 lines of text
int currentLine = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize OLED
  u8g2.begin();
  clearDisplay();
  displayMessage("Booting...");
  
  // Connect to WiFi
  connectToWiFi();
  
  // Start the server
  server.begin();
  displayMessage("Server started");
  delay(1000);
  
  // Show IP address
  String ipMsg = "IP: " + WiFi.localIP().toString();
  displayMessage(ipMsg.c_str());
  delay(2000);
  
  // Clear and show initial message
  clearDisplay();
  addLine("Waiting for");
  addLine("connections...");
}

void loop() {
  // Check for client connections
  WiFiClient client = server.available();
  
  if (client) {
    displayMessage("Client connected");
    
    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        
        if (line.length() > 0) {
          // Add the received line to the display
          addLine("> " + line);
          
          // Echo back to client
          client.println("Echo: " + line);
        }
      }
    }
    
    client.stop();
    addLine("Client disconnected");
  }
  
  // Small delay to prevent WDT resets
  delay(10);
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  displayMessage("Connecting to WiFi...");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    String msg = "Connected: ";
    msg += WiFi.localIP().toString();
    displayMessage(msg.c_str());
    delay(1000);
  } else {
    displayMessage("WiFi Failed!");
    while(1) delay(1000);  // Halt if no WiFi
  }
}

void displayMessage(const char* message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 12, message);
  u8g2.sendBuffer();
}

void clearDisplay() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  for (int i = 0; i < 4; i++) {
    displayText[i] = "";
  }
  currentLine = 0;
}

void addLine(String text) {
  // Shift all lines up
  for (int i = 3; i > 0; i--) {
    displayText[i] = displayText[i-1];
  }
  displayText[0] = text;
  
  // Update display
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  // Draw each line
  for (int i = 0; i < 4; i++) {
    if (displayText[i].length() > 0) {
      u8g2.drawStr(0, 12 + (i * 12), displayText[i].c_str());
    }
  }
  
  u8g2.sendBuffer();
}
