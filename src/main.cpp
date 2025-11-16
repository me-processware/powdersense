#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "board_config.h"     // ✅ Board-specific pin definitions
// #include "axs5106l_device.h"   // Temporarily disabled. Board has an AXS5106L.
#include <Wire.h>              // For I2C communication with touch controller and ADS1115
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>               // For generic File System access
#include <SPIFFS.h>           // Specifically for SPIFFS functions
#include <WebSocketsServer.h> // For WebSocket communication
#include <ArduinoJson.h>      // For JSON parsing and serialization
#include <Update.h>           // For OTA updates
#ifdef HAS_RGB_LED
  #include <FastLED.h>        // ✅ Only include if board has RGB LED
#endif
#include <DNSServer.h>        // For Captive Portal
#include <Preferences.H>      // For Non-Volatile Storage (NVS)
#include <time.h>             // For NTP time synchronization
#include <Adafruit_ADS1X15.h> // For ADS1115 ADC

// --- LovyanGFX Configuration ---
// ✅ Uses board-specific pins from board_config.h
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;

public:
  LGFX(void)
  {
    { // Configure SPI bus settings
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 10000000;
      cfg.freq_read  = 8000000;
      cfg.pin_sclk = TFT_SCLK;  // ✅ From board_config.h
      cfg.pin_mosi = TFT_MOSI;  // ✅ From board_config.h
      cfg.pin_miso = -1;
      cfg.pin_dc   = TFT_DC;    // ✅ From board_config.h
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // Configure display panel settings
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = TFT_CS;   // ✅ From board_config.h
      cfg.pin_rst          = TFT_RST;  // ✅ From board_config.h
      cfg.pin_busy         = -1;
      cfg.panel_width      = DISPLAY_WIDTH;   // ✅ From board_config.h
      cfg.panel_height     = DISPLAY_HEIGHT;  // ✅ From board_config.h
      cfg.offset_x         = DISPLAY_OFFSET_X; // ✅ From board_config.h
      cfg.offset_y         = DISPLAY_OFFSET_Y; // ✅ From board_config.h
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = false;
      cfg.invert           = DISPLAY_INVERT;  // ✅ Board-specific inversion
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = false;
      _panel_instance.config(cfg);
    }

    { // Configure backlight control
        auto cfg = _light_instance.config();
        cfg.pin_bl = TFT_BL;  // ✅ From board_config.h
        cfg.invert = false;
        cfg.freq = 44100;
        cfg.pwm_channel = 0;
        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};

// --- Pin Definitions ---
// ✅ All pin definitions now come from board_config.h
// This keeps the code clean and board-agnostic

#ifdef HAS_RGB_LED
  CRGB leds[NUM_LEDS];  // ✅ Only define if board has RGB LED
#endif

// Initialize GFX
LGFX gfx;

// ADS1115 ADC for precise voltage measurement
Adafruit_ADS1115 ads;
bool adsInitialized = false;

// --- Double Buffering with Sprite ---
// This eliminates screen flicker by drawing to an off-screen buffer first
LGFX_Sprite canvas(&gfx);  // Create sprite buffer for smooth rendering

// Reference for drawing operations (enables double buffering)
// Use 'display' for all drawing in draw functions
LGFX_Sprite& display = canvas;

// Touch data struct
// touch_data_t touch_data; // Temporarily disabled

// --- Button Pin Definitions ---
// Removed: No longer using physical buttons

// --- Calibration and Measurement Variables ---
// These values will need to be determined through calibration
// For a spring-based potentiometer, 0mm might correspond to POT_MIN_ADC
// and 60mm to POT_MAX_ADC, or vice-versa depending on mounting.
// We'll assume 0mm is the lowest ADC reading and 60mm is the highest.
// IMPORTANT: Calibrate these values with your actual potentiometer!
// These values are now stored per-configuration in the PowderConfig struct.
// potMaxAdc is no longer used in 2-step calibration.
const float MEASUREMENT_RANGE_MM = 60.0; // 60mm travel for Bourns PTA6043-2015DPB103

// Calibration state machine
enum CalibrationState {
  CALIBRATE_NONE,
  CALIBRATE_ZERO_STEP,      // Step 1: Set probe at 0mm
  CALIBRATE_KNOWN_GRAINS_STEP // Step 2: Set probe at known grains
};
CalibrationState currentCalibrationState = CALIBRATE_NONE;
float tempZeroAdc = 0.0; // Temporary storage for zero calibration ADC value
float tempKnownGrainsDepth = 0.0; // Temporary storage for depth during known grains calibration

// --- Moving Average Filter Variables ---
const int ADC_READINGS_COUNT = 5; // Number of readings to average (reduced for faster response)
float adcReadings[ADC_READINGS_COUNT];
int adcReadingsIndex = 0;
bool adcReadingsFilled = false; // Flag to indicate if buffer is initially filled

// --- Powder Weight Conversion Factor ---
// This factor is now stored per-configuration in the PowderConfig struct as 'grainsPerMmFactor'.

// --- Wi-Fi Credentials ---
// These will be loaded from SPIFFS or configured via AP mode
String wifiSsid = "";
String wifiPassword = "";

// Preferences object for NVS
Preferences preferences;

// --- Web Server & WebSocket Server ---
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); // WebSocket server on port 81
DNSServer dnsServer; // For Captive Portal

// --- Global State Variables (for web UI) ---
float currentPowderWeight = 0.0; // Placeholder for actual weight calculation

// --- OTA Update Variables ---
bool isUpdating = false;
String updateType = ""; // "firmware" or "filesystem"
size_t updateSize = 0;
size_t updateReceived = 0;

// --- Configuration Management ---
#define MAX_CONFIGS 20 // Max number of custom configurations
struct PowderConfig {
  char name[32];
  char caliber[24];
  char bulletWeight[10];
  char powderName[24];
  float targetGrain;
  // New fields for per-config calibration
  float potMinAdc;
  float grainsPerMmFactor;
  bool isCalibrated;
};

PowderConfig powderConfigs[MAX_CONFIGS];
int configCount = 0;
int currentConfigIndex = -1; // -1 means no config selected

bool alarmActive = false;
unsigned long systemUptimeMillis = 0;

// Measurement history for stats and chart
struct Measurement {
  time_t timestamp; // Epoch time (seconds since 1970-01-01)
  float weight;
  PowderConfig config; // Store a copy of the config at time of measurement
  bool configWasSet;   // Flag to know if a config was active
};
#define MAX_MEASUREMENTS_HISTORY 100 // Store up to 100 measurements
Measurement measurementHistory[MAX_MEASUREMENTS_HISTORY];
int measurementCount = 0; // Total measurements taken
int sessionMeasurementCount = 0; // Measurements in current session
float minWeight = 0.0; // Initialize with 0.0, will be set on first measurement
float maxWeight = 0.0;    // Initialize with 0.0, will be set on first measurement
float sumWeight = 0.0;

// Session log for tracking completed sessions
struct SessionLog {
  time_t startTime; // Epoch time when session started
  time_t endTime;   // Epoch time when session ended
  int bulletCount;         // Number of bullets created in session
  float totalWeight;       // Total weight measured in session
  // Store measurement indices for this session
  int measurementStartIndex; // Index of first measurement in this session
  int measurementCount;      // Number of measurements in this session
};
#define MAX_SESSION_LOGS 50 // Store up to 50 session logs
SessionLog sessionLogs[MAX_SESSION_LOGS];
int sessionLogCount = 0;
unsigned long currentSessionStartTime = 0; // Track start time of current session
int sessionStartMeasurementIndex = 0; // Track where current session measurements start

// Alarm settings
// Changed default lowThreshold to 0.0 and highThreshold to 100.0
struct AlarmSettings {
  bool enabled;
  float lowThreshold;
  float highThreshold;
} alarmSettings = {true, 0.0, 100.0}; // Initial defaults, will be overwritten by loaded settings or calibration

// --- Auto-Measurement Variables ---
const unsigned long AUTO_MEASURE_COOLDOWN_MS = 10000; // Cooldown period after a measurement (10 seconds)
unsigned long lastAutoMeasureTime = 0; // Timestamp of last auto-measurement
unsigned long autoMeasureTimerStart = 0; // Timer for stable measurement
const float AUTO_MEASURE_TOLERANCE_GRAINS = 0.1; // Tolerance for auto-measurement (+/- grains)
const unsigned long AUTO_MEASURE_STABLE_DURATION_MS = 750; // ⚡ OPTIMIZED - Reduced from 2000ms to 750ms (2.5x faster!)
const unsigned long AUTO_MEASURE_MAX_STABLE_TIME_MS = 30000; // Maximum time to stay in stable state (30 seconds) before resetting timer
bool lastMeasurementTaken = false; // Flag to prevent continuous counting at perfect level
const float RESET_MEASUREMENT_THRESHOLD = 0.1; // Weight must drop below this to allow next measurement

// --- Display Update Variables ---
unsigned long lastDisplayUpdateTime = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 100; // ✅ FASTER - Update display every 100ms (10 times per second)
// Keep track of the current screen to know when to clear fully
enum ScreenState {
  SCREEN_MEASUREMENT,
  SCREEN_CALIBRATION,
  SCREEN_AP_MODE
};
ScreenState currentScreenState = SCREEN_MEASUREMENT; // Default to measurement screen

// --- Touch Button Definitions ---
struct TouchButton {
  int x, y, width, height;
  String label;
  uint16_t color;
  bool enabled;
};

// Touch buttons for main screen
TouchButton calibrateBtn = {10, 280, 80, 30, "CAL", TFT_BLUE, true};
TouchButton profileBtn = {100, 280, 80, 30, "PROF", TFT_GREEN, true};
TouchButton settingsBtn = {190, 280, 80, 30, "SET", TFT_ORANGE, true};

// Touch buttons for calibration screen
// NOTE: These buttons are currently unused as calibration is driven by the Web UI.
// TouchButton zeroBtn = {20, 250, 100, 40, "ZERO", TFT_RED, true};
// TouchButton spanBtn = {140, 250, 100, 40, "SPAN", TFT_YELLOW, true};
// TouchButton backBtn = {260, 250, 50, 40, "BACK", TFT_DARKGREY, true};

// --- WebSocket Update Variables ---
unsigned long lastWebSocketUpdateTime = 0;
const unsigned long WEBSOCKET_UPDATE_INTERVAL_MS = 500; // Update WebSocket every 500ms for faster web UI response

// --- Stable Measurement Variables ---
unsigned long lastGreenLEDTime = 0;
bool wasGreenLED = false;
const unsigned long STABLE_MEASUREMENT_DURATION_MS = 1500; // 1.5 seconds of stable green LED for recording
bool isStable = false; // Declare isStable globally

// --- Function Prototypes ---
void setupDisplay(); // Renamed from setupOLED
float readDepthADC(); // Changed to return ADC value
float calculatePowderWeight(float adcValue); // Changed to accept ADC value
void updateLEDs(float currentWeight); // ✅ Function prototype for RGB LED support
void setupWiFi();
void startAPMode(); // New function for AP mode
void handleRoot();
void handleGetDepth(); // Will be updated to send full state
void handleApiMeasurement(); // New handler for /api/measurement fallback
void handleNotFound();
void sendCurrentStateToClients();
void handleAutoMeasure(); // New function for auto-measurement
// void displayExampleScreen(); // Removed as it's no longer used
void saveSettings(); // Save settings to SPIFFS
float calculateStandardDeviation(); // New function for standard deviation
void loadSettings(); // Load settings from SPIFFS
void saveWiFiCredentials(const String& ssid, const String& password); // Modified to use NVS
void loadWiFiCredentials(); // Modified to use NVS
void handleWiFiConfigSave(); // New function for AP mode config save
void handleWiFiConfigPage(); // New function to serve AP mode config page

// Command handlers (placeholders for now)
void handleZeroCommand();
void handleSDCommand(String action); // New command for SD card management
void handleMeasureCommand();
void handleCalibrateCommand();
void handleSelectConfigCommand(int index);
void handleSaveConfigCommand(JsonObject doc);
void handleDeleteConfigCommand(int index);
void handleSetAlarmsCommand(bool enabled, float low, float high);
void handleAcknowledgeAlarmCommand();
void handleResetSessionCommand();
void handleStartSessionCommand(); // New command
void handleEndSessionCommand(); // New command
void handleExportDataCommand(); // HTTP endpoint for CSV export
void handleExportSessionCommand(); // HTTP endpoint for session CSV export
void handleFactoryResetCommand(); // New command for factory reset
void handleUpdateFirmwareCommand(String type, String filename, size_t size); // New command for OTA updates
void handleUpdateFirmwareCommand(String type, String filename, size_t size); // New command for OTA updates
void handleImportConfigsCommand(JsonArray configs); // New command for importing configurations
void handleExportSessionDetailsCommand(int sessionIndex); // Command for exporting session details


// Calibration wizard functions
void startCalibrationWizard();
void setCalibrationZeroPoint();
void setCalibrationKnownGrains(float knownWeight);
void cancelCalibration();

// New display functions
void drawMeasurementScreen(float weight, bool alarmActive, float lowThreshold, float highThreshold);
void drawCalibrationScreen(CalibrationState state, float currentAdc, float currentWeight); // Changed to currentWeight
void drawAPModeScreen();

// Touch handling functions
void handleTouch();
bool isButtonPressed(TouchButton &btn, int touchX, int touchY);
void drawButton(TouchButton &btn, bool pressed = false);
void handleMainTouch(int x, int y);
// void handleCalibrationTouch(int x, int y); // Removed as calibration is driven by Web UI

// setCaliberName and setPowderName are removed as they are now part of dynamic configurations.


// Helper to get content type for files
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Powder Depth Measurement System...");
  Serial.printf("Board: %s\n", BOARD_NAME);  // ✅ Print board name

  #ifdef HAS_RGB_LED
    Serial.println("Initializing RGB LED...");
    FastLED.addLeds<WS2812B, RGB_LED_PIN, RGB>(leds, NUM_LEDS);  // ✅ RGB color order (not GRB)
    leds[0] = CRGB::Black;
    FastLED.show();
  #else
    Serial.println("No RGB LED on this board");
  #endif

  // Removed: Button Pin Initialization

  // Initialize SPI for the display with non-default pins
  // SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1); // Not needed for software SPI (Arduino_ESP32SPI)

  Serial.println("Initializing Display...");
  setupDisplay(); // Initialize the new display and touch controller
  Serial.println("Display Initialized.");

  // Initialize sprite buffer for double buffering (eliminates flicker!)
  canvas.createSprite(gfx.width(), gfx.height());
  canvas.fillSprite(TFT_BLACK); // ✅ Ensure sprite starts with black background
  Serial.printf("Sprite buffer created: %dx%d pixels\n", gfx.width(), gfx.height());

  // Initialize direct ADC pin as fallback (in case ADS1115 initialization fails)
  pinMode(LEVEL_SENSOR_PIN, INPUT);

  // Initial message on Display - with multiple explicit display calls
  gfx.fillScreen(TFT_BLACK);
  gfx.display();
  gfx.flush();
  delay(100);
  
  gfx.setTextColor(TFT_WHITE, TFT_BLACK);
  gfx.setTextSize(2); // Increased font size
  String initMsg = "... System Initializing ...";
  uint16_t msgWidth = gfx.textWidth(initMsg);
  int initX = (gfx.width() - msgWidth) / 2; // Center horizontally
  gfx.setCursor(initX, 10);
  gfx.println(initMsg);
  gfx.display();
  gfx.flush();
  delay(500);

  Serial.println("Initializing SPIFFS...");
  // First, try to mount. If it fails, format and then then try again.
  if(!SPIFFS.begin(false)){ // Use 'false' here to not format on first attempt
    Serial.println("SPIFFS Mount Failed! Attempting to format...");
    if(SPIFFS.format()){
      Serial.println("SPIFFS formatted successfully. Retrying mount...");
      if(!SPIFFS.begin(false)){ // Try mounting again after format
        Serial.println("SPIFFS Mount Failed even after format! Halting.");
        for(;;); // Don't proceed
      }
    } else {
      Serial.println("SPIFFS Format Failed! Halting.");
      for(;;); // Don't proceed
    }
  }
  Serial.println("SPIFFS mounted successfully");
  Serial.printf("SPIFFS Total: %d bytes, Used: %d bytes\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());

  loadWiFiCredentials(); // Load saved Wi-Fi credentials (now from NVS) - MUST be before loadSettings
  loadSettings(); // Load settings, which now include calibration data

  if (wifiSsid == "" || wifiPassword == "") {
    Serial.println("No WiFi credentials found. Starting AP mode...");
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK); // Add background color
    gfx.setTextSize(2); // Increased font size
    String noWifiMsg = "No WiFi Config!";
    uint16_t noWifiWidth = gfx.textWidth(noWifiMsg);
    int noWifiX = (gfx.width() - noWifiWidth) / 2;
    gfx.setCursor(noWifiX, 10);
    gfx.println(noWifiMsg);
    
    String apMsg = "Starting AP Mode...";
    uint16_t apWidth = gfx.textWidth(apMsg);
    int apX = (gfx.width() - apWidth) / 2;
    gfx.setCursor(apX, 40);
    gfx.println(apMsg);
    
    gfx.setTextSize(1); // Smaller font for details
    String connectMsg = "Connect to:";
    uint16_t connectWidth = gfx.textWidth(connectMsg);
    int connectX = (gfx.width() - connectWidth) / 2;
    gfx.setCursor(connectX, 70);
    gfx.println(connectMsg);
    
    gfx.setTextSize(3); // Larger font for AP name
    String apName = "PowderSense";
    uint16_t apNameWidth = gfx.textWidth(apName);
    int apNameX = (gfx.width() - apNameWidth) / 2;
    gfx.setCursor(apNameX, 100);
    gfx.println(apName);
    
    gfx.setTextSize(3); // Larger font for IP
    String apIP = "192.168.4.1";
    uint16_t apIPWidth = gfx.textWidth(apIP);
    int apIPX = (gfx.width() - apIPWidth) / 2;
    gfx.setCursor(apIPX, 150);
    gfx.println(apIP);
    
    gfx.display(); // Explicitly flush buffer
    delay(500); // Longer delay to ensure text is drawn
    startAPMode();
  } else {
    Serial.println("Connecting to WiFi...");
    setupWiFi();
    Serial.println("WiFi setup complete.");

    // Web Server Routes for STA mode
    server.on("/", HTTP_GET, handleRoot);
    server.on("/depth", HTTP_GET, handleGetDepth); // This will now send full state
    server.on("/api/measurement", HTTP_GET, handleApiMeasurement); // New fallback API endpoint
    server.on("/api/export", HTTP_GET, handleExportDataCommand); // New export endpoint
    server.on("/api/export_session", HTTP_GET, handleExportSessionCommand); // New session export endpoint
    server.onNotFound(handleNotFound); // This will now handle static files too

    server.begin();
    Serial.println("HTTP server started");

    // WebSocket server setup
    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length){
      switch (type) {
        case WStype_DISCONNECTED:
          Serial.printf("[%u] Disconnected!\n", num);
          break;
        case WStype_CONNECTED: {
          IPAddress ip = webSocket.remoteIP(num);
          Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
          // Send current state immediately upon connection
          sendCurrentStateToClients();
        }
          break;
        case WStype_TEXT: { // Added curly braces to create a new scope
          Serial.printf("[%u] get Text: %s\n", num, payload);
          // Parse JSON command from client
          // Use JsonDocument for modern ArduinoJson API
          // Increased size to handle large configuration imports
          DynamicJsonDocument doc(4096); 
          DeserializationError error = deserializeJson(doc, payload);

          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }

          String command = doc["command"];
          if (command == "zero") {
            handleZeroCommand();
          } else if (command == "measure") {
            handleMeasureCommand();
          } else if (command == "calibrate") {
            // Calibration command now takes a step parameter
            String step = doc["step"];
            if (step == "startWizard") { // New command to start the wizard
              startCalibrationWizard();
            } else if (step == "setZeroPoint") { // New command for zero point
              setCalibrationZeroPoint();
            } else if (step == "setKnownGrains") { // New command for known grains
              float knownWeight = doc["knownWeight"];
              setCalibrationKnownGrains(knownWeight);
            } else if (step == "cancel") {
              cancelCalibration();
            }
            handleCalibrateCommand(); // Call to update UI with new state
          } else if (command == "selectConfig") {
            int index = doc["index"];
            handleSelectConfigCommand(index);
          } else if (command == "saveConfig") {
            JsonObject data = doc["data"];
            handleSaveConfigCommand(data);
          } else if (command == "deleteConfig") {
            int index = doc["index"];
            handleDeleteConfigCommand(index);
          } else if (command == "setAlarms") {
            bool enabled = doc["enabled"];
            float low = doc["lowThreshold"];
            float high = doc["highThreshold"];
            handleSetAlarmsCommand(enabled, low, high);
          } else if (command == "acknowledgeAlarm") {
            handleAcknowledgeAlarmCommand();
          } else if (command == "resetSession") {
            handleResetSessionCommand();
          } else if (command == "startSession") {
            handleStartSessionCommand();
          } else if (command == "endSession") {
            handleEndSessionCommand();
          } else if (command == "factoryReset") { // New command
            handleFactoryResetCommand();
          } else if (command == "importConfigs") { // New command for importing configurations
            JsonArray configs = doc["configs"];
            handleImportConfigsCommand(configs);
          } else if (command == "updateFirmware") { // New command for OTA updates
            String type = doc["type"];
            String filename = doc["filename"];
            size_t size = doc["size"];
            handleUpdateFirmwareCommand(type, filename, size);
          } else if (command == "setSetting") { // New command to set a generic setting
            // This command is currently unused after removing autoMeasureInsertionWeight.
            // Kept as a placeholder for future settings.
            String key = doc["key"];
            Serial.printf("Received setSetting for key: %s (no action)\n", key.c_str());
            sendCurrentStateToClients(); // Send updated state back to client
          }
          // Add more command handlers as needed
          break;
        } // End of WStype_TEXT scope
        case WStype_BIN: {
          if (isUpdating) {
            size_t len = length;
            if (updateReceived + len > updateSize) {
              len = updateSize - updateReceived; // Prevent overflow
            }
            Update.write((uint8_t*)payload, len);
            updateReceived += len;
            Serial.printf("Received %d bytes, total %d/%d\n", len, updateReceived, updateSize);
            if (updateReceived >= updateSize) {
              if (Update.end(true)) {
                Serial.println("Update successful. Sending status and rebooting...");
                // Send success status to client
                DynamicJsonDocument statusDoc(128);
                statusDoc["updateStatus"] = "success";
                String statusJson;
                serializeJson(statusDoc, statusJson);
                webSocket.broadcastTXT(statusJson);
                delay(2000); // Give time for the message to be sent
                ESP.restart();
              } else {
                Serial.println("Update failed.");
                // Send error status to client
                DynamicJsonDocument statusDoc(128);
                statusDoc["updateStatus"] = "error";
                statusDoc["message"] = "Update failed";
                String statusJson;
                serializeJson(statusDoc, statusJson);
                webSocket.broadcastTXT(statusJson);
                isUpdating = false;
                Update.abort();
              }
            }
          }
          break;
        }
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
          break;
      }
    });
    Serial.println("WebSocket server started on port 81");

    // Display IP on Display
    gfx.fillScreen(TFT_BLACK);
    gfx.setCursor(0,0);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK); // Add background color
    gfx.setTextSize(1);
    gfx.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    gfx.println("Ready!");
    gfx.display(); // Explicitly flush buffer
    delay(1000); // Add delay to ensure display updates

    // Initialize session start time
    currentSessionStartTime = time(nullptr);

    // Force initial draw of measurement screen after setup is complete
    // This ensures the screen updates immediately after the "IP and Ready" message.
    currentScreenState = SCREEN_MEASUREMENT; // Ensure state is set
    gfx.fillScreen(TFT_BLACK); // Clear screen completely
    drawMeasurementScreen(currentPowderWeight, alarmActive, alarmSettings.lowThreshold, alarmSettings.highThreshold);
    lastDisplayUpdateTime = millis(); // Reset timer after initial draw
    gfx.display(); // Explicitly push to display after initial draw
    Serial.println("Initial measurement screen drawn and displayed.");
  }
} // <-- Closing brace for setup()


void loop() {
  server.handleClient(); // Handle incoming web requests
  dnsServer.processNextRequest(); // For Captive Portal
  webSocket.loop(); // Handle WebSocket events

  // Handle touch input
  handleTouch();

  systemUptimeMillis = millis(); // Update uptime

  float currentAdc = readDepthADC(); // Get ADC value
  currentPowderWeight = calculatePowderWeight(currentAdc); // Calculate weight from ADC

  // New Auto-measurement logic: triggers when weight is stable within a tolerance of the target
  // Only allows next measurement after weight has dropped below threshold
  // Skip auto-measure during calibration
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED && currentConfigIndex != -1 && currentCalibrationState == CALIBRATE_NONE) {
    float target = powderConfigs[currentConfigIndex].targetGrain;
    float lowerBound = target - AUTO_MEASURE_TOLERANCE_GRAINS;
    float upperBound = target + AUTO_MEASURE_TOLERANCE_GRAINS;

    // Check if we need to reset the measurement flag (weight has dropped to near zero)
    if (lastMeasurementTaken && currentPowderWeight < RESET_MEASUREMENT_THRESHOLD) {
      lastMeasurementTaken = false;
      Serial.println("Measurement flag reset: weight dropped below threshold.");
    }

    // Only proceed with auto-measure if last measurement has been reset
    if (!lastMeasurementTaken) {
      // Check if current weight is within the auto-measure range
      if (currentPowderWeight >= lowerBound && currentPowderWeight <= upperBound) {
          // We are inside the target range.
          if (autoMeasureTimerStart == 0) {
              // We just entered the range, start the timer.
              autoMeasureTimerStart = millis();
              Serial.printf("Entered auto-measure range [%.2f, %.2f]. Starting timer.\n", lowerBound, upperBound);
          } else {
              // Timer is already running, check if it has been stable for long enough.
              unsigned long elapsedTime = millis() - autoMeasureTimerStart;
              
              // Check for timeout: if weight has been stable for too long, reset timer to prevent stale measurements
              if (elapsedTime >= AUTO_MEASURE_MAX_STABLE_TIME_MS) {
                  Serial.printf("Auto-measure timeout: weight stable for %lu ms. Resetting timer.\n", elapsedTime);
                  autoMeasureTimerStart = 0; // Reset timer to wait for weight to drop and re-enter range
              } else if (elapsedTime >= AUTO_MEASURE_STABLE_DURATION_MS) {
                  // Stable for required duration. Check cooldown before measuring.
                  if (millis() - lastAutoMeasureTime > AUTO_MEASURE_COOLDOWN_MS) {
                      Serial.println("Auto-measure triggered! Calling handleAutoMeasure().");
                      handleAutoMeasure(); // Trigger a measurement
                      lastAutoMeasureTime = millis(); // Reset cooldown timer
                      autoMeasureTimerStart = 0; // Reset timer to prevent immediate re-triggering
                      lastMeasurementTaken = true; // Set flag to prevent continuous counting
                  }
              }
          }
      } else {
          // We are outside the target range, reset the timer.
          if (autoMeasureTimerStart != 0) {
              autoMeasureTimerStart = 0;
          }
      }
    }
  } else {
    // No config selected, not connected, or calibration active, ensure timer is reset.
    if (autoMeasureTimerStart != 0) {
        autoMeasureTimerStart = 0;
    }
  }


  // Determine the desired screen state
  ScreenState newScreenState;
  if (WiFi.getMode() == WIFI_AP) {
    newScreenState = SCREEN_AP_MODE;
  } else if (currentCalibrationState != CALIBRATE_NONE) {
    newScreenState = SCREEN_CALIBRATION;
  } else {
    newScreenState = SCREEN_MEASUREMENT;
  }

  // If screen state changes, force a full redraw and update currentScreenState
  if (newScreenState != currentScreenState) {
    Serial.printf("ScreenState changed from %d to %d. Clearing screen.\n", currentScreenState, newScreenState); // Debug print
    gfx.fillScreen(TFT_BLACK); // Clear screen completely for new layout
    currentScreenState = newScreenState;
    lastDisplayUpdateTime = 0; // Force immediate update of new screen
  }

  // Handle TFT display updates with DOUBLE BUFFERING (no more flicker!)
  if (millis() - lastDisplayUpdateTime >= DISPLAY_UPDATE_INTERVAL_MS) {
    // Draw to off-screen sprite buffer (no flicker!)
    canvas.fillScreen(TFT_BLACK);

    if (currentScreenState == SCREEN_MEASUREMENT) {
      drawMeasurementScreen(currentPowderWeight, alarmActive, alarmSettings.lowThreshold, alarmSettings.highThreshold);
    } else if (currentScreenState == SCREEN_CALIBRATION) {
      drawCalibrationScreen(currentCalibrationState, currentAdc, currentPowderWeight);
    } else if (currentScreenState == SCREEN_AP_MODE) {
      drawAPModeScreen();
    }

    // Push complete frame to display in one smooth operation (eliminates flicker!)
    canvas.pushSprite(0, 0);

    lastDisplayUpdateTime = millis();
  }
  

  // Update RGB LED based on weight (this can be more frequent as it's not a full screen redraw)
  #ifdef HAS_RGB_LED
    updateLEDs(currentPowderWeight); // ✅ Only call if board has RGB LED
  #endif

  // Periodically send state to WebSocket clients for live updates
  // Re-added this block to ensure web UI updates
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED && millis() - lastWebSocketUpdateTime >= WEBSOCKET_UPDATE_INTERVAL_MS) {
    sendCurrentStateToClients();
    lastWebSocketUpdateTime = millis();
  }

  delay(5); // Reduced delay for faster loop iteration and more responsive ADC readings
}

/**
 * @brief Initializes the display and touch controller.
 */
void setupDisplay() {
  Serial.println("setupDisplay() called."); // Debug print
  // Init Display
  if (!gfx.begin()) {
    Serial.println("gfx.begin() failed!");
    // If display initialization fails, turn LED red to indicate error
    // leds[0] = CRGB::Red; // RGB LED not present
    // FastLED.show();
    while(true) { // Halt execution if display fails
      Serial.println("Display initialization failed. Halting.");
      delay(1000);
    }
    return; 
  }
  Serial.println("gfx.begin() succeeded."); // Debug print
  gfx.setRotation(DISPLAY_ROTATION); // ✅ Board-specific rotation from board_config.h
  Serial.printf("gfx.setRotation(%d) called\n", DISPLAY_ROTATION); // Debug print
  gfx.fillScreen(TFT_BLACK);
  Serial.println("gfx.fillScreen(TFT_BLACK) called."); // Debug print

  // Backlight is handled by the LGFX config's Light_PWM instance.
  // We can set brightness here if needed.
  gfx.setBrightness(255); // Set to full brightness
  Serial.println("gfx.setBrightness(255) called."); // Debug print

  // Initialize I2C for ADS1115 ADC
  Serial.println("Initializing I2C for ADS1115...");
  Wire.begin(ADS1115_SDA_PIN, ADS1115_SCL_PIN);
  
  // Initialize ADS1115
  if (!ads.begin(ADS1115_ADDRESS, &Wire)) {
    Serial.println("Failed to initialize ADS1115! Check I2C connections.");
    Serial.println("Falling back to direct ADC on GPIO5...");
    adsInitialized = false;
    // Initialize direct ADC pin as fallback
    pinMode(LEVEL_SENSOR_PIN, INPUT);
    Serial.printf("Direct ADC initialized on GPIO%d\n", LEVEL_SENSOR_PIN);
  } else {
    Serial.println("ADS1115 initialized successfully.");
    // Set gain to ±4.096V for better precision with 3.3V signals
    ads.setGain(GAIN_TWOTHIRDS);  // ±6.144V range
    adsInitialized = true;
  }

  gfx.setTextWrap(true);
  gfx.setTextSize(1);
  gfx.setTextColor(TFT_WHITE);
  
  // Load and display startup image (only if SPIFFS is mounted)
  if (SPIFFS.exists("/powdersense_2.png")) {
    File file = SPIFFS.open("/powdersense_2.png", "r");
    if (file) {
      // Read file into buffer
      size_t fileSize = file.size();
      Serial.printf("Image file size: %d bytes\n", fileSize);
      uint8_t *buffer = (uint8_t *)malloc(fileSize);
      if (buffer) {
        size_t bytesRead = file.read(buffer, fileSize);
        file.close();
        Serial.printf("Bytes read: %d\n", bytesRead);
        if (bytesRead == fileSize) {
          // Draw PNG image centered on screen
          int16_t x = (gfx.width() - 172) / 2; // Assuming image width 172, center it
          int16_t y = (gfx.height() - 320) / 2; // Assuming image height 320, center it
          Serial.printf("Drawing PNG at x=%d, y=%d\n", x, y);
          if (gfx.drawPng(buffer, fileSize, x, y)) {
            Serial.println("PNG drawn successfully");
            gfx.display(); // Ensure the image is displayed
            delay(3000); // Show for 3 seconds
          } else {
            Serial.println("Failed to draw PNG");
          }
        } else {
          Serial.println("Failed to read entire image file");
        }
        free(buffer);
      } else {
        Serial.println("Failed to allocate buffer for image");
        file.close();
      }
    } else {
      Serial.println("Failed to open image file");
    }
  } else {
    Serial.println("Image file not found - skipping startup image");
  }
  
  // Display initialization message
  gfx.setTextColor(TFT_WHITE);
  gfx.setTextSize(2);
  gfx.setCursor(10, 10);
  gfx.print("Display OK");
  gfx.display();
  
  delay(500);
  
  Serial.println("setupDisplay() finished."); // Debug print
}

/**
 * @brief Reads the analog value from the ADS1115 ADC or direct GPIO5 ADC and returns the averaged ADC value.
 * @return The averaged ADC value (0-32767 for 16-bit ADS1115 or 0-4095 for 12-bit direct ADC).
 */
float readDepthADC() { // Changed function name and return type
  float currentAdcValue = 0.0;
  
  if (adsInitialized) {
    // Read from ADS1115 channel A0
    int16_t rawAdc = ads.readADC_SingleEnded(ADS1115_CHANNEL);
    currentAdcValue = (float)rawAdc;
  } else {
    // Fallback to direct ADC on GPIO5 if ADS1115 is not available
    int rawAdc = analogRead(LEVEL_SENSOR_PIN);
    currentAdcValue = (float)rawAdc;
  }

  // Store the reading in the circular buffer
  adcReadings[adcReadingsIndex] = currentAdcValue;
  adcReadingsIndex = (adcReadingsIndex + 1) % ADC_READINGS_COUNT;

  // If the buffer hasn't been filled yet, only average the readings taken so far
  if (!adcReadingsFilled && adcReadingsIndex == 0) {
    adcReadingsFilled = true;
  }

  // Calculate the average of the readings in the buffer
  float sumAdc = 0.0;
  int count = adcReadingsFilled ? ADC_READINGS_COUNT : adcReadingsIndex;
  for (int i = 0; i < count; i++) {
    sumAdc += adcReadings[i];
  }
  float averagedAdcValue = sumAdc / count;

  return averagedAdcValue; // Return averaged ADC value
}

/**
 * @brief Converts ADC value to powder weight in grains.
 *        This function uses the calibrated GRAINS_PER_MM_FACTOR (now grains/ADC_unit_diff).
 * @param adcValue The averaged ADC value.
 * @return Estimated powder weight in grains.
 */
float calculatePowderWeight(float adcValue) { // Changed to accept ADC value
  // Use calibration data from the currently selected configuration
  if (currentConfigIndex != -1 && powderConfigs[currentConfigIndex].isCalibrated) {
    const PowderConfig& config = powderConfigs[currentConfigIndex];
    float weight = (adcValue - config.potMinAdc) * config.grainsPerMmFactor;

    // Basic bounds checking for weight
    if (weight < 0.0) weight = 0.0;
    return weight;
  }
  
  return 0.0; // Return 0 if no config is selected or it's not calibrated
}

/**
 * @brief Calculates the standard deviation of the current session measurements.
 * @return The standard deviation in grains.
 */
float calculateStandardDeviation() {
  if (sessionMeasurementCount <= 1) {
    return 0.0;
  }

  float average = sumWeight / sessionMeasurementCount;
  float sumOfSquares = 0.0;

  for (int i = 0; i < sessionMeasurementCount; i++) {
    float difference = measurementHistory[i].weight - average;
    sumOfSquares += difference * difference;
  }

  // Use N-1 for sample standard deviation
  return sqrt(sumOfSquares / (sessionMeasurementCount - 1));
}

/**
 * @brief Updates RGB LED based on current powder weight and alarm thresholds.
 * @param currentWeight The current measured powder weight in grains.
 */
void updateLEDs(float currentWeight) {
  #ifdef HAS_RGB_LED
    bool currentlyGreen = false;
    if (alarmSettings.enabled) {
      if (currentWeight < alarmSettings.lowThreshold || currentWeight > alarmSettings.highThreshold) {
        leds[0] = CRGB::Red;
        alarmActive = true;
      } else {
        leds[0] = CRGB::Green;
        alarmActive = false;
        currentlyGreen = true;
      }
    } else {
      leds[0] = CRGB::Blue;
      alarmActive = false;
    }
    FastLED.show();

    // Update stable measurement state
    if (currentlyGreen && !wasGreenLED) {
      lastGreenLEDTime = millis();
      isStable = false;
    } else if (currentlyGreen && (millis() - lastGreenLEDTime >= STABLE_MEASUREMENT_DURATION_MS)) {
      isStable = true;
    } else if (!currentlyGreen) {
      isStable = false;
    }
    wasGreenLED = currentlyGreen;
  #endif
}

/**
 * @brief Connects to Wi-Fi.
 */
void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSsid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) { // Try for 30 seconds
    delay(500);
    gfx.fillScreen(TFT_BLACK); // Clear display before updating
    gfx.setTextSize(2); // Increased font size
    String wifiMsg = "... Connecting to WiFi ...";
    uint16_t wifiMsgWidth = gfx.textWidth(wifiMsg);
    int wifiX = (gfx.width() - wifiMsgWidth) / 2; // Center horizontally
    gfx.setCursor(wifiX, 10);
    gfx.println(wifiMsg);
    gfx.setTextSize(1);
    uint16_t ssidWidth = gfx.textWidth(wifiSsid);
    int ssidX = (gfx.width() - ssidWidth) / 2; // Center horizontally
    gfx.setCursor(ssidX, 40);
    gfx.println(wifiSsid);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Sync time with NTP
    Serial.println("Syncing time with NTP...");
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
    delay(2000); // Wait for NTP sync
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.println("Time synced successfully");
      char timeStr[30];
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
      Serial.printf("Current time: %s\n", timeStr);
    } else {
      Serial.println("Failed to sync time with NTP");
    }

    gfx.fillScreen(TFT_BLACK); // Clear display before showing IP
    gfx.setTextSize(4); // Extra large font size for IP display
    String ipMsg = WiFi.localIP().toString();
    uint16_t ipWidth = gfx.textWidth(ipMsg);
    int ipX = (gfx.width() - ipWidth) / 2; // Center horizontally
    gfx.setCursor(ipX, 10);
    gfx.println(ipMsg);
    
    gfx.setTextSize(3); // Large font size for WiFi SSID
    String ssidMsg = WiFi.SSID();
    uint16_t ssidWidth = gfx.textWidth(ssidMsg);
    int ssidX = (gfx.width() - ssidWidth) / 2; // Center horizontally
    gfx.setCursor(ssidX, 70);
    gfx.println(ssidMsg);
    
    gfx.setTextSize(2); // Medium font size for Ready message
    String readyMsg = "Ready!";
    uint16_t readyWidth = gfx.textWidth(readyMsg);
    int readyX = (gfx.width() - readyWidth) / 2; // Center horizontally
    gfx.setCursor(readyX, 130);
    gfx.println(readyMsg);
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi. Starting AP mode.");
    gfx.fillScreen(TFT_BLACK); // Clear display before updating
    gfx.setCursor(0,0);
    gfx.setTextSize(2); // Increased font size
    gfx.println("WiFi Failed!");
    gfx.println("Starting AP Mode...");
    startAPMode(); // Fallback to AP mode if STA connection fails
  }
}

/**
 * @brief Starts Access Point (AP) mode for Wi-Fi configuration.
 */
void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PowderSense", ""); // SSID, no password for easy setup
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // DNS server for captive portal
  dnsServer.start(53, "*", apIP);

  // Web server routes for AP mode
  server.on("/", HTTP_GET, handleWiFiConfigPage);
  server.on("/wifi_config.html", HTTP_GET, handleWiFiConfigPage); // Explicitly serve the config page
  server.on("/save_wifi", HTTP_POST, handleWiFiConfigSave);
  server.onNotFound([]() { // Redirect all other requests to config page
    server.sendHeader("Location", "/wifi_config.html");
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("AP Mode HTTP server started.");
}

/**
 * @brief Handles requests to the root URL ("/"). Serves index.html from SPIFFS.
 */
void handleRoot() {
  Serial.println("Request for / (root)");
  File file = SPIFFS.open("/index.html", "r");
  if(!file){
    Serial.println("Failed to open /index.html from SPIFFS");
    server.send(404, "text/plain", "File Not Found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
  Serial.println("Served /index.html");
}

/**
 * @brief Handles requests to "/depth". Returns current state as JSON.
 */
void handleGetDepth() {
  Serial.println("HTTP Request for /depth (full state)");
  DynamicJsonDocument doc(3072); // Increased size for configs

  doc["currentWeight"] = currentPowderWeight;
  doc["currentAdc"] = readDepthADC(); // Add current ADC value
  doc["alarmActive"] = alarmActive;
  doc["alarmEnabled"] = alarmSettings.enabled;
  doc["lowThreshold"] = alarmSettings.lowThreshold;
  doc["highThreshold"] = alarmSettings.highThreshold;
  doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["rssi"] = WiFi.RSSI();
  doc["ipAddress"] = WiFi.localIP().toString();
  doc["uptime"] = systemUptimeMillis;
  time_t now = time(nullptr);
  doc["currentTime"] = now;
  doc["calibrationState"] = currentCalibrationState;
  doc["isCalibrated"] = (currentConfigIndex != -1) ? powderConfigs[currentConfigIndex].isCalibrated : false;
  doc["tempKnownGrainsDepth"] = tempKnownGrainsDepth;
  doc["isStable"] = isStable;
  
  doc["currentConfigIndex"] = currentConfigIndex;

  // Add current config details if one is selected
  if (currentConfigIndex != -1 && currentConfigIndex < configCount) {
    JsonObject currentConfig = doc.createNestedObject("currentConfig");
    currentConfig["name"] = powderConfigs[currentConfigIndex].name;
    currentConfig["caliber"] = powderConfigs[currentConfigIndex].caliber;
    currentConfig["bulletWeight"] = powderConfigs[currentConfigIndex].bulletWeight;
    currentConfig["powderName"] = powderConfigs[currentConfigIndex].powderName;
    currentConfig["targetGrain"] = powderConfigs[currentConfigIndex].targetGrain;
    currentConfig["potMinAdc"] = powderConfigs[currentConfigIndex].potMinAdc; // Added
    currentConfig["grainsPerMmFactor"] = powderConfigs[currentConfigIndex].grainsPerMmFactor; // Added
    currentConfig["isCalibrated"] = powderConfigs[currentConfigIndex].isCalibrated; // Added
  }

  // Add the list of all configurations
  JsonArray configs = doc.createNestedArray("powderConfigs");
  for (int i = 0; i < configCount; i++) {
    JsonObject config_out = configs.createNestedObject();
    config_out["name"] = powderConfigs[i].name;
    config_out["caliber"] = powderConfigs[i].caliber;
    config_out["bulletWeight"] = powderConfigs[i].bulletWeight;
    config_out["powderName"] = powderConfigs[i].powderName;
    config_out["targetGrain"] = powderConfigs[i].targetGrain;
    config_out["potMinAdc"] = powderConfigs[i].potMinAdc;
    config_out["grainsPerMmFactor"] = powderConfigs[i].grainsPerMmFactor;
    config_out["isCalibrated"] = powderConfigs[i].isCalibrated;
  }

  JsonObject stats = doc["stats"].to<JsonObject>();
  stats["averageWeight"] = (sessionMeasurementCount > 0) ? (sumWeight / sessionMeasurementCount) : 0.0; // Handle division by zero
  stats["standardDeviation"] = calculateStandardDeviation();
  stats["minWeight"] = minWeight;
  stats["maxWeight"] = maxWeight;
  stats["totalMeasurements"] = measurementCount;
  stats["sessionMeasurements"] = sessionMeasurementCount;

  JsonArray recentMeasurements = doc["recentMeasurements"].to<JsonArray>();
  // Send all measurements in the current session
  // The JS will reverse it to show newest first.
  for (int i = 0; i < sessionMeasurementCount; i++) {
    JsonObject m = recentMeasurements.createNestedObject();
    m["timestamp"] = measurementHistory[i].timestamp;
    m["weight"] = measurementHistory[i].weight;
    if (measurementHistory[i].configWasSet) {
      JsonObject config = m.createNestedObject("config");
      config["name"] = measurementHistory[i].config.name;
      config["caliber"] = measurementHistory[i].config.caliber;
      config["bulletWeight"] = measurementHistory[i].config.bulletWeight;
      config["powderName"] = measurementHistory[i].config.powderName;
      config["targetGrain"] = measurementHistory[i].config.targetGrain;
    }
  }

  // Add session logs
  JsonArray sessionLogsArray = doc.createNestedArray("sessionLogs");
  Serial.printf("Sending %d session logs to client\n", sessionLogCount);
  for (int i = 0; i < sessionLogCount; i++) {
    JsonObject log = sessionLogsArray.createNestedObject();
    log["startTime"] = sessionLogs[i].startTime;
    log["endTime"] = sessionLogs[i].endTime;
    log["bulletCount"] = sessionLogs[i].bulletCount;
    log["totalWeight"] = sessionLogs[i].totalWeight;
    Serial.printf("  Log %d: bullets=%d, weight=%.2f\n", i, sessionLogs[i].bulletCount, sessionLogs[i].totalWeight);
  }

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
  Serial.println("Served /depth JSON state.");
}

/**
 * @brief Handles requests for /api/measurement (fallback for JS).
 */
void handleApiMeasurement() {
  Serial.println("HTTP Request for /api/measurement (fallback)");
  DynamicJsonDocument doc(128);
  doc["powderWeight"] = currentPowderWeight; // Use currentPowderWeight for API
  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
  Serial.println("Served /api/measurement JSON.");
}

/**
 * @brief Handles requests for unknown URLs. Attempts to serve static files from SPIFFS.
 */
void handleNotFound() {
  String path = server.uri();
  Serial.print("Handling not found: Requested path: ");
  Serial.println(path);

  if (path == "/ws") {
    Serial.println("Ignoring /ws request, handled by WebSocketsServer.");
    return;
  }

  // Handle captive portal requests (respond with 204 No Content)
  if (path == "/generate_204" || path == "/generate204" || path == "/connecttest.txt" || path == "/hotspot-detect.html") {
    server.send(204, "text/plain", "");
    return;
  }

  if (WiFi.getMode() == WIFI_AP) {
    server.sendHeader("Location", "/wifi_config.html");
    server.send(302, "text/plain", "");
    return;
  }

  // If the path is exactly "/" or ends with "/", try to serve index.html
  if(path == "/" || path.endsWith("/")){
    path = "/index.html"; // Explicitly set to index.html
  }

  String contentType = getContentType(path);
  if(SPIFFS.exists(path)){
    Serial.print("File found in SPIFFS: ");
    Serial.println(path);
    File file = SPIFFS.open(path, "r");
    if(file){
      server.streamFile(file, contentType);
      file.close();
      Serial.print("Served file: ");
      Serial.println(path);
    } else {
      Serial.print("Failed to open file from SPIFFS: ");
      Serial.println(path);
    }
  } else {
    Serial.print("File not found in SPIFFS: ");
    Serial.println(path);
  }

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
  Serial.println("Sent 404 Not Found.");
}

/**
 * @brief Sends the current state of the device to all connected WebSocket clients.
 */
void sendCurrentStateToClients() {
  DynamicJsonDocument doc(3072); // Increased size for configs

  doc["currentWeight"] = currentPowderWeight;
  doc["currentAdc"] = readDepthADC(); // Add current ADC value
  doc["alarmActive"] = alarmActive;
  doc["alarmEnabled"] = alarmSettings.enabled;
  doc["lowThreshold"] = alarmSettings.lowThreshold;
  doc["highThreshold"] = alarmSettings.highThreshold;
  doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["rssi"] = WiFi.RSSI();
  doc["ipAddress"] = WiFi.localIP().toString();
  doc["uptime"] = systemUptimeMillis;
  doc["calibrationState"] = currentCalibrationState;
  doc["isCalibrated"] = (currentConfigIndex != -1) ? powderConfigs[currentConfigIndex].isCalibrated : false;
  doc["tempKnownGrainsDepth"] = tempKnownGrainsDepth;
  doc["isStable"] = isStable;
  
  doc["currentConfigIndex"] = currentConfigIndex;

  // Add current config details if one is selected
  if (currentConfigIndex != -1 && currentConfigIndex < configCount) {
    JsonObject currentConfig = doc.createNestedObject("currentConfig");
    currentConfig["name"] = powderConfigs[currentConfigIndex].name;
    currentConfig["caliber"] = powderConfigs[currentConfigIndex].caliber;
    currentConfig["bulletWeight"] = powderConfigs[currentConfigIndex].bulletWeight;
    currentConfig["powderName"] = powderConfigs[currentConfigIndex].powderName;
    currentConfig["targetGrain"] = powderConfigs[currentConfigIndex].targetGrain;
    currentConfig["potMinAdc"] = powderConfigs[currentConfigIndex].potMinAdc; // Added
    currentConfig["grainsPerMmFactor"] = powderConfigs[currentConfigIndex].grainsPerMmFactor; // Added
    currentConfig["isCalibrated"] = powderConfigs[currentConfigIndex].isCalibrated; // Added
  }

  // Add the list of all configurations
  JsonArray configs = doc.createNestedArray("powderConfigs");
  for (int i = 0; i < configCount; i++) {
    JsonObject config_out = configs.createNestedObject();
    config_out["name"] = powderConfigs[i].name;
    config_out["caliber"] = powderConfigs[i].caliber;
    config_out["bulletWeight"] = powderConfigs[i].bulletWeight;
    config_out["powderName"] = powderConfigs[i].powderName;
    config_out["targetGrain"] = powderConfigs[i].targetGrain;
    config_out["potMinAdc"] = powderConfigs[i].potMinAdc;
    config_out["grainsPerMmFactor"] = powderConfigs[i].grainsPerMmFactor;
    config_out["isCalibrated"] = powderConfigs[i].isCalibrated;
  }

  JsonObject stats = doc["stats"].to<JsonObject>();
  stats["averageWeight"] = (sessionMeasurementCount > 0) ? (sumWeight / sessionMeasurementCount) : 0.0; // Handle division by zero
  stats["standardDeviation"] = calculateStandardDeviation();
  stats["minWeight"] = minWeight;
  stats["maxWeight"] = maxWeight;
  stats["totalMeasurements"] = measurementCount;
  stats["sessionMeasurements"] = sessionMeasurementCount;

  JsonArray recentMeasurements = doc["recentMeasurements"].to<JsonArray>();
  // Send all measurements in the current session
  // The JS will reverse it to show newest first.
  for (int i = 0; i < sessionMeasurementCount; i++) {
    JsonObject m = recentMeasurements.createNestedObject();
    m["timestamp"] = measurementHistory[i].timestamp;
    m["weight"] = measurementHistory[i].weight;
    if (measurementHistory[i].configWasSet) {
      JsonObject config = m.createNestedObject("config");
      config["name"] = measurementHistory[i].config.name;
      config["caliber"] = measurementHistory[i].config.caliber;
      config["bulletWeight"] = measurementHistory[i].config.bulletWeight;
      config["powderName"] = measurementHistory[i].config.powderName;
      config["targetGrain"] = measurementHistory[i].config.targetGrain;
    }
  }

  // Add session logs
  JsonArray sessionLogsArray = doc.createNestedArray("sessionLogs");
  Serial.printf("Sending %d session logs to client via WebSocket\n", sessionLogCount);
  for (int i = 0; i < sessionLogCount; i++) {
    JsonObject log = sessionLogsArray.createNestedObject();
    log["startTime"] = sessionLogs[i].startTime;
    log["endTime"] = sessionLogs[i].endTime;
    log["bulletCount"] = sessionLogs[i].bulletCount;
    log["totalWeight"] = sessionLogs[i].totalWeight;
    Serial.printf("  Log %d: bullets=%d, weight=%.2f\n", i, sessionLogs[i].bulletCount, sessionLogs[i].totalWeight);
  }

  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.broadcastTXT(jsonString);
}


/**
 * @brief Saves settings to SPIFFS.
 */
void saveSettings() {
  // Increased JSON document size to handle array of configs
  DynamicJsonDocument doc(2048); 

  // Save global settings
  doc["alarmEnabled"] = alarmSettings.enabled;
  doc["lowThreshold"] = alarmSettings.lowThreshold;
  doc["highThreshold"] = alarmSettings.highThreshold;
  doc["currentConfigIndex"] = currentConfigIndex;

  // Save powder configurations
  JsonArray configs = doc.createNestedArray("powderConfigs");
  for (int i = 0; i < configCount; i++) {
    JsonObject config_out = configs.createNestedObject();
    config_out["name"] = powderConfigs[i].name;
    config_out["caliber"] = powderConfigs[i].caliber;
    config_out["bulletWeight"] = powderConfigs[i].bulletWeight;
    config_out["powderName"] = powderConfigs[i].powderName;
    config_out["targetGrain"] = powderConfigs[i].targetGrain;
    config_out["potMinAdc"] = powderConfigs[i].potMinAdc; // Added
    config_out["grainsPerMmFactor"] = powderConfigs[i].grainsPerMmFactor; // Added
    config_out["isCalibrated"] = powderConfigs[i].isCalibrated; // Added
  }

  // Save session logs
  JsonArray logs = doc.createNestedArray("sessionLogs");
  for (int i = 0; i < sessionLogCount; i++) {
    JsonObject log_out = logs.createNestedObject();
    log_out["startTime"] = sessionLogs[i].startTime;
    log_out["endTime"] = sessionLogs[i].endTime;
    log_out["bulletCount"] = sessionLogs[i].bulletCount;
    log_out["totalWeight"] = sessionLogs[i].totalWeight;
  }

  File file = SPIFFS.open("/settings.json", "w");
  if (!file) {
    Serial.println("Failed to open settings file for writing");
    return;
  }
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write settings to file");
  } else {
    Serial.println("Settings data saved.");
  }
  file.close();
}

/**
 * @brief Loads settings from SPIFFS.
 */
void loadSettings() {
  if (!SPIFFS.exists("/settings.json")) {
    Serial.println("Settings file not found, using initial defaults.");
    return;
  }

  File file = SPIFFS.open("/settings.json", "r");
  if (!file) {
    Serial.println("Failed to open settings file, using initial defaults.");
    return;
  }

  // Check if file is empty
  if (file.size() == 0) {
    Serial.println("Settings file is empty, using initial defaults.");
    file.close();
    return;
  }

  // Increased JSON document size to handle array of configs
  DynamicJsonDocument doc(2048); 
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to read settings file, using initial defaults.");
    Serial.println(error.f_str());
    file.close();
    return;
  }

  // Load global settings
  alarmSettings.enabled = doc["alarmEnabled"] | true;
  alarmSettings.lowThreshold = doc["lowThreshold"] | 0.0;
  alarmSettings.highThreshold = doc["highThreshold"] | 100.0;
  currentConfigIndex = doc["currentConfigIndex"] | -1;

  // Load powder configurations
  JsonArray configs = doc["powderConfigs"].as<JsonArray>();
  configCount = 0;
  for (JsonObject config_in : configs) {
    if (configCount >= MAX_CONFIGS) break;

    strlcpy(powderConfigs[configCount].name, config_in["name"].as<const char*>(), sizeof(powderConfigs[configCount].name));
    strlcpy(powderConfigs[configCount].caliber, config_in["caliber"].as<const char*>(), sizeof(powderConfigs[configCount].caliber));
    strlcpy(powderConfigs[configCount].bulletWeight, config_in["bulletWeight"].as<const char*>(), sizeof(powderConfigs[configCount].bulletWeight));
    strlcpy(powderConfigs[configCount].powderName, config_in["powderName"].as<const char*>(), sizeof(powderConfigs[configCount].powderName));
    powderConfigs[configCount].targetGrain = config_in["targetGrain"] | 0.0;
    powderConfigs[configCount].potMinAdc = config_in["potMinAdc"] | 0.0;
    powderConfigs[configCount].grainsPerMmFactor = config_in["grainsPerMmFactor"] | 0.0;
    powderConfigs[configCount].isCalibrated = config_in["isCalibrated"] | false;

    configCount++;
  }

  // Load session logs
  JsonArray logs = doc["sessionLogs"].as<JsonArray>();
  sessionLogCount = 0;
  for (JsonObject log_in : logs) {
    if (sessionLogCount >= MAX_SESSION_LOGS) break;
    sessionLogs[sessionLogCount].startTime = log_in["startTime"] | 0;
    sessionLogs[sessionLogCount].endTime = log_in["endTime"] | 0;
    sessionLogs[sessionLogCount].bulletCount = log_in["bulletCount"] | 0;
    sessionLogs[sessionLogCount].totalWeight = log_in["totalWeight"] | 0.0;
    sessionLogCount++;
  }

  // Ensure currentConfigIndex is valid after loading configs
  if (currentConfigIndex >= configCount || currentConfigIndex < -1) {
    currentConfigIndex = -1;
  }

  Serial.printf("Settings loaded. %d configurations and %d session logs found.\n", configCount, sessionLogCount);
  file.close();
}

/**
 * @brief Saves Wi-Fi credentials to NVS.
 */
void saveWiFiCredentials(const String& ssid, const String& password) {
  preferences.begin("wifi-creds", false); // Open NVS namespace
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end(); // Close NVS namespace
  Serial.println("WiFi credentials saved to NVS.");
}

/**
 * @brief Loads Wi-Fi credentials from NVS.
 */
void loadWiFiCredentials() {
  preferences.begin("wifi-creds", false); // Open NVS namespace
  wifiSsid = preferences.getString("ssid", "");
  wifiPassword = preferences.getString("password", "");
  preferences.end(); // Close NVS namespace

  if (wifiSsid == "") {
    Serial.println("WiFi credentials not found in NVS.");
  } else {
    Serial.printf("WiFi credentials loaded from NVS: SSID=%s\n", wifiSsid.c_str());
  }
}

/**
 * @brief Handles the submission of Wi-Fi configuration from the AP mode page.
 */
void handleWiFiConfigSave() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String newSsid = server.arg("ssid");
    String newPassword = server.arg("password");
    saveWiFiCredentials(newSsid, newPassword);
    
    server.send(200, "text/html", "<h1>WiFi Config Saved!</h1><p>Attempting to connect to your network. Please restart your device or wait for it to reconnect.</p><p>You can now disconnect from 'PowderSense' AP and connect to your home network.</p>");
    Serial.println("WiFi credentials received and saved. Restarting...");
    delay(1000);
    ESP.restart(); // Restart to connect to the new network
  } else {
    server.send(400, "text/plain", "Missing SSID or Password");
  }
}

/**
 * @brief Serves the Wi-Fi configuration HTML page for AP mode.
 */
void handleWiFiConfigPage() {
  File file = SPIFFS.open("/wifi_config.html", "r");
  if(!file){
    Serial.println("Failed to open /wifi_config.html from SPIFFS");
    server.send(404, "text/plain", "WiFi Config Page Not Found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
  Serial.println("Served /wifi_config.html");
}


// --- Command Handler Implementations (Placeholders) ---

void handleZeroCommand() {
  Serial.println("Command: Zero Sensor");
  // Implement zeroing logic here
  // For now, let's simulate a measurement after zeroing
  currentPowderWeight = 0.0; // Set to zero after zeroing
  // Add to history if desired
  sendCurrentStateToClients();
}

void handleMeasureCommand() {
  Serial.println("handleMeasureCommand() called."); // Debug print

  // Prevent measurements during calibration to avoid messing with statistics
  if (currentCalibrationState != CALIBRATE_NONE) {
    Serial.println("Measurement blocked: Calibration in progress.");
    sendCurrentStateToClients(); // Still send state update to UI
    return;
  }

  // Get a fresh, current measurement value at the moment the command is issued
  // Use the currentPowderWeight for recording, as it's already averaged
  float newWeight = currentPowderWeight;

  // Add to history
  int historyIndex;
  if (sessionMeasurementCount < MAX_MEASUREMENTS_HISTORY) {
    historyIndex = sessionMeasurementCount;
    sessionMeasurementCount++;
  } else {
    // Shift history if full
    for (int i = 0; i < MAX_MEASUREMENTS_HISTORY - 1; i++) {
      measurementHistory[i] = measurementHistory[i+1];
    }
    historyIndex = MAX_MEASUREMENTS_HISTORY - 1;
  }

  measurementHistory[historyIndex].timestamp = time(nullptr);
  measurementHistory[historyIndex].weight = newWeight;

  // Record the configuration used for this measurement
  if (currentConfigIndex != -1) {
    measurementHistory[historyIndex].config = powderConfigs[currentConfigIndex];
    measurementHistory[historyIndex].configWasSet = true;
    Serial.printf("Measurement recorded: %.3f grains with config '%s'. Session count: %d\n", newWeight, powderConfigs[currentConfigIndex].name, sessionMeasurementCount);
  } else {
    measurementHistory[historyIndex].configWasSet = false;
    memset(&measurementHistory[historyIndex].config, 0, sizeof(PowderConfig)); // Clear out old data
    Serial.printf("Measurement recorded: %.3f grains without config. Session count: %d\n", newWeight, sessionMeasurementCount);
  }

  // Update min/max/sum only if this is the first measurement or if it's truly min/max
  if (sessionMeasurementCount == 1) { // First measurement in session
    minWeight = newWeight;
    maxWeight = newWeight;
    sumWeight = newWeight;
  } else {
    if (newWeight < minWeight) minWeight = newWeight;
    if (newWeight > maxWeight) maxWeight = newWeight;
    sumWeight += newWeight;
  }

  measurementCount++; // Total count (across all sessions since boot)

  // Auto-save settings after each measurement to persist session data
  saveSettings();

  sendCurrentStateToClients();
}

void handleCalibrateCommand() {
  Serial.println("Command: Calibrate (UI update)");
  // This function is now primarily for sending the current calibration state to the UI
  sendCurrentStateToClients();
}

void handleSelectConfigCommand(int index) {
  Serial.printf("Command: Select Config index %d\n", index);
  if (index >= 0 && index < configCount) {
    currentConfigIndex = index;
    // When a config is selected, auto-set the alarm thresholds based on its target grain
    float target = powderConfigs[index].targetGrain;
    alarmSettings.lowThreshold = target - 0.10; // Default to +/- 0.10 grain tolerance (difference 0.20)
    alarmSettings.highThreshold = target + 0.10;
    Serial.printf("Selected config '%s'. Alarms set to %.2f/%.2f\n", powderConfigs[index].name, alarmSettings.lowThreshold, alarmSettings.highThreshold);
  } else {
    currentConfigIndex = -1; // Deselect
    Serial.println("No configuration selected.");
  }
  saveSettings(); // Save the new current index and thresholds
  sendCurrentStateToClients();
}

void handleSaveConfigCommand(JsonObject data) {
  int index = data["index"];
  
  if (index < 0 || index >= MAX_CONFIGS) {
    Serial.println("Error: Invalid index for saveConfig");
    return;
  }

  // If index is for a new config, make sure there's space
  if (index == configCount && configCount < MAX_CONFIGS) {
    // Initialize calibration data for new config
    powderConfigs[index].potMinAdc = 0.0;
    powderConfigs[index].grainsPerMmFactor = 0.0;
    powderConfigs[index].isCalibrated = false;
    configCount++; // It's a new config, increment count
    Serial.printf("Command: Add new config at index %d\n", index);
  } else if (index < configCount) {
    Serial.printf("Command: Update config at index %d\n", index);
  } else {
    Serial.println("Error: Cannot add new config, max reached or invalid index.");
    return;
  }

  strlcpy(powderConfigs[index].caliber, data["caliber"], sizeof(powderConfigs[index].caliber));
  strlcpy(powderConfigs[index].bulletWeight, data["bulletWeight"], sizeof(powderConfigs[index].bulletWeight));
  strlcpy(powderConfigs[index].powderName, data["powderName"], sizeof(powderConfigs[index].powderName)); 
  powderConfigs[index].targetGrain = data["targetGrain"];

  // Construct the name to include target grain
  char nameBuffer[sizeof(powderConfigs[index].name)];
  snprintf(nameBuffer, sizeof(nameBuffer), "%s %sgr %s %.2fgr", 
           powderConfigs[index].caliber, 
           powderConfigs[index].bulletWeight, 
           powderConfigs[index].powderName,
           powderConfigs[index].targetGrain);
  strlcpy(powderConfigs[index].name, nameBuffer, sizeof(powderConfigs[index].name));


  saveSettings();
  sendCurrentStateToClients();
}

void handleDeleteConfigCommand(int index) {
  Serial.printf("Command: Delete config at index %d\n", index);
  if (index < 0 || index >= configCount) {
    Serial.println("Error: Invalid index for deleteConfig");
    return;
  }

  // Shift all subsequent configs down
  for (int i = index; i < configCount - 1; i++) {
    powderConfigs[i] = powderConfigs[i+1];
  }
  configCount--;

  // If the deleted config was the currently selected one, deselect
  if (currentConfigIndex == index) {
    currentConfigIndex = -1;
  } else if (currentConfigIndex > index) {
    // If we deleted one before the current one, decrement the current index
    currentConfigIndex--;
  }

  saveSettings();
  sendCurrentStateToClients();
}

void handleSetAlarmsCommand(bool enabled, float low, float high) {
  Serial.printf("Command: Set Alarms Enabled: %s, Low: %.1f, High: %.1f\n", enabled ? "true" : "false", low, high);
  alarmSettings.enabled = enabled;
  alarmSettings.lowThreshold = low;
  alarmSettings.highThreshold = high;
  saveSettings(); // Save changes to settings.json
  sendCurrentStateToClients();
}

void handleAcknowledgeAlarmCommand() {
  Serial.println("Command: Acknowledge Alarm");
  alarmActive = false;
  #ifdef HAS_RGB_LED
    leds[0] = CRGB::Green;
    FastLED.show();
  #endif
  sendCurrentStateToClients();
}

void handleResetSessionCommand() {
  Serial.println("Command: Reset Session");

  // Save current session to log before resetting (if active)
  if (sessionMeasurementCount > 0) {
    handleEndSessionCommand(); // End the current session and log it
  }

  // Reset session variables
  sessionMeasurementCount = 0;
  minWeight = 0.0; // Reset to 0.0
  maxWeight = 0.0;    // Reset to 0.0
  sumWeight = 0.0;
  currentSessionStartTime = time(nullptr); // Start new session (effectively a reset)

  // Clear history array (optional, as count handles it)
  for (int i = 0; i < MAX_MEASUREMENTS_HISTORY; i++) {
    measurementHistory[i].timestamp = 0;
    measurementHistory[i].weight = 0.0;
    measurementHistory[i].configWasSet = false;
    memset(&measurementHistory[i].config, 0, sizeof(PowderConfig));
  }
  sendCurrentStateToClients();
}

void handleStartSessionCommand() {
  Serial.println("Command: Start Session");
  // A session is implicitly started when the device boots or after a reset.
  // This command primarily ensures the session is fresh and ready to log.
  // We treat this as a soft reset of the current session stats.
  if (sessionMeasurementCount > 0) {
    handleEndSessionCommand(); // Log the previous session if it exists
  }
  
  // Reset session variables
  sessionMeasurementCount = 0;
  minWeight = 0.0;
  maxWeight = 0.0;
  sumWeight = 0.0;
  currentSessionStartTime = time(nullptr);
  sessionStartMeasurementIndex = measurementCount; // Mark where this session's measurements start
  
  // Clear history array for the new session
  for (int i = 0; i < MAX_MEASUREMENTS_HISTORY; i++) {
    measurementHistory[i].timestamp = 0;
    measurementHistory[i].weight = 0.0;
    measurementHistory[i].configWasSet = false;
    memset(&measurementHistory[i].config, 0, sizeof(PowderConfig));
  }
  
  saveSettings(); // Save the reset state
  sendCurrentStateToClients();
}

void handleEndSessionCommand() {
  Serial.println("Command: End Session");
  
  // Save current session to log
  if (sessionMeasurementCount > 0) {
    SessionLog currentLog;
    currentLog.startTime = currentSessionStartTime;
    currentLog.endTime = time(nullptr);
    currentLog.bulletCount = sessionMeasurementCount;
    currentLog.totalWeight = sumWeight;
    currentLog.measurementStartIndex = sessionStartMeasurementIndex;
    currentLog.measurementCount = sessionMeasurementCount;

    Serial.printf("Creating session log: bullets=%d, weight=%.3f, start=%ld, end=%ld\n", 
                  currentLog.bulletCount, currentLog.totalWeight, currentLog.startTime, currentLog.endTime);

    // Log to SPIFFS (for web UI display)
    if (sessionLogCount < MAX_SESSION_LOGS) {
      sessionLogs[sessionLogCount] = currentLog;
      sessionLogCount++;
      Serial.printf("Added session log. Total logs: %d\n", sessionLogCount);
    } else {
      // Shift logs if full
      for (int i = 0; i < MAX_SESSION_LOGS - 1; i++) {
        sessionLogs[i] = sessionLogs[i+1];
      }
      sessionLogs[MAX_SESSION_LOGS - 1] = currentLog;
      Serial.printf("Session logs full. Shifted and added new log.\n");
    }
    saveSettings(); // Save session logs (SPIFFS)
    Serial.printf("Session saved to SPIFFS. Total session logs: %d\n", sessionLogCount);
    
    // After logging, reset the current session stats to zero, but keep the history buffer intact
    // The next measurement will start a new implicit session.
    sessionMeasurementCount = 0;
    minWeight = 0.0;
    maxWeight = 0.0;
    sumWeight = 0.0;
    
    // Clear history array after session ends
    for (int i = 0; i < MAX_MEASUREMENTS_HISTORY; i++) {
      measurementHistory[i].timestamp = 0;
      measurementHistory[i].weight = 0.0;
      measurementHistory[i].configWasSet = false;
      memset(&measurementHistory[i].config, 0, sizeof(PowderConfig));
    }
    
    currentSessionStartTime = time(nullptr);
    sessionStartMeasurementIndex = measurementCount; // Next session starts after current measurements
  } else {
    Serial.println("No measurements in current session to log.");
  }
  
  sendCurrentStateToClients();
}

void handleExportDataCommand() {
  Serial.println("HTTP Request: Export Data (CSV)");
  // This will be a simple CSV export of the current session history
  String csv = "Timestamp,Weight(grains),Config Name,Caliber,Bullet Weight,Powder Name,Target Grain\n";
  for (int i = 0; i < sessionMeasurementCount; i++) {
    csv += String(measurementHistory[i].timestamp) + "," + String(measurementHistory[i].weight, 3);
    if (measurementHistory[i].configWasSet) {
      csv += "," + String(measurementHistory[i].config.name);
      csv += "," + String(measurementHistory[i].config.caliber);
      csv += "," + String(measurementHistory[i].config.bulletWeight);
      csv += "," + String(measurementHistory[i].config.powderName);
      csv += "," + String(measurementHistory[i].config.targetGrain, 3);
    } else {
      csv += ",,,,,,"; // Add empty columns if no config was set
    }
    csv += "\n";
  }
  server.send(200, "text/csv", csv);
  Serial.println("Served CSV data.");
}

void handleExportSessionCommand() {
  Serial.println("HTTP Request: Export Session Details (CSV)");
  
  if (!server.hasArg("index")) {
    server.send(400, "text/plain", "Missing session index parameter");
    return;
  }
  
  int sessionIndex = server.arg("index").toInt();
  
  if (sessionIndex < 0 || sessionIndex >= sessionLogCount) {
    server.send(404, "text/plain", "Session not found");
    return;
  }

  SessionLog& log = sessionLogs[sessionIndex];
  
  // Create a detailed CSV export for this session with all measurements
  String csv = "Timestamp,Weight(grains),Config Name,Caliber,Bullet Weight,Powder Name,Target Grain\n";
  
  // Export all measurements from this session
  // Note: We're exporting from the stored session indices
  for (int i = 0; i < log.measurementCount; i++) {
    // Calculate the actual index in the global measurement history
    // Since measurements wrap around, we need to be careful here
    // For now, we'll iterate through recent measurements that match the session timeframe
    for (int j = 0; j < sessionMeasurementCount; j++) {
      if (measurementHistory[j].timestamp >= log.startTime && 
          measurementHistory[j].timestamp <= log.endTime) {
        csv += String(measurementHistory[j].timestamp) + "," + String(measurementHistory[j].weight, 3);
        if (measurementHistory[j].configWasSet) {
          csv += "," + String(measurementHistory[j].config.name);
          csv += "," + String(measurementHistory[j].config.caliber);
          csv += "," + String(measurementHistory[j].config.bulletWeight);
          csv += "," + String(measurementHistory[j].config.powderName);
          csv += "," + String(measurementHistory[j].config.targetGrain, 3);
        } else {
          csv += ",,,,,";
        }
        csv += "\n";
      }
    }
  }
  
  server.send(200, "text/csv", csv);
  Serial.println("Served session export CSV data.");
}


void handleAutoMeasure() {
  Serial.println("handleAutoMeasure() called."); // Debug print
  handleMeasureCommand(); // Call the existing measure command handler
}

void handleFactoryResetCommand() {
  Serial.println("Command: Factory Reset initiated!");
  // Clear all saved data
  SPIFFS.format(); // This will erase all files on SPIFFS
  Serial.println("SPIFFS formatted. Restarting device.");
  ESP.restart(); // Restart the device to apply changes
}

void handleUpdateFirmwareCommand(String type, String filename, size_t size) {
  Serial.printf("Command: Update %s with file %s, size %d\n", type.c_str(), filename.c_str(), size);
  if (isUpdating) {
    Serial.println("Update already in progress.");
    return;
  }
  updateType = type;
  updateSize = size;
  updateReceived = 0;
  isUpdating = true;

  bool updateStarted = false;
  if (type == "firmware") {
    updateStarted = Update.begin(size, U_FLASH);
  } else if (type == "filesystem") {
    updateStarted = Update.begin(size, U_SPIFFS);
  } else {
    Serial.println("Unknown update type.");
    isUpdating = false;
    return;
  }

  if (!updateStarted) {
    Serial.println("Update.begin() failed.");
    Update.printError(Serial);
    isUpdating = false;
    return;
  }

  Serial.printf("Update started for %s. Waiting for binary data...\n", type.c_str());
  // Send status to client
  DynamicJsonDocument statusDoc(128);
  statusDoc["updateStatus"] = "started";
  String statusJson;
  serializeJson(statusDoc, statusJson);
  webSocket.broadcastTXT(statusJson);
}

void handleImportConfigsCommand(JsonArray configs) {
  Serial.printf("Command: Import %d configurations\n", configs.size());
  configCount = 0; // Replace existing configs with imported ones
  for (JsonObject config_in : configs) {
    if (configCount >= MAX_CONFIGS) break;

    strlcpy(powderConfigs[configCount].name, config_in["name"].as<const char*>(), sizeof(powderConfigs[configCount].name));
    strlcpy(powderConfigs[configCount].caliber, config_in["caliber"].as<const char*>(), sizeof(powderConfigs[configCount].caliber));
    strlcpy(powderConfigs[configCount].bulletWeight, config_in["bulletWeight"].as<const char*>(), sizeof(powderConfigs[configCount].bulletWeight));
    strlcpy(powderConfigs[configCount].powderName, config_in["powderName"].as<const char*>(), sizeof(powderConfigs[configCount].powderName));
    powderConfigs[configCount].targetGrain = config_in["targetGrain"] | 0.0;
    powderConfigs[configCount].potMinAdc = config_in["potMinAdc"] | 0.0;
    powderConfigs[configCount].grainsPerMmFactor = config_in["grainsPerMmFactor"] | 0.0;
    powderConfigs[configCount].isCalibrated = config_in["isCalibrated"] | false;

    configCount++;
  }
  saveSettings();
  Serial.printf("Imported %d configurations successfully.\n", configCount);
  sendCurrentStateToClients();
}

void handleExportSessionDetailsCommand(int sessionIndex) {
  Serial.printf("Command: Export session details for index %d\n", sessionIndex);
  
  if (sessionIndex < 0 || sessionIndex >= sessionLogCount) {
    Serial.println("Error: Invalid session index");
    return;
  }

  SessionLog& log = sessionLogs[sessionIndex];
  
  // Create a detailed CSV export for this session
  String csv = "PowderSense Session Export\n";
  csv += "Session Date: " + String(ctime(&log.startTime));
  csv += "Total Bullets: " + String(log.bulletCount) + "\n";
  csv += "Total Weight: " + String(log.totalWeight, 3) + " grains\n";
  csv += "Average Weight: " + String(log.totalWeight / log.bulletCount, 3) + " grains\n\n";
  
  csv += "Note: Per-bullet measurements are stored in the Recent Measurements table during the session.\n";
  csv += "To export detailed per-bullet data, use the 'Export Data' button during or immediately after a session.\n\n";
  
  csv += "Session Summary\n";
  csv += "Metric,Value\n";
  csv += "Total Bullets," + String(log.bulletCount) + "\n";
  csv += "Total Weight (gr)," + String(log.totalWeight, 3) + "\n";
  csv += "Average Weight (gr)," + String(log.totalWeight / log.bulletCount, 3) + "\n";
  csv += "Start Time," + String(log.startTime) + "\n";
  csv += "End Time," + String(log.endTime) + "\n";
  csv += "Duration (seconds)," + String(log.endTime - log.startTime) + "\n";
  
  // Send as HTTP response (this would be called via HTTP endpoint)
  // For now, just log it
  Serial.println("Session export CSV generated (length: " + String(csv.length()) + " bytes)");
}

// --- Calibration Wizard Functions ---
// Moved these function definitions above webSocketEvent
// so they are visible when webSocketEvent is compiled.

void startCalibrationWizard() {
  if (currentConfigIndex == -1) {
    Serial.println("Error: Cannot start calibration. No configuration selected.");
    // The UI should prevent this, but as a safeguard:
    return;
  }
  currentCalibrationState = CALIBRATE_ZERO_STEP;
  tempKnownGrainsDepth = 0.0; // Reset this to 0.0 when starting calibration
  Serial.printf("Calibration Wizard started for config: '%s'\n", powderConfigs[currentConfigIndex].name);
  alarmActive = false; // Ensure alarm is not active during calibration
  sendCurrentStateToClients();
}

void setCalibrationZeroPoint() {
  // Capture the current ADC value as the zero point.
  // We use the averaged ADC value for better stability.
  float sumAdc = 0.0;
  int count = adcReadingsFilled ? ADC_READINGS_COUNT : adcReadingsIndex;
  for (int i = 0; i < count; i++) {
    sumAdc += adcReadings[i];
  }
  float averagedAdc = sumAdc / count;
  powderConfigs[currentConfigIndex].potMinAdc = averagedAdc;

  currentCalibrationState = CALIBRATE_KNOWN_GRAINS_STEP; // Move to next step
  Serial.printf("Zero point set to ADC: %.0f for config '%s'.\n", averagedAdc, powderConfigs[currentConfigIndex].name);
  alarmActive = false; // Ensure alarm is not active during calibration
  sendCurrentStateToClients();
}

void setCalibrationKnownGrains(float knownWeight) {
  if (currentCalibrationState == CALIBRATE_KNOWN_GRAINS_STEP) {
    float currentAdcAtKnownGrains = 0.0;
    int count = adcReadingsFilled ? ADC_READINGS_COUNT : adcReadingsIndex;
    for (int i = 0; i < count; i++) {
      currentAdcAtKnownGrains += adcReadings[i];
    }
    currentAdcAtKnownGrains /= count;

    PowderConfig& config = powderConfigs[currentConfigIndex];
    float adcDifference = currentAdcAtKnownGrains - config.potMinAdc;

    if (adcDifference > 0.01 && knownWeight > 0.0) { // Avoid division by zero or near-zero values
      config.grainsPerMmFactor = knownWeight / adcDifference;
      config.isCalibrated = true; // Mark this configuration as calibrated
      Serial.printf("Calculated Grains/ADC_Diff Factor for '%s': %.4f (Known Weight: %.2f gr, ADC Diff: %.2f)\n", config.name, config.grainsPerMmFactor, knownWeight, adcDifference);
    
      saveSettings(); // Save all settings, including the new calibration data

      // Auto-set alarm thresholds based on target grain
      alarmSettings.lowThreshold = powderConfigs[currentConfigIndex].targetGrain - 0.10; // 0.10 grains lower (difference 0.20)
      alarmSettings.highThreshold = powderConfigs[currentConfigIndex].targetGrain + 0.10; // 0.10 grains higher
      saveSettings(); // Save the updated alarm settings

      // Reset session statistics to prevent calibration sample from being counted
      sessionMeasurementCount = 0;
      minWeight = 0.0;
      maxWeight = 0.0;
      sumWeight = 0.0;
      // Clear measurement history
      for (int i = 0; i < MAX_MEASUREMENTS_HISTORY; i++) {
        measurementHistory[i].timestamp = 0;
        measurementHistory[i].weight = 0.0;
        measurementHistory[i].configWasSet = false;
        memset(&measurementHistory[i].config, 0, sizeof(PowderConfig));
      }
      currentSessionStartTime = time(nullptr);
      Serial.println("Calibration complete. Session statistics reset to prevent calibration sample from being counted.");

      currentCalibrationState = CALIBRATE_NONE;
      alarmActive = false; // Calibration finished, reset alarm
    } else {
      Serial.println("Error: Cannot calibrate Grains/ADC_Diff factor. ADC difference or known weight is zero/too small.");
    }
  } else {
    Serial.println("Error: Not in Known Grains calibration state.");
  }
  sendCurrentStateToClients();
}

void cancelCalibration() {
  currentCalibrationState = CALIBRATE_NONE;
  Serial.println("Calibration cancelled.");
  alarmActive = false; // Calibration cancelled, reset alarm
  sendCurrentStateToClients();
}

/**
 * @brief Directly draws a test pattern to the display without using buffer
 * @param color The color to use for the test rectangle
 * @param counter The counter value to display
 */
// Function removed as it's no longer needed


/**
 * @brief Draws the main measurement screen on the display.
 * @param weight Current powder weight.
 * @param alarmActive True if alarm is active.
 * @param lowThreshold Low alarm threshold.
 * @param highThreshold High alarm threshold.
 */
void drawMeasurementScreen(float weight, bool alarmActive, float lowThreshold, float highThreshold) {
  // Rotation is set in setupDisplay()

  // --- Weight Display (Large) ---
  display.setTextSize(3);
  display.setTextColor(TFT_WHITE); // Set text color
  char weightStr[15];
  snprintf(weightStr, sizeof(weightStr), "%.3f grain", weight); // 3 decimal places for precision
  
  // Clear area for weight string (clear the whole line to avoid artifacts)
  display.fillRect(0, 5, display.width(), display.fontHeight() * 3, TFT_BLACK); // Clear enough for 3 lines of text size 3
  uint16_t w = display.textWidth(weightStr);
  display.setCursor((display.width() - w) / 2, 5); // Center horizontally
  display.print(weightStr);

  // --- Caliber and Powder Info ---
  display.setTextSize(2);
  display.setTextColor(TFT_CYAN);

  // Clear and draw config name
  display.fillRect(0, 45, display.width(), display.fontHeight() * 2, TFT_BLACK); // Clear enough for 2 lines of text size 2
  display.setCursor(5, 45);
  if (currentConfigIndex != -1) {
    display.print(powderConfigs[currentConfigIndex].name);
  } else {
    display.print("No Config");
  }

  // Clear and draw target grain
  display.fillRect(0, 70, display.width(), display.fontHeight() * 2, TFT_BLACK); // Clear enough for 2 lines of text size 2
  display.setCursor(5, 70);
  if (currentConfigIndex != -1) {
    display.printf("T: %.3f gr", powderConfigs[currentConfigIndex].targetGrain);
  } else {
    display.print("T: ---");
  }

  // --- Alarm Status (Bigger and Center) ---
  display.setTextSize(3); // Increased font size for ALARM/OK
  char alarmStatusStr[15]; // Increased size for "Powdersense"
  if (currentConfigIndex == -1) {
    display.setTextColor(TFT_WHITE);
    strcpy(alarmStatusStr, "Powdersense");
  } else if (weight < lowThreshold) {
    display.setTextColor(TFT_BLUE);
    strcpy(alarmStatusStr, "LOW");
  } else if (weight > highThreshold) {
    display.setTextColor(TFT_RED);
    strcpy(alarmStatusStr, "HIGH");
  } else {
    display.setTextColor(TFT_GREEN);
    strcpy(alarmStatusStr, "PERFECT");
  }

  // Calculate position for center (moved up one line)
  w = display.textWidth(alarmStatusStr);
  uint16_t h = display.fontHeight();
  int alarmX = (display.width() - w) / 2; // Center horizontally
  int alarmY = display.height() - h - 25; // Moved up one line (was -5, now -25)
  
  // Clear area for alarm status
  display.fillRect(0, display.height() - h - 30, display.width(), h + 10, TFT_BLACK); // Clear area for alarm status
  display.setCursor(alarmX, alarmY);
  display.print(alarmStatusStr);

  // --- IP Address (Small at Bottom) ---
  display.setTextSize(1); // Small font for IP
  display.setTextColor(TFT_WHITE);
  String ipStr = WiFi.localIP().toString();
  uint16_t ipW = display.textWidth(ipStr);
  int ipX = (display.width() - ipW) / 2; // Center horizontally
  int ipY = display.height() - 10; // 10 pixels from bottom
  
  // Clear area for IP address
  display.fillRect(0, display.height() - 15, display.width(), 15, TFT_BLACK);
  display.setCursor(ipX, ipY);
  display.print(ipStr);

  // --- Vertical Bar Graph (Scale) ---
  // This part of the screen layout needs to be re-evaluated if the bar is always on the right.
  // If the screen is rotated, the "right" side changes.
  // For now, I'll assume the bar is always on the "physical" right side of the display,
  // which means its coordinates might need to be fixed relative to the native resolution,
  // or the drawing logic needs to be more complex to adapt to rotation.
  // Given the current setup, the bar will move with the rotation.
  // If the bar is always supposed to be on the right of the *physical* display,
  // regardless of rotation, then its coordinates need to be fixed to the native resolution
  // and then transformed by LovyanGFX's internal rotation matrix.
  // For simplicity, I'll keep it relative to display.width() for now, meaning it moves with rotation.

  int barX = display.width() - 50; // X position of the bar (right side of the *logical* screen)
  int barY = 5;                // Y position of the top of the bar
  int barWidth = 40;           // Width of the bar
  int barHeight = display.height() - 10; // Height of the bar (almost full screen height)

  // Clear the entire bar area before redrawing to prevent artifacts
  display.fillRect(barX, barY, barWidth, barHeight, TFT_BLACK);
  
  // Draw bar background outline
  display.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);

  // Calculate maxDisplayWeight dynamically based on thresholds
  // Ensure the range covers from 0 up to at least the high threshold, plus some buffer
  float minVal = 0.0; // Always start from 0 for the bar
  float maxVal = highThreshold * 1.2; // High threshold + 20% buffer
  if (maxVal < 10.0) maxVal = 10.0; // Ensure a minimum scale for very small thresholds

  int fillHeight = map(weight, minVal, maxVal, 0, barHeight);
  fillHeight = constrain(fillHeight, 0, barHeight); // Ensure it stays within bounds

  // Draw filled portion of the bar
  uint16_t barColor = TFT_GREEN;
  if (alarmActive) {
    barColor = TFT_RED;
  } else if (alarmSettings.enabled && (weight < lowThreshold || weight > highThreshold)) {
    barColor = (weight < lowThreshold) ? TFT_BLUE : TFT_RED; // ✅ IMPROVED - Blue for too low, Red for too high
  }
  display.fillRect(barX + 1, barY + barHeight - fillHeight + 1, barWidth - 2, fillHeight - 2, barColor);

  // Draw alarm thresholds on the bar if enabled
  if (alarmSettings.enabled) {
    // Low threshold line
    int lowY = map(lowThreshold, minVal, maxVal, 0, barHeight);
    lowY = barY + barHeight - lowY; // Invert Y for drawing from top
    display.drawFastHLine(barX, lowY, barWidth, TFT_BLUE); // Blue line for low threshold

    // High threshold line
    int highY = map(highThreshold, minVal, maxVal, 0, barHeight);
    highY = barY + barHeight - highY; // Invert Y for drawing from top
    display.drawFastHLine(barX, highY, barWidth, TFT_BLUE); // Blue line for high threshold

  // --- Touch Buttons ---
  }
}

/**
 * @brief Draws the calibration screen on the display.
 * @param state Current calibration state.
 * @param currentAdc Current ADC reading.
 * @param currentWeight Current weight in grains.
 */
void drawCalibrationScreen(CalibrationState state, float currentAdc, float currentWeight) { // Changed to currentWeight
  // Rotation is set in setupDisplay()
  display.setTextColor(TFT_WHITE); // Set text color

  // Clear the entire screen to avoid artifacts from previous states
  display.fillScreen(TFT_BLACK);

  display.setCursor(5, 5);
  display.setTextSize(1); // Smallest size for title
  display.println("--- CALIBRATION WIZARD ---"); 

  display.setTextSize(2); // Larger for step title
  display.setCursor(5, 30);
  if (state == CALIBRATE_ZERO_STEP) {
    display.println("Step 1/2: Set Zero"); 
    
    display.setTextSize(1); // Smaller for instructions
    display.setCursor(5, 60);
    display.println("Place probe at 0mm");
    display.setCursor(5, 75); // New line for clarity
    display.println("(empty container).");
    
    display.setCursor(5, 100); // Clear and draw Current ADC
    display.printf("ADC: %.0f", currentAdc); 
    
    display.setCursor(5, 120);
    display.println("Confirm on Web UI."); 
  } else if (state == CALIBRATE_KNOWN_GRAINS_STEP) { // This is now Step 2
    display.println("Step 2/2: Known Grains"); 
    
    display.setTextSize(1); // Smaller for instructions
    display.setCursor(5, 60);
    display.println("Insert probe into");
    display.setCursor(5, 75); // New line for clarity
    display.println("container w/ powder.");
    
    display.setCursor(5, 100); // Clear and draw Current Weight
    display.printf("Weight: %.3f gr", currentWeight); 
    
    display.setCursor(5, 120);
    display.println("Enter weight on Web UI."); 
  }
  
  // Common instruction for cancelling
  display.setTextSize(1);
  display.setCursor(5, display.height() - 15); // Position at bottom

  // --- Touch Buttons ---
  // NOTE: These buttons are currently unused as calibration is driven by the Web UI.
  // They are left here for potential future use on the device screen.
  // drawButton(zeroBtn);
  // drawButton(spanBtn);
  // drawButton(backBtn);
  display.println("Cancel via Web UI."); 
}

/**
 * @brief Draws the AP mode screen on the display.
 */
void drawAPModeScreen() {
  // Rotation is set in setupDisplay()
  display.setTextColor(TFT_WHITE); // Set text color

  display.fillScreen(TFT_BLACK); // Clear screen completely

  display.setCursor(5, 5);
  display.setTextSize(1);
  display.println("--- AP MODE ACTIVE ---"); 
  display.setTextSize(2);
  display.setCursor(5, 30);
  display.println("SSID: PowderSense"); 
  display.setCursor(5, 60);
  display.println("IP: 192.168.4.1"); 
  display.setTextSize(1);
  display.setCursor(5, 90);
  display.println("Connect to this network"); 
  display.setCursor(5, 105);
  display.println("to configure Wi-Fi."); 
}

// --- Touch Handling Functions ---
void handleTouch() {
  uint16_t touchX, touchY;
  if (gfx.getTouch(&touchX, &touchY)) { // Note: This may need touch controller setup
    Serial.printf("Touch detected: X=%d, Y=%d\n", touchX, touchY);

    switch (currentScreenState) {
      case SCREEN_MEASUREMENT:
        handleMainTouch(touchX, touchY);
        break;
      case SCREEN_CALIBRATION:
        // Touch handling disabled for calibration screen as it is Web UI driven
        break;
    }

    delay(200); // Debounce
  }
}

bool isButtonPressed(TouchButton &btn, int touchX, int touchY) {
  return (touchX >= btn.x && touchX <= btn.x + btn.width &&
          touchY >= btn.y && touchY <= btn.y + btn.height &&
          btn.enabled);
}

void drawButton(TouchButton &btn, bool pressed) {
  uint16_t bgColor = pressed ? TFT_DARKGREY : TFT_BLACK;
  uint16_t borderColor = btn.enabled ? btn.color : TFT_DARKGREY;
  uint16_t textColor = btn.enabled ? TFT_WHITE : TFT_DARKGREY;
  
  // Draw button background
  display.fillRect(btn.x, btn.y, btn.width, btn.height, bgColor);
  
  // Draw button border
  display.drawRect(btn.x, btn.y, btn.width, btn.height, borderColor);
  
  // Draw button text
  display.setTextSize(1);
  display.setTextColor(textColor);
  uint16_t textW = display.textWidth(btn.label);
  uint16_t textH = display.fontHeight();
  int textX = btn.x + (btn.width - textW) / 2;
  int textY = btn.y + (btn.height - textH) / 2;
  display.setCursor(textX, textY);
  display.print(btn.label);
}

void handleMainTouch(int x, int y) {
  if (isButtonPressed(calibrateBtn, x, y)) {
    Serial.println("✅ Calibrate button pressed");
    currentScreenState = SCREEN_CALIBRATION;
  } else if (isButtonPressed(profileBtn, x, y)) {
    Serial.println("✅ Profile button pressed");
    // Cycle through profiles or open profile selection
    currentConfigIndex = (currentConfigIndex + 1) % MAX_CONFIGS;
    Serial.printf("Switched to profile %d\n", currentConfigIndex);
  } else if (isButtonPressed(settingsBtn, x, y)) {
    Serial.println("✅ Settings button pressed");
    // Could add settings screen later
  }
}

