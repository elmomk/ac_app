/*
 * AC Controller Simulation for Home Automation System
 * 
 * This sketch simulates an AC controller that can be controlled via serial commands.
 * It receives commands in the format: "isOn,temperature,mode,fanSpeed"
 * 
 * Example command: "1,24,COOL,HIGH"
 *   - isOn: 1 for ON, 0 for OFF
 *   - temperature: Desired temperature (16-30°C)
 *   - mode: COOL, HEAT, or FAN
 *   - fanSpeed: LOW, MEDIUM, or HIGH
 */

// Define the possible modes
enum Mode {
  COOL,
  HEAT,
  FAN_ONLY
};

// Define the possible fan speeds
enum FanSpeed {
  LOW,
  MEDIUM,
  HIGH
};

// AC State variables
bool isOn = false;
int currentTemperature = 24;  // Default temperature
Mode currentMode = COOL;      // Default mode
FanSpeed currentFanSpeed = MEDIUM; // Default fan speed

// Pin definitions for Heltec LoRa 32 v3
const int RELAY_PIN = 2;          // Controls the AC power relay
const int TEMP_SENSOR_PIN = 36;   // ADC1_CH0 (VP) - Can be used for temperature sensor
const int LED_PIN = 35;           // Built-in LED on Heltec LoRa 32 v3
const int IR_LED_PIN = 17;        // IR transmitter LED (use a suitable GPIO pin)

// Create IR sender object
IRToshibaAC ac(IR_LED_PIN);  // Change to your AC's protocol if not Toshiba

// Heltec LoRa 32 v3 has an OLED display, we can use it for status
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Toshiba.h>  // For Toshiba AC protocol
// Note: You may need to change this include based on your AC brand
// Common options: ir_Toshiba.h, ir_Mitsubishi.h, ir_Gree.h, etc.

// Disable LoRa and Bluetooth to save power and memory
#define DISABLE_LORA
#define CONFIG_BT_ENABLED 0
#define CONFIG_BTDM_CTRL_MODE_BLE_ONLY 0
#define CONFIG_BTDM_CTRL_MODE_BR_EDR_ONLY 0
#define CONFIG_BTDM_CTRL_MODE_BTDM 0

// WiFi credentials - replace with your network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Web server running on port 80
WebServer server(80);

// Initialize OLED display (SSD1306)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Function prototypes
void updateDisplay();
void handleRoot();
void handleSetState();
void handleNotFound();
void startWebServer();
void connectToWiFi();
void sendIRCommand();

void setup() {
  // Disable brownout detector to prevent unexpected resets
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // Initialize serial communication (minimal, just in case)
  Serial.begin(115200);
  
  // Initialize OLED display first for status messages
  u8g2.begin();
  clearDisplay();
  displayMessage("Booting...");
  
  // Disable WiFi sleep mode for better performance
  WiFi.setSleep(false);
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(IR_LED_PIN, OUTPUT);
  
  // Initialize IR sender
  ac.begin();
  ac.on();  // Start with AC on
  ac.setFan(1);  // Start with fan speed 1
  ac.setTemp(24);  // Default temperature
  ac.setMode(kToshibaAcCool);  // Default mode
  
  // Disable unused peripherals to save power
  #ifdef ARDUINO_HELTEC_WIFI_LORA_32_V3
    // Disable LoRa radio
    pinMode(14, INPUT_PULLDOWN);  // LoRa SCK
    pinMode(8, INPUT_PULLDOWN);   // LoRa MISO
    pinMode(9, INPUT_PULLDOWN);   // LoRa MOSI
    pinMode(10, INPUT_PULLDOWN);  // LoRa NSS
    pinMode(13, INPUT_PULLDOWN);  // LoRa RST
    pinMode(12, INPUT_PULLDOWN);  // LoRa DIO0
  #endif
  
  // Initialize OLED display is now done at the start of setup
  
  // Turn off the AC initially
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Connect to WiFi with static IP (optional, uncomment and modify as needed)
  /*
  IPAddress local_IP(192, 168, 1, 100);  // Set your desired static IP
  IPAddress gateway(192, 168, 1, 1);     // Set your gateway IP
  IPAddress subnet(255, 255, 255, 0);    // Set your subnet mask
  IPAddress primaryDNS(8, 8, 8, 8);      // Optional: Set primary DNS
  IPAddress secondaryDNS(8, 8, 4, 4);    // Optional: Set secondary DNS
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Static IP configuration failed, using DHCP");
  }
  */
  
  // Connect to WiFi
  connectToWiFi();
  
  // Configure WiFi parameters for better performance
  esp_wifi_set_ps(WIFI_PS_NONE);  // Disable WiFi power save for better response
  
  // Show connection status
  if (WiFi.status() == WL_CONNECTED) {
    String ip = "IP: " + WiFi.localIP().toString();
    displayMessage(ip.c_str(), 2000);
  } else {
    displayMessage("WiFi Failed!");
  }
  
  // Start web server
  startWebServer();
  
  // Reduce CPU frequency to save power (optional)
  // setCpuFrequencyMhz(80);  // Set to 80MHz instead of default 240MHz
  
  displayMessage("AC Ready");
  delay(1000);
  updateDisplay();
}

void loop() {
  // Handle web server requests with timeout
  static unsigned long lastClientTime = 0;
  server.handleClient();
  
  // Simple watchdog to prevent WiFi disconnections
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 5000) {  // Try to reconnect every 5 seconds
      lastReconnectAttempt = millis();
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
  
  // Check for incoming serial data (kept for debugging but not used in UI)
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    displayMessage("Processing...");
    processCommand(command);
  }
  
  // Update the LED status
  digitalWrite(LED_PIN, isOn ? HIGH : LOW);
  
  // Update the display every 500ms
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 500) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}

void processCommand(String command) {
  // Parse the command string
  // Format: "isOn,temperature,mode,fanSpeed"
  // Example: "1,24,COOL,HIGH"
  
  // Find the positions of the commas
  int firstComma = command.indexOf(',');
  int secondComma = command.indexOf(',', firstComma + 1);
  int thirdComma = command.indexOf(',', secondComma + 1);
  
  if (firstComma == -1 || secondComma == -1 || thirdComma == -1) {
    Serial.println("Error: Invalid command format. Expected: isOn,temperature,mode,fanSpeed");
    return;
  }
  
  // Extract values
  String isOnStr = command.substring(0, firstComma);
  String tempStr = command.substring(firstComma + 1, secondComma);
  String modeStr = command.substring(secondComma + 1, thirdComma);
  String fanSpeedStr = command.substring(thirdComma + 1);
  
  // Convert and validate values
  bool newIsOn = (isOnStr.toInt() == 1);
  int newTemp = tempStr.toInt();
  
  // Validate temperature range (16-30°C is typical for AC units)
  if (newTemp < 16 || newTemp > 30) {
    displayMessage("Error: Temp 16-30C");
    delay(2000);
    return;
  }
  
  // Map mode string to enum
  Mode newMode;
  if (modeStr == "COOL") {
    newMode = COOL;
  } else if (modeStr == "HEAT") {
    newMode = HEAT;
  } else if (modeStr == "FAN") {
    newMode = FAN_ONLY;
  } else {
    Serial.println("Error: Invalid mode. Must be COOL, HEAT, or FAN");
    return;
  }
  
  // Map fan speed string to enum
  FanSpeed newFanSpeed;
  if (fanSpeedStr == "LOW") {
    newFanSpeed = LOW;
  } else if (fanSpeedStr == "MEDIUM") {
    newFanSpeed = MEDIUM;
  } else if (fanSpeedStr == "HIGH") {
    newFanSpeed = HIGH;
  } else {
    Serial.println("Error: Invalid fan speed. Must be LOW, MEDIUM, or HIGH");
    return;
  }
  
  // Update the AC state
  isOn = newIsOn;
  currentTemperature = newTemp;
  currentMode = newMode;
  currentFanSpeed = newFanSpeed;
  
  // Control the relay and send IR command
  digitalWrite(RELAY_PIN, isOn ? HIGH : LOW);
  
  // Send IR command to AC
  sendIRCommand();
  
  // Show update on display
  displayMessage("AC Command Sent");
  updateDisplay();
  
  // Here you would add code to actually control the AC unit
  // based on the new state
}

// Function to update the OLED display
void updateDisplay() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(0, 12);
  u8g2.print("AC Control");
  
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 26);
  u8g2.print("Power: ");
  u8g2.print(isOn ? "ON " : "OFF");
  
  u8g2.setCursor(0, 38);
  u8g2.print("Temp: ");
  u8g2.print(currentTemperature);
  u8g2.print("°C");
  
  u8g2.setCursor(0, 50);
  u8g2.print("Mode: ");
  switch (currentMode) {
    case COOL: u8g2.print("COOL"); break;
    case HEAT: u8g2.print("HEAT"); break;
    case FAN_ONLY: u8g2.print("FAN "); break;
  }
  
  u8g2.setCursor(64, 50);
  u8g2.print("Fan: ");
  switch (currentFanSpeed) {
    case LOW: u8g2.print("LOW "); break;
    case MEDIUM: u8g2.print("MED "); break;
    case HIGH: u8g2.print("HIGH"); break;
  }
  
  // Display WiFi status
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.setCursor(0, 64);
  if (WiFi.status() == WL_CONNECTED) {
    // Show last octet of IP address
    String ip = WiFi.localIP().toString();
    int lastDot = ip.lastIndexOf('.');
    u8g2.print("IP: " + ip.substring(lastDot + 1));
  } else {
    u8g2.print("WiFi Disconnected!");
  }
  
  u8g2.sendBuffer();
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  
  // Clear display and show connection status
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Connecting to");
  u8g2.drawStr(0, 25, ssid);
  u8g2.sendBuffer();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { // Try for 10 seconds
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED while connecting
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to WiFi. IP address: ");
    Serial.println(WiFi.localIP());
    
    // Blink LED 3 times to indicate successful connection
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  } else {
    Serial.println("\nFailed to connect to WiFi. Please check credentials.");
    // Keep LED on to indicate connection failure
    digitalWrite(LED_PIN, HIGH);
  }
}

void startWebServer() {
  // Define endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/set-state", HTTP_POST, handleSetState);
  server.onNotFound(handleNotFound);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>AC Controller</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:0 auto;padding:20px;}";
  html += ".container{background:#f5f5f5;padding:20px;border-radius:8px;}";
  html += "button{padding:10px 20px;margin:5px;font-size:16px;}";
  html += ".power-btn{background-color:#4CAF50;color:white;border:none;}";
  html += ".power-btn.off{background-color:#f44336;}";
  html += "</style></head><body>";
  html += "<div class='container'><h1>AC Controller</h1>";
  
  // Current status
  html += "<h2>Current Status</h2>";
  html += "<p>Power: <strong>" + String(isOn ? "ON" : "OFF") + "</strong></p>";
  html += "<p>Temperature: <strong>" + String(currentTemperature) + "°C</strong></p>";
  
  // Power control
  html += "<h2>Power</h2>";
  html += "<button class='power-btn " + String(isOn ? "" : "off") + "' onclick='togglePower()'>" + String(isOn ? "TURN OFF" : "TURN ON") + "</button>";
  
  // Temperature control
  html += "<h2>Temperature</h2>";
  html += "<div><button onclick='changeTemp(-1)'>-</button> ";
  html += "<span id='temp'>" + String(currentTemperature) + "</span>°C ";
  html += "<button onclick='changeTemp(1)'>+</button></div>";
  
  // Mode selection
  html += "<h2>Mode</h2>";
  html += "<div>";
  html += "<button onclick='setMode(\"COOL\")' " + (currentMode == COOL ? "style='background:#4CAF50;color:white;'" : "") + ">Cool</button>";
  html += "<button onclick='setMode(\"HEAT\")' " + (currentMode == HEAT ? "style='background:#4CAF50;color:white;'" : "") + ">Heat</button>";
  html += "<button onclick='setMode(\"FAN\")' " + (currentMode == FAN_ONLY ? "style='background:#4CAF50;color:white;'" : "") + ">Fan Only</button>";
  html += "</div>";
  
  // Fan speed
  html += "<h2>Fan Speed</h2>";
  html += "<div>";
  html += "<button onclick='setFanSpeed(\"LOW\")' " + (currentFanSpeed == LOW ? "style='background:#4CAF50;color:white;'" : "") + ">Low</button>";
  html += "<button onclick='setFanSpeed(\"MEDIUM\")' " + (currentFanSpeed == MEDIUM ? "style='background:#4CAF50;color:white;'" : "") + ">Medium</button>";
  html += "<button onclick='setFanSpeed(\"HIGH\")' " + (currentFanSpeed == HIGH ? "style='background:#4CAF50;color:white;'" : "") + ">High</button>";
  html += "</div>";
  
  // JavaScript for handling UI updates
  html += "<script>";
  html += "function togglePower() {";
  html += "  fetch('/api/set-state', {method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({isOn:!" + String(isOn) + "})})";
  html += "  .then(response => window.location.reload());";
  html += "}";
  
  html += "function changeTemp(delta) {";
  html += "  const newTemp = parseInt(document.getElementById('temp').textContent) + delta;";
  html += "  if (newTemp >= 16 && newTemp <= 30) {";
  html += "    fetch('/api/set-state', {method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({temperature:newTemp})})";
  html += "    .then(response => window.location.reload());";
  html += "  }";
  html += "}";
  
  html += "function setMode(mode) {";
  html += "  fetch('/api/set-state', {method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({mode:mode})})";
  html += "  .then(response => window.location.reload());";
  html += "}";
  
  html += "function setFanSpeed(speed) {";
  html += "  fetch('/api/set-state', {method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({fanSpeed:speed})})";
  html += "  .then(response => window.location.reload());";
  html += "}";
  html += "</script>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleSetState() {
  if (server.hasArg("plain") == false) {
    server.send(400, "text/plain", "Bad Request: No data received");
    return;
  }
  
  String body = server.arg("plain");
  
  // Parse JSON (simplified - in production, use a proper JSON parser)
  if (body.indexOf("isOn") != -1) {
    isOn = body.indexOf("true") != -1;
  }
  
  if (body.indexOf("temperature") != -1) {
    int start = body.indexOf("temperature") + 12;
    int end = body.indexOf(',', start);
    if (end == -1) end = body.indexOf('}', start);
    if (end > start) {
      String tempStr = body.substring(start, end);
      int newTemp = tempStr.toInt();
      if (newTemp >= 16 && newTemp <= 30) {
        currentTemperature = newTemp;
      }
    }
  }
  
  if (body.indexOf("mode") != -1) {
    if (body.indexOf("COOL") != -1) currentMode = COOL;
    else if (body.indexOf("HEAT") != -1) currentMode = HEAT;
    else if (body.indexOf("FAN") != -1) currentMode = FAN_ONLY;
  }
  
  if (body.indexOf("fanSpeed") != -1) {
    if (body.indexOf("LOW") != -1) currentFanSpeed = LOW;
    else if (body.indexOf("MEDIUM") != -1) currentFanSpeed = MEDIUM;
    else if (body.indexOf("HIGH") != -1) currentFanSpeed = HIGH;
  }
  
  // Update the relay based on power state
  digitalWrite(RELAY_PIN, isOn ? HIGH : LOW);
  
  // Update the display
  updateDisplay();
  
  // Return the current state
  String response = "{\"status\":\"success\",\"state\":{\"isOn\":" + String(isOn ? "true" : "false") + 
                   ",\"temperature\":" + String(currentTemperature) + 
                   ",\"mode\":\"" + (currentMode == COOL ? "COOL" : currentMode == HEAT ? "HEAT" : "FAN") + 
                   "\",\"fanSpeed\":\"" + (currentFanSpeed == LOW ? "LOW" : currentFanSpeed == MEDIUM ? "MEDIUM" : "HIGH") + "\"}}";
  
  server.send(200, "application/json", response);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void displayMessage(const char* message, int displayTime = 1000) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  // Split long messages into two lines if needed
  int split = strlen(message) / 2;
  if (split > 0 && split < strlen(message)) {
    // Find the nearest space to split on
    while (split < strlen(message) && message[split] != ' ') {
      split++;
    }
    
    char line1[64] = {0};
    char line2[64] = {0};
    strncpy(line1, message, split);
    strncpy(line2, message + split + 1, sizeof(line2) - 1);
    
    u8g2.drawStr(0, 12, line1);
    u8g2.drawStr(0, 30, line2);
  } else {
    u8g2.drawStr(0, 20, message);
  }
  
  u8g2.sendBuffer();
  if (displayTime > 0) {
    delay(displayTime);
  }
}

void clearDisplay() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void sendIRCommand() {
  // Map our mode to IR library's mode
  switch (currentMode) {
    case COOL:
      ac.setMode(kToshibaAcCool);
      break;
    case HEAT:
      ac.setMode(kToshibaAcHeat);
      break;
    case FAN_ONLY:
      ac.setMode(kToshibaAcFan);
      break;
  }
  
  // Map fan speed
  switch (currentFanSpeed) {
    case LOW:
      ac.setFan(1);
      break;
    case MEDIUM:
      ac.setFan(2);
      break;
    case HIGH:
      ac.setFan(3);
      break;
  }
  
  // Set power state and temperature
  if (isOn) {
    ac.on();
  } else {
    ac.off();
  }
  ac.setTemp(currentTemperature);
  
  // Send the IR command
  ac.send();
  
  // Small delay to let the IR LED recover
  delay(100);
}
}
