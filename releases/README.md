# PowderSense Pre-Built Firmware Releases

This directory contains pre-built firmware binaries for users who want to flash PowderSense without compiling from source.

---

## 📦 Available Releases

### [v1.0](v1.0/) - Latest Stable

**Release Date**: November 2025  
**Status**: Stable

**What's New**:
- Initial open source release
- ESP32-C6 support with 1.47" LCD
- WiFi configuration via Access Point
- Web-based dashboard and configuration
- Multi-profile support
- Data logging with CSV export
- Automatic measurement with stability detection

**Files**:
- `firmware.bin` - Main application firmware (~1.2MB)
- `spiffs.bin` - Web interface files (~1.5MB)
- `FLASHING.md` - Detailed flashing instructions

**Quick Flash**:
```bash
esptool.py --chip esp32c6 --port COM3 write_flash 0x10000 firmware.bin
esptool.py --chip esp32c6 --port COM3 write_flash 0x290000 spiffs.bin
```

---

## 🚀 Quick Start

### Option 1: Pre-Built Binaries (This Directory)

**For users who want to flash without compiling**:

1. **Download** firmware from `releases/v1.0/`
2. **Install** esptool: `pip install esptool`
3. **Connect** ESP32-C6 via USB
4. **Flash** firmware:
   ```bash
   cd releases/v1.0
   esptool.py --chip esp32c6 --port COM3 write_flash 0x10000 firmware.bin
   esptool.py --chip esp32c6 --port COM3 write_flash 0x290000 spiffs.bin
   ```
5. **Configure** WiFi via Access Point
6. **Calibrate** and start measuring

**See**: [v1.0/FLASHING.md](v1.0/FLASHING.md) for detailed instructions

---

### Option 2: Compile from Source

**For developers who want to modify the code**:

1. **Clone** repository
2. **Install** PlatformIO + VS Code
3. **Build** and upload
4. **See**: Main [README.md](../README.md) for build instructions

---

## 🔄 Updating Firmware

**To update to a newer version**:
1. Download new firmware binaries
2. Flash using same commands
3. WiFi credentials and calibration are preserved

**To do a clean install**:
```bash
esptool.py --chip esp32c6 --port COM3 erase_flash
# Then flash firmware + spiffs
```

---

## 📋 Version History

| Version | Date | Status | Notes |
|---------|------|--------|-------|
| v1.0 | Nov 2025 | Stable | Initial open source release |

---

## 🛠️ Hardware Requirements

- **Board**: Waveshare ESP32-C6 1.47" LCD (touch version)
- **USB**: USB-C cable (data capable)
- **Drivers**: CH340/CP2102 (usually auto-detected)

---

## 📖 Documentation

- **Flashing Guide**: [v1.0/FLASHING.md](v1.0/FLASHING.md)
- **User Manual**: [docs/USER_MANUAL.md](../docs/USER_MANUAL.md)
- **Assembly Guide**: [docs/ASSEMBLY.md](../docs/ASSEMBLY.md)
- **BOM**: [docs/BOM.md](../docs/BOM.md)

---

## 🆘 Support

**Issues with flashing?**
- Check [FLASHING.md](v1.0/FLASHING.md) troubleshooting section
- GitHub Issues: https://github.com/me-processware/powdersense/issues
- Email: info@processware.nl

---

## 🔐 Verification

All binaries are built from tagged releases in this repository. You can verify by:
1. Checking out the corresponding git tag
2. Building from source with PlatformIO
3. Comparing binary checksums

**Build Environment**:
- PlatformIO Core 6.1.15
- ESP-IDF framework
- GCC 11.2.0

---

*Pre-built binaries by Processware*  
*Part of the PowderSense Open Source Project*
