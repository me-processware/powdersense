#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// ========================================
// BOARD CONFIGURATION
// ========================================
// Select your board variant in platformio.ini using build_flags
// Example: -D BOARD_WAVESHARE_CLONE or -D BOARD_CUSTOM_RGB

#if defined(BOARD_ESP32C6_TOUCH)
    // ========================================
    // ESP32-C6 with Touch (Default)
    // ========================================
    #define BOARD_NAME "ESP32-C6 Touch"
    
    // Display (ST7789) Pins
    #define TFT_SCLK 1
    #define TFT_MOSI 2
    #define TFT_DC   15
    #define TFT_CS   14
    #define TFT_RST  22
    #define TFT_BL   23
    
    // Analog Input Pin (Potentiometer/Level Sensor)
    #define LEVEL_SENSOR_PIN 5
    
    // I2C Pins (for ADS1115 ADC)
    #define ADS1115_SDA_PIN 18
    #define ADS1115_SCL_PIN 19
    #define ADS1115_ADDRESS 0x48  // ADDR pin connected to ground
    #define ADS1115_CHANNEL 0     // A0 pin for VDS measurement
    
    // RGB LED - NOT PRESENT on this board
    #undef HAS_RGB_LED
    
    // Display color inversion setting
    #define DISPLAY_INVERT false
    
#elif defined(BOARD_ESP32C6_NO_TOUCH_RGB)
    // ========================================
    // ESP32-C6 No Touch with RGB LED
    // ========================================
    #define BOARD_NAME "ESP32-C6 No Touch RGB"
    
    // Display (ST7789) Pins
    #define TFT_SCLK 7
    #define TFT_MOSI 6
    #define TFT_DC   15
    #define TFT_CS   14
    #define TFT_RST  21
    #define TFT_BL   22
    
    // Display color inversion setting
    #define DISPLAY_INVERT true
    
    // Analog Input Pin (Potentiometer/Level Sensor)
    #define LEVEL_SENSOR_PIN 3  // GPIO3 (ADC1_CH3) - free pin for sensor
    
    // I2C Pins (for ADS1115 ADC)
    #define ADS1115_SDA_PIN 18
    #define ADS1115_SCL_PIN 19
    #define ADS1115_ADDRESS 0x48  // ADDR pin connected to ground
    #define ADS1115_CHANNEL 0     // A0 pin for VDS measurement
    
    // RGB LED Configuration
    // Note: HAS_RGB_LED is defined in platformio.ini build_flags
    #define RGB_LED_PIN 8
    #define NUM_LEDS 1
    
#else
    #error "No board defined! Add -D BOARD_ESP32C6_TOUCH or -D BOARD_ESP32C6_NO_TOUCH_RGB to platformio.ini build_flags"
#endif

// ========================================
// DISPLAY CONFIGURATION (Common)
// ========================================
#define DISPLAY_WIDTH  172
#define DISPLAY_HEIGHT 320
#define DISPLAY_OFFSET_X 34
#define DISPLAY_OFFSET_Y 0

// Board-specific display rotation
#if defined(BOARD_ESP32C6_TOUCH)
    #define DISPLAY_ROTATION 7  // USB connector on left, horizontal readable
#elif defined(BOARD_ESP32C6_NO_TOUCH_RGB)
    #define DISPLAY_ROTATION 1  // Landscape mode (320x172)
#else
    #define DISPLAY_ROTATION 0  // Default rotation
#endif

// ========================================
// FEATURE FLAGS (Common)
// ========================================
// Uncomment to enable features
// #define ENABLE_TOUCH_CONTROLLER
// #define ENABLE_IMU_SENSOR

#endif // BOARD_CONFIG_H
