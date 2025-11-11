# PowderSense v1.0 - Pre-Built Firmware

**Release Date**: November 2025  
**Status**: Stable Release

---

## 📦 Files in This Release

### Required for Flashing

| File | Size | Description | Flash Address |
|------|------|-------------|---------------|
| `firmware.bin` | ~1.2MB | Main application firmware | 0x10000 |
| `spiffs.bin` | ~1.5MB | Web interface (HTML/CSS/JS) | 0x290000 |

### Optional Reference Files

| File | Description |
|------|-------------|
| `firmware.elf` | Debug symbols (not needed for flashing) |
| `firmware.map` | Memory map (not needed for flashing) |
| `partitions.csv` | Partition table source |

---

## 🚀 Quick Flash

**Prerequisites**: Install esptool
```bash
pip install esptool
```

**Flash Command**:
```bash
# Flash firmware
esptool.py --chip esp32c6 --port COM3 write_flash 0x10000 firmware.bin

# Flash web interface
esptool.py --chip esp32c6 --port COM3 write_flash 0x290000 spiffs.bin
```

**Note**: Replace `COM3` with your actual port (Windows: COM3, Linux: /dev/ttyUSB0, macOS: /dev/cu.usbserial-*)

---

## 📖 Detailed Instructions

See [FLASHING.md](FLASHING.md) for:
- Complete flashing guide
- Multiple flashing methods (esptool, ESP Flash Tool, PlatformIO)
- Memory address details
- Troubleshooting
- Update procedures

---

## ✨ What's New in v1.0

### Features
- ✅ ESP32-C6 support with 1.47" TFT LCD
- ✅ WiFi configuration via Access Point mode
- ✅ Web-based dashboard with real-time measurements
- ✅ Multi-profile configuration system
- ✅ Data logging with CSV export
- ✅ Automatic measurement with stability detection
- ✅ Visual feedback (color-coded status: LOW/PERFECT/HIGH)
- ✅ Session statistics tracking

### Hardware Support
- **Board**: Waveshare ESP32-C6 1.47" LCD
- **Display**: ST7789 TFT (172x320 pixels)
- **ADC**: ADS1115 16-bit I2C
- **Sensor**: 10kΩ slide potentiometer (60mm travel)

### Known Limitations
- ⚠️ Touch interface not implemented (display only)
- ⚠️ OTA updates not implemented (flash via USB)

---

## 🔧 Hardware Requirements

**Minimum**:
- Waveshare ESP32-C6 1.47" LCD board
- USB-C cable (data capable)
- Computer with USB port

**For Complete System**:
- See [BOM.md](../../docs/BOM.md) for full component list
- See [ASSEMBLY.md](../../docs/ASSEMBLY.md) for build instructions

---

## 📝 First-Time Setup

After flashing:

1. **Power On**: Connect USB-C cable
2. **Access Point**: Device creates "PowderSense-XXXX" WiFi
3. **Connect**: Join the WiFi (password: "powdersense")
4. **Configure**: Navigate to 192.168.4.1 and set WiFi credentials
5. **Reboot**: Device connects to your network
6. **Access**: Open web interface at device IP
7. **Calibrate**: Follow on-screen calibration wizard
8. **Measure**: Start using PowderSense!

**See**: [USER_MANUAL.md](../../docs/USER_MANUAL.md) for detailed setup and usage

---

## 🔄 Updating from Previous Version

**This is the first release**, no previous version exists.

For future updates:
- WiFi credentials will be preserved
- Calibration data will be preserved
- Configuration profiles will be preserved

---

## 🐛 Known Issues

None reported yet. Please report issues at:
https://github.com/me-processware/powdersense/issues

---

## 🆘 Support

**Flashing Problems?**
- See [FLASHING.md](FLASHING.md) troubleshooting section
- Check USB cable (must support data)
- Try lower baud rate: `--baud 115200`

**Usage Questions?**
- Read [USER_MANUAL.md](../../docs/USER_MANUAL.md)
- GitHub Issues: https://github.com/me-processware/powdersense/issues
- Email: info@processware.nl

---

## 📊 Build Information

- **Version**: 1.0
- **Build Date**: November 2025
- **Git Commit**: TBD (will be added when tagged)
- **Platform**: ESP32-C6
- **Framework**: Arduino (ESP-IDF)
- **PlatformIO**: 6.1.15
- **Compiler**: GCC 11.2.0

---

## 🔐 Checksums

**bootloader.bin**:
- MD5: `9985e72399d371ee61de7c4d1d669485`
- SHA256: `a57485bef96b7be694204427b70d920263f076a352956cb0e36b9ffce0fa12a8`

**partitions.bin**:
- MD5: `588df43564950cbb0bd3c5fb5c081df0`
- SHA256: `5233eab607f53cb0d175f13f25cf917e5496e340e2565188cf9550b4080c449c`

**firmware.bin**:
- MD5: `ba6a9b08752c0f15e5bdd31c9f690724`
- SHA256: `f2a37c0f4d5cb3068e576668ed19a5684c8047fa11ae392ddbd338c04efd0115`

**spiffs.bin**:
- MD5: `10b9ae4c20b10dfc378bfb63925f72f4`
- SHA256: `d3db9c5fb5bb24c81f5cb9a64315ccbd27517e52b2513ab54a03e943bd2a8304`

**See [CHECKSUMS.md](CHECKSUMS.md) for detailed verification instructions.**

---

*Pre-built by Processware - November 2025*  
*Part of the PowderSense Open Source Project*
