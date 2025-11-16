# Generating Checksums on Windows

Windows doesn't have `md5sum` or `sha256sum` commands like Linux. Instead, use **PowerShell**.

---

## Method 1: PowerShell Commands (Manual)

### Open PowerShell

1. Press `Win + X`
2. Select **Windows PowerShell** or **Terminal**
3. Navigate to firmware directory:
   ```powershell
   cd releases\v1.0\
   ```

### Generate MD5 Checksums

```powershell
Get-FileHash -Path firmware.bin -Algorithm MD5
Get-FileHash -Path spiffs.bin -Algorithm MD5
Get-FileHash -Path bootloader.bin -Algorithm MD5
Get-FileHash -Path partitions.bin -Algorithm MD5
```

**Output example**:
```
Algorithm       Hash                                                                   Path
---------       ----                                                                   ----
MD5             A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6                                       C:\...\firmware.bin
```

### Generate SHA256 Checksums

```powershell
Get-FileHash -Path firmware.bin -Algorithm SHA256
Get-FileHash -Path spiffs.bin -Algorithm SHA256
Get-FileHash -Path bootloader.bin -Algorithm SHA256
Get-FileHash -Path partitions.bin -Algorithm SHA256
```

### Get File Sizes

```powershell
Get-Item firmware.bin | Select-Object Name, Length
Get-Item spiffs.bin | Select-Object Name, Length
Get-Item bootloader.bin | Select-Object Name, Length
Get-Item partitions.bin | Select-Object Name, Length
```

---

## Method 2: PowerShell Script (Automated)

We've provided a script that generates all checksums automatically!

### Usage

1. **Copy binaries** to `releases/v1.0/`:
   ```powershell
   Copy-Item .pio\build\esp32c6_touch\*.bin releases\v1.0\
   ```

2. **Navigate** to releases directory:
   ```powershell
   cd releases\v1.0\
   ```

3. **Run script**:
   ```powershell
   .\generate_checksums.ps1
   ```

4. **Output**: Creates `CHECKSUMS.md` with all hashes and file sizes

### Script Output Example

```markdown
# PowderSense v1.0 - Firmware Checksums

Generated: 2025-11-10 15:30:00

## bootloader.bin

- **Size**: 24576 bytes (24.00 KB / 0.02 MB)
- **MD5**: `A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6`
- **SHA256**: `1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF`

## partitions.bin

- **Size**: 3072 bytes (3.00 KB / 0.00 MB)
- **MD5**: `X1Y2Z3A4B5C6D7E8F9G0H1I2J3K4L5M6`
- **SHA256**: `FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321FEDCBA0987654321`

## firmware.bin

- **Size**: 1234567 bytes (1205.63 KB / 1.18 MB)
- **MD5**: `M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6`
- **SHA256**: `ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890`

## spiffs.bin

- **Size**: 1572864 bytes (1536.00 KB / 1.50 MB)
- **MD5**: `C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6`
- **SHA256**: `1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF`
```

---

## Method 3: Third-Party Tools (Optional)

### HashTab (Windows Explorer Integration)

1. Download: http://implbits.com/products/hashtab/
2. Install (adds "File Hashes" tab to Properties)
3. Right-click file → Properties → File Hashes tab
4. See MD5, SHA256, and other hashes

### certutil (Built-in Windows Tool)

```cmd
certutil -hashfile firmware.bin MD5
certutil -hashfile firmware.bin SHA256
```

**Note**: Output format is different (multi-line)

---

## Quick Reference Card

| Task | PowerShell Command |
|------|-------------------|
| MD5 hash | `Get-FileHash file.bin -Algorithm MD5` |
| SHA256 hash | `Get-FileHash file.bin -Algorithm SHA256` |
| File size | `(Get-Item file.bin).Length` |
| All files | `Get-ChildItem *.bin \| Get-FileHash -Algorithm MD5` |
| Copy hash only | `(Get-FileHash file.bin -Algorithm MD5).Hash` |

---

## Verifying Downloads (For Users)

Users can verify downloaded firmware:

```powershell
# Download firmware
# Then verify:
Get-FileHash -Path firmware.bin -Algorithm SHA256

# Compare output with published checksum in README.md
```

**If hashes match**: File is authentic and not corrupted ✅  
**If hashes differ**: File is corrupted or tampered ❌

---

## Why Checksums Matter

✅ **Integrity**: Verify files weren't corrupted during download  
✅ **Authenticity**: Confirm files are from official source  
✅ **Debugging**: Ensure correct firmware version is flashed  
✅ **Transparency**: Open source best practice

---

*Windows Checksums Guide by Processware - November 2025*  
*Part of the PowderSense Open Source Project*
