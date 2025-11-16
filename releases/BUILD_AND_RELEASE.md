# Building and Releasing PowderSense Firmware

This guide explains how to build firmware binaries and prepare them for release.

---

## 📦 Required Files Overview

### Essential Files (Always Needed)

| File | Location After Build | Flash Address | Description |
|------|---------------------|---------------|-------------|
| `bootloader.bin` | `.pio/build/esp32c6_touch/` | 0x0000 | ESP32-C6 bootloader |
| `partitions.bin` | `.pio/build/esp32c6_touch/` | 0x8000 | Partition table |
| `firmware.bin` | `.pio/build/esp32c6_touch/` | 0x10000 | Main application |
| `spiffs.bin` | `.pio/build/esp32c6_touch/` | 0x290000 | Web interface files |

### Optional Files (For Reference)

| File | Description |
|------|-------------|
| `firmware.elf` | Debug symbols (for debugging) |
| `firmware.map` | Memory map (for analysis) |

---

## 🔨 Building Firmware

### Step 1: Build Firmware

**In VS Code with PlatformIO**:
1. Open project folder
2. Click **Build** (✓ icon in status bar)
3. **OR** press `Ctrl+Alt+B`
4. Wait for compilation to complete

**Command Line** (alternative):
```bash
cd /path/to/powdersense
pio run -e esp32c6_touch
```

**Output**: Firmware files in `.pio/build/esp32c6_touch/`

---

### Step 2: Build Filesystem (SPIFFS)

**In VS Code with PlatformIO**:
1. Click PlatformIO icon (alien head) in sidebar
2. Expand **esp32c6_touch** environment
3. Expand **Platform**
4. Click **Build Filesystem Image**

**Command Line** (alternative):
```bash
pio run -e esp32c6_touch -t buildfs
```

**Output**: `spiffs.bin` in `.pio/build/esp32c6_touch/`

---

## 📂 Locating Build Files

After building, all files are in:
```
.pio/build/esp32c6_touch/
├── bootloader.bin       # ESP32-C6 bootloader (from ESP-IDF)
├── partitions.bin       # Partition table (from platformio.ini)
├── firmware.bin         # Your compiled application
├── firmware.elf         # Debug symbols
├── firmware.map         # Memory map
└── spiffs.bin           # Web interface (after buildfs)
```

**Windows Path**:
```
C:\Users\YourName\Documents\PlatformIO\Projects\powdersense\.pio\build\esp32c6_touch\
```

**Copy to Releases**:
```bash
# Windows (PowerShell)
Copy-Item .pio\build\esp32c6_touch\bootloader.bin releases\v1.0\
Copy-Item .pio\build\esp32c6_touch\partitions.bin releases\v1.0\
Copy-Item .pio\build\esp32c6_touch\firmware.bin releases\v1.0\
Copy-Item .pio\build\esp32c6_touch\spiffs.bin releases\v1.0\

# Linux/macOS
cp .pio/build/esp32c6_touch/bootloader.bin releases/v1.0/
cp .pio/build/esp32c6_touch/partitions.bin releases/v1.0/
cp .pio/build/esp32c6_touch/firmware.bin releases/v1.0/
cp .pio/build/esp32c6_touch/spiffs.bin releases/v1.0/
```

---

## 🚀 Flashing to ESP32-C6

### Method 1: Using PlatformIO (Easiest During Development)

**Upload Firmware**:
- Click **Upload** (→ icon) in status bar
- **OR** press `Ctrl+Alt+U`

**Upload Filesystem**:
- PlatformIO sidebar → **Upload Filesystem Image**
- **OR** command: `pio run -t uploadfs`

**This automatically flashes**:
- Bootloader (if needed)
- Partitions (if needed)
- Firmware
- SPIFFS (when using uploadfs)

---

### Method 2: Using esptool.py (For Manual Flashing)

**Install esptool**:
```bash
pip install esptool
```

#### Option A: Flash Everything (First Time / Clean Install)

```bash
esptool.py --chip esp32c6 --port COM3 --baud 921600 \
  --before default_reset --after hard_reset write_flash \
  -z --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin \
  0x290000 spiffs.bin
```

**Note**: Replace `COM3` with your port:
- Windows: `COM3`, `COM4` (check Device Manager)
- Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`
- macOS: `/dev/cu.usbserial-*`

#### Option B: Flash Firmware Only (Updates)

**For firmware updates** (bootloader/partitions already on device):
```bash
esptool.py --chip esp32c6 --port COM3 write_flash 0x10000 firmware.bin
```

#### Option C: Flash Firmware + Web Interface

**Most common for updates**:
```bash
# Flash firmware
esptool.py --chip esp32c6 --port COM3 write_flash 0x10000 firmware.bin

# Flash web interface
esptool.py --chip esp32c6 --port COM3 write_flash 0x290000 spiffs.bin
```

---

### Method 3: Using ESP Flash Download Tool (Windows GUI)

**Download**: [ESP Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)

**Steps**:
1. Open tool and select **ESP32-C6**
2. Add files with addresses:
   - `bootloader.bin` @ `0x0000` ✓
   - `partitions.bin` @ `0x8000` ✓
   - `firmware.bin` @ `0x10000` ✓
   - `spiffs.bin` @ `0x290000` ✓
3. Configure:
   - SPI SPEED: 80MHz
   - SPI MODE: DIO
   - FLASH SIZE: 4MB
4. Select COM port
5. Click **START**

---

## 🔍 Why Bootloader and Partitions?

### Bootloader.bin (0x0000)
- **What**: ESP32-C6 bootloader from ESP-IDF
- **Purpose**: Initializes hardware and loads firmware
- **When to flash**: 
  - ✅ First time flashing a new board
  - ✅ After erasing entire flash
  - ❌ Not needed for firmware updates

### Partitions.bin (0x8000)
- **What**: Partition table defining flash layout
- **Purpose**: Tells ESP32 where firmware, SPIFFS, NVS are located
- **When to flash**:
  - ✅ First time flashing
  - ✅ When partition layout changes
  - ❌ Not needed for firmware updates

### Firmware.bin (0x10000)
- **What**: Your compiled application code
- **Purpose**: The actual PowderSense application
- **When to flash**:
  - ✅ Every update
  - ✅ Always needed

### SPIFFS.bin (0x290000)
- **What**: Web interface (HTML/CSS/JS files)
- **Purpose**: Dashboard and configuration pages
- **When to flash**:
  - ✅ When web interface changes
  - ✅ First time setup
  - ❌ Not needed if only firmware code changed

---

## 📋 Release Checklist

### 1. Build Binaries
- [ ] Build firmware: `pio run -e esp32c6_touch`
- [ ] Build filesystem: `pio run -e esp32c6_touch -t buildfs`
- [ ] Verify no compilation errors

### 2. Test on Device
- [ ] Flash to test device
- [ ] Verify WiFi AP mode works
- [ ] Verify web interface loads
- [ ] Test calibration
- [ ] Test measurement
- [ ] Check all features work

### 3. Copy to Releases
- [ ] Copy `bootloader.bin` to `releases/v1.0/`
- [ ] Copy `partitions.bin` to `releases/v1.0/`
- [ ] Copy `firmware.bin` to `releases/v1.0/`
- [ ] Copy `spiffs.bin` to `releases/v1.0/`

### 4. Generate Checksums
```bash
cd releases/v1.0/

# Windows (PowerShell)
Get-FileHash firmware.bin -Algorithm MD5
Get-FileHash firmware.bin -Algorithm SHA256
Get-FileHash spiffs.bin -Algorithm MD5
Get-FileHash spiffs.bin -Algorithm SHA256

# Linux/macOS
md5sum firmware.bin spiffs.bin
sha256sum firmware.bin spiffs.bin
```

### 5. Update Documentation
- [ ] Update `releases/v1.0/README.md` with checksums
- [ ] Update file sizes if changed
- [ ] Update build date
- [ ] Delete `PLACE_BINARIES_HERE.txt`

### 6. Commit and Tag
```bash
git add releases/v1.0/
git commit -m "release: add v1.0 firmware binaries"
git tag -a v1.0 -m "PowderSense v1.0 - Initial Release"
git push origin master --tags
```

### 7. Create GitHub Release (Optional)
1. Go to GitHub repository
2. Click **Releases** → **Create a new release**
3. Select tag `v1.0`
4. Upload binaries as release assets
5. Copy release notes from `releases/v1.0/README.md`
6. Publish release

---

## 🔄 For Future Updates

**When releasing v1.1, v1.2, etc.**:

1. Create new directory: `releases/v1.1/`
2. Copy and update README.md and FLASHING.md
3. Build and copy new binaries
4. Update version history in `releases/README.md`
5. Tag and push

---

## 🆘 Troubleshooting

### "Bootloader.bin not found"
- Build firmware first: `pio run -e esp32c6_touch`
- PlatformIO generates bootloader automatically

### "SPIFFS.bin not found"
- Build filesystem: `pio run -e esp32c6_touch -t buildfs`
- Ensure `data/` directory exists with web files

### "Upload failed"
- Check USB cable (must support data)
- Hold BOOT button while connecting
- Try lower baud rate: `--baud 115200`
- Close other programs using serial port

### "Partition table mismatch"
- Flash partitions.bin: `esptool.py write_flash 0x8000 partitions.bin`
- Or do full erase: `esptool.py erase_flash`

---

*Build and Release Guide by Processware - November 2025*  
*Part of the PowderSense Open Source Project*
