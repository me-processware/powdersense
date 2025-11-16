# Project: Powder Depth Measurement System

## Project Requirements & Status

-   **Measure Depth:** Use a 10k slide potentiometer (60mm travel, Bourns PTA6043-2015DPB103) with a theoretical precision of **0.0146 mm**.
    -   Status: **In Progress** (Initial setup with ESP32 ADC, precision accepted, range corrected)
-   **Presets/Thresholds:** Define depth thresholds.
    -   Status: **To Do**
-   **Logging:** Log entries on the ESP32.
    -   Status: **To Do**
-   **Visual Feedback:** Use green/red LEDs to indicate if thresholds are met.
    -   Status: **To Do**
-   **User Interface:** 1.47-inch TFT LCD screen (ST7789) and 4 buttons to set thresholds, reset etc.
    -   Status: **In Progress** (Display initialized, basic text output implemented)
-   **Persistence:** Store all settings on the ESP32 (EEPROM/SPIFFS).
    -   Status: **To Do**
-   **Data Visualization:** Display measurements in a graph on the TFT LCD.
    -   Status: **To Do**
-   **Data Export:** Download measurements as a CSV file (via web interface).
    -   Status: **In Progress** (Basic web server and data endpoint)
-   **Web Interface:** HTML page to view and track powder loads.
    -   Status: **In Progress** (Basic HTML page served)

## Progress Log

### 2023-10-27
-   Initialized project structure.
-   Created `project.md`, `README.md`, `platformio.ini`, and `src/main.cpp`.
-   Set up basic ESP32 Arduino sketch for potentiometer reading.
-   Identified precision challenge with 10k pot and ESP32 ADC.

### 2023-10-28
-   Confirmed 0.18mm precision is acceptable.
-   Added web interface requirement for viewing and tracking powder loads.
-   Updated `platformio.ini` with Wi-Fi and WebServer libraries.
-   Implemented basic Wi-Fi connectivity and web server in `src/main.cpp`.
-   Created `data/index.html` and `data/style.css` for the web interface.
-   Added API endpoint for current depth data.

### 2023-10-29
-   Corrected potentiometer measurement range from 75cm to 75mm.
-   Temporarily disabled OLED display functionality as it is not currently connected.
-   Added more detailed serial logging for debugging boot process and file serving.

### 2023-10-30
-   Updated potentiometer travel to 60mm based on Bourns PTA6043-2015DPB103 datasheet.
-   Recalculated theoretical precision based on new travel distance.
-   Reviewed `partitions/default_8MB.csv` - no errors found.
-   Reviewed `partitions/default_4MB.csv` - no errors found.
-   Successfully resolved boot loop and SPIFFS mount issues. Device now boots, connects to WiFi, and mounts SPIFFS.
-   Next step: Verify SPIFFS content upload and web page display.

### 2023-11-01
-   **Hardware Change:** Switched from ESP32-S3 to ESP32-C6 1.47-inch LCD development board.
-   Updated `platformio.ini` to target `esp32-c6-devkitc-1` and added `Adafruit_ST7735_and_ST7789_Library`.
-   Modified `src/main.cpp` to initialize and use the ST7789 TFT display instead of SSD1306 OLED.
-   Adjusted display output functions (`setupDisplay`, `displayExampleScreen`, `loop` display logic) for the new screen resolution (172x320) and capabilities.
-   Updated `project.md` to reflect the new hardware and display.

### 2023-11-02
-   **Framework Change:** Migrated from Arduino framework to ESP-IDF for better ESP32-C6 compatibility and direct hardware control.
-   Updated `platformio.ini` to use `framework = espidf`.
-   `src/main.cpp` has been converted to a minimal ESP-IDF application, focusing on basic display initialization. Further application logic (Wi-Fi, WebSockets, etc.) will need to be re-implemented using ESP-IDF APIs.
-   **Pinout Update:** Updated `src/main.cpp`, `README.md`, and `project.md` with verified pin definitions for the Waveshare ESP32-C6-LCD-1.47 board.

## Version History

-   **0.1.0** (2023-10-27): Initial project setup and basic potentiometer reading.
-   **0.2.0** (2023-10-28): Implemented basic web interface for real-time depth viewing.
-   **0.2.1** (2023-10-29): Corrected potentiometer range and temporarily disabled OLED.
-   **0.2.2** (2023-10-30): Updated potentiometer specifications and theoretical precision.
-   **0.3.0** (2023-11-01): Migrated to ESP32-C6 with 1.47-inch ST7789 TFT LCD.
-   **0.4.0** (2023-11-02): Switched to ESP-IDF framework and updated pinout for Waveshare board.

## Technical Stack

-   **Microcontroller:** ESP32-C6
-   **Development Environment:** PlatformIO (VS Code)
-   **Programming Language:** C (ESP-IDF), HTML, CSS, JavaScript
-   **Framework:** Espressif IoT Development Framework (ESP-IDF)
-   **Libraries (Initial ESP-IDF):**
    -   `driver/gpio`
    -   `driver/spi_master`
    -   `esp_log`
    -   `freertos/FreeRTOS`
    -   `freertos/task`
    -   (Further libraries for Wi-Fi, HTTP server, WebSockets, display graphics like LVGL will be added as needed)
-   **Hardware:**
    -   **Waveshare ESP32-C6-LCD-1.47** development board (with integrated ST7789 TFT LCD)
    -   10k Slide Potentiometer (60mm travel, Bourns PTA6043-2015DPB103)
    -   Push Buttons (x4)
    -   Onboard WS2812 RGB LED

## Notes

-   Theoretical precision of 0.0146 mm over 60mm with a 10k potentiometer and ESP32's 12-bit ADC is achievable. Calibration and noise reduction techniques will be crucial for practical accuracy.
-   Consider power supply for the potentiometer to ensure stable readings.
-   The `data` folder for SPIFFS content needs to be uploaded using PlatformIO's "Upload File System Image" command.
-   **IMPORTANT:** The current `src/main.cpp` is a minimal ESP-IDF example for display initialization. Migrating the full application logic (Wi-Fi, WebSockets, JSON parsing, sensor reading, button handling, FastLED, etc.) from the Arduino framework to ESP-IDF's native APIs is a significant undertaking and will require substantial development.
