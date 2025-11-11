# PowderSense v1.0 - Pre-Built Firmware Flashing Guide

This directory contains pre-built firmware binaries for PowderSense v1.0, allowing you to flash the device without compiling from source.

---

## 📦 What's Included

### Required Files (for flashing)

| File | Size | Description | Flash Address |
|------|------|-------------|---------------|
| `bootloader.bin` | ~24KB | ESP32-C6 bootloader | 0x0000 |
| `partitions.bin` | ~3KB | Partition table | 0x8000 |
| `firmware.bin` | ~1.2MB | Main application firmware | 0x10000 |
| `spiffs.bin` | ~1.5MB | Web interface files (HTML/CSS/JS) | 0x290000 |

**Important Notes**:
- **Waveshare boards come with bootloader and partitions pre-flashed**
- For **updates**: You only need `firmware.bin` and `spiffs.bin`
- For **first-time** or **after erase**: You need all 4 files

### Optional Files (for reference)

| File | Description |
|------|-------------|
| `firmware.elf` | Debug symbols (not needed for flashing) |
| `firmware.map` | Memory map (not needed for flashing) |

---

## 🔧 Flashing Methods

### Method 1: Using esptool.py (Recommended)

**Prerequisites**:
```bash
pip install esptool
```

**Flash Command** (all-in-one):
```bash
esptool.py --chip esp32c6 --port COM3 --baud 921600 \
  --before default_reset --after hard_reset write_flash \
  -z --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin \
  0x290000 spiffs.bin
```

**Note**: Replace `COM3` with your actual port:
- **Windows**: `COM3`, `COM4`, etc. (check Device Manager)
- **Linux**: `/dev/ttyUSB0`, `/dev/ttyACM0`
- **macOS**: `/dev/cu.usbserial-*`

**Simplified Flash** (firmware + spiffs only):
```bash
# Flash firmware
esptool.py --chip esp32c6 --port COM3 write_flash 0x10000 firmware.bin

# Flash web interface
esptool.py --chip esp32c6 --port COM3 write_flash 0x290000 spiffs.bin
```

---

### Method 2: Using ESP Flash Download Tool (Windows GUI)

**Download**: [ESP Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)

**Steps**:
1. Open ESP Flash Download Tool
2. Select **ESP32-C6**
3. Configure flash settings:
   - **SPI SPEED**: 80MHz
   - **SPI MODE**: DIO
   - **FLASH SIZE**: 4MB
4. Add files with addresses:
   - `bootloader.bin` @ `0x0000`
   - `partitions.bin` @ `0x8000`
   - `firmware.bin` @ `0x10000`
   - `spiffs.bin` @ `0x290000`
5. Select COM port
6. Click **START**

---

### Method 3: Using PlatformIO (if you have VS Code)

**If you have PlatformIO installed but don't want to compile**:

1. Copy `firmware.bin` to `.pio/build/esp32c6_touch/`
2. Copy `spiffs.bin` to `.pio/build/esp32c6_touch/`
3. In PlatformIO:
   - Click **Upload** (uploads firmware)
   - Click **Upload Filesystem Image** (uploads spiffs)

---

## 📍 Memory Addresses (ESP32-C6)

| Component | Address | Size | Description |
|-----------|---------|------|-------------|
| Bootloader | 0x0000 | ~24KB | ESP32-C6 bootloader |
| Partition Table | 0x8000 | 3KB | Partition layout |
| NVS | 0x9000 | 20KB | Non-volatile storage |
| OTA Data | 0xE000 | 8KB | OTA update metadata |
| **Firmware** | **0x10000** | ~1.2MB | **Main application** |
| **SPIFFS** | **0x290000** | ~1.5MB | **Web interface files** |

---

## ⚠️ Troubleshooting

### "Failed to connect"
- Hold **BOOT** button while connecting USB
- Try lower baud rate: `--baud 115200`
- Check USB cable (must support data, not just charging)
- Install CH340/CP2102 drivers if needed

### "Invalid head of packet"
- Wrong chip type selected (ensure ESP32-C6)
- Corrupted binary file (re-download)
- Bad USB connection

### "Flash will be erased"
- Normal! This erases old firmware before writing new
- Your WiFi credentials will be preserved (stored in NVS)

### "A fatal error occurred"
- Check COM port is correct
- Close other programs using serial port (Arduino IDE, PuTTY, etc.)
- Try different USB port

---

## 🔄 Updating Firmware

**To update to a newer version**:
1. Download new `firmware.bin` and `spiffs.bin`
2. Flash using same commands as above
3. Device will reboot automatically
4. WiFi credentials and calibration data are preserved

**To do a clean install** (erase everything):
```bash
# Erase entire flash
esptool.py --chip esp32c6 --port COM3 erase_flash

# Then flash firmware + spiffs
esptool.py --chip esp32c6 --port COM3 write_flash 0x10000 firmware.bin
esptool.py --chip esp32c6 --port COM3 write_flash 0x290000 spiffs.bin
```

---

## ✅ Verification

After flashing:
1. Open serial monitor (115200 baud)
2. Press **RESET** button on device
3. You should see boot messages
4. Device creates WiFi AP: "PowderSense-XXXX"
5. Connect and configure WiFi

---

## 📝 Build Information

- **Version**: 1.0
- **Build Date**: November 2025
- **Chip**: ESP32-C6
- **Board**: Waveshare ESP32-C6 1.47" LCD
- **Framework**: Arduino (ESP-IDF)
- **Compiler**: GCC 11.2.0

---

## 🆘 Support

**Issues with flashing?**
- GitHub Issues: https://github.com/me-processware/powdersense/issues
- Email: info@processware.nl

**Want to compile from source instead?**
- See main README.md for build instructions
- Requires PlatformIO and Visual Studio Code

---

*Pre-built binaries by Processware - November 2025*  
*Part of the PowderSense Open Source Project*
