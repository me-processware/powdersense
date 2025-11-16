// User_Setup.h for TFT_eSPI
#define USER_SETUP_INFO "User_Setup for Waveshare ESP32-C6-Touch-LCD-1.47"

// Define the ST7796 driver
#define ST7796_DRIVER

// Define pins for ESP32-C6-Touch-LCD-1.47
#define TFT_MISO -1
#define TFT_MOSI 2
#define TFT_SCLK 1
#define TFT_CS   14
#define TFT_DC   15
#define TFT_RST  22
#define TFT_BL   23  // Backlight control pin

// Define the display dimensions
#define TFT_WIDTH  172
#define TFT_HEIGHT 320

// Rotation setting (0-3)
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

// SPI frequency - reduced for stability
#define SPI_FREQUENCY  10000000

// Display offset
#define TFT_OFFSET_X 34
#define TFT_OFFSET_Y 0
