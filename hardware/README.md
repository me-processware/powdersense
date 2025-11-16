# PowderSense Hardware Documentation

**Designer**: Processware  
**License**: CERN-OHL-W  
**Version**: 1.0

---

## 📁 Directory Structure

```
hardware/
├── README.md              # This file
├── enclosure/             # 3D printable case files
│   ├── README.md         # Detailed printing and assembly instructions
│   ├── *.3mf             # Print-ready 3MF files
│   ├── *.stl             # Individual STL files (coming soon)
│   └── source/           # Original CAD files (Fusion 360, STEP)
├── schematics/           # Circuit diagrams and wiring (coming soon)
│   ├── main_schematic.pdf
│   └── wiring_diagram.pdf
└── pcb/                  # Custom PCB design files (future)
    ├── gerbers/
    └── kicad/
```

---

## 🔌 Hardware Overview

### Core Components

The PowderSense system is built around readily available development boards and modules:

**Main Controller**:
- **Waveshare ESP32-C6-LCD-1.47** development board
  - ESP32-C6 microcontroller with WiFi
  - Integrated 1.47" ST7789 TFT LCD (172x320 pixels)
  - USB-C programming and power
  - Compact form factor

**Precision Measurement**:
- **ADS1115** 16-bit ADC module (I2C interface)
  - 4 differential or 8 single-ended inputs
  - Programmable gain amplifier (PGA)
  - 860 samples/second max
  - I2C address: 0x48 (default)

**Position Sensor**:
- **10kΩ Linear Slide Potentiometer** (60mm travel)
  - Recommended: Bourns PTA6043-2015DPB103
  - Linearity: ±0.25%
  - Mechanical life: 1 million cycles
  - Alternatives: Any 60mm linear pot with similar specs

**Optional**:
- **WS2812B RGB LED** (on some board variants)
- **Touch interface** or **4x tactile buttons** (board dependent)

### Bill of Materials (BOM)

For a complete list of components with part numbers, suppliers, and pricing, see:
- [BOM.md](../docs/BOM.md) - Detailed bill of materials

---

## 🛠️ Assembly

### Quick Start

1. **Print Enclosure**: See [enclosure/README.md](enclosure/README.md)
2. **Gather Components**: Order parts from BOM
3. **Solder Connections**: Follow wiring diagram (coming soon)
4. **Install in Case**: Follow assembly instructions in enclosure README
5. **Upload Firmware**: See main project README
6. **Calibrate**: Follow calibration guide

### Detailed Instructions

For step-by-step assembly with photos:
- [Assembly Guide](../docs/ASSEMBLY.md) (coming soon)

---

## 📐 Schematics

### Wiring Overview

**ESP32-C6 to ADS1115 (I2C)**:
- SDA → GPIO4 (or board-specific I2C pin)
- SCL → GPIO5 (or board-specific I2C pin)
- VCC → 3.3V
- GND → GND

**ADS1115 to Potentiometer**:
- A0 → Potentiometer wiper (center pin)
- GND → Potentiometer ground (one end)
- 3.3V → Potentiometer VCC (other end)

**Power**:
- USB-C → ESP32-C6 board (5V input)
- Board regulates to 3.3V for peripherals

### Full Schematics

Detailed circuit diagrams will be added to `schematics/` directory:
- Main schematic (PDF)
- Wiring diagram with color codes
- PCB layout (if custom board developed)

---

## 🔧 Hardware Variants

### Board Configurations

The firmware supports two ESP32-C6 board variants:

**Variant 1: Touch Interface** (Default)
- Touch-enabled display
- No RGB LED
- Pins: SCLK=1, MOSI=2, DC=15, CS=14, RST=22, BL=23
- ADC on GPIO5

**Variant 2: RGB LED**
- 4x physical buttons
- WS2812B RGB LED on GPIO8
- Pins: SCLK=7, MOSI=6, DC=15, CS=14, RST=21, BL=22
- ADC on GPIO2

See `platformio.ini` for build configuration.

### Sensor Options

**Standard**: 10kΩ potentiometer with ADS1115 (16-bit)
- Resolution: 0.0146mm theoretical
- Accuracy: ±0.1mm after calibration

**Alternative**: Direct ESP32 ADC (12-bit)
- Lower cost (no ADS1115 needed)
- Reduced precision: ~0.15mm
- Sufficient for many applications

**Future**: Non-contact sensors
- Ultrasonic distance sensor
- Laser ToF sensor
- Magnetic linear encoder

---

## 🎨 Customization

### Mechanical Adaptations

The enclosure design can be modified for:
- Different display sizes
- Alternative mounting methods
- Integration with specific equipment
- Ruggedized versions for harsh environments

**Source files provided**:
- Fusion 360 (.f3d) in `enclosure/source/`
- STEP format (.step) for universal compatibility

### Electrical Modifications

**Possible enhancements**:
- Battery power (LiPo + charging circuit)
- External sensor inputs
- Relay outputs for automation
- RS-485 for industrial networks
- Ethernet module for wired connectivity

**Community contributions welcome!**

---

## 📏 Specifications

### Mechanical

| Parameter | Value |
|-----------|-------|
| Enclosure Dimensions | 120 x 80 x 45 mm (approx) |
| Weight (assembled) | ~200g |
| Mounting | Desktop, wall-mount, or custom |
| Ingress Protection | IP20 (indoor use) |

### Electrical

| Parameter | Value |
|-----------|-------|
| Input Voltage | 5V DC (USB-C) |
| Power Consumption | <1W typical, <2W max |
| Operating Temperature | 0-50°C |
| Storage Temperature | -20 to 70°C |
| Humidity | 20-80% RH (non-condensing) |

### Measurement

| Parameter | Value |
|-----------|-------|
| Range | 0-60mm (potentiometer dependent) |
| Resolution | 0.0146mm (16-bit ADC) |
| Accuracy | ±0.1mm (after calibration) |
| Repeatability | ±0.05mm |
| Sampling Rate | 10 Hz (configurable) |

---

## 🔐 License

All hardware designs in this directory are licensed under:

**CERN Open Hardware License Version 2 - Weakly Reciprocal (CERN-OHL-W)**

This means:
- ✅ You can use, modify, and sell products based on this design
- ✅ You can integrate into proprietary products
- ⚠️ Modifications to the hardware design must be shared under CERN-OHL-W
- ⚠️ Attribution to Processware required

Full license: [LICENSE-HARDWARE.txt](../LICENSE-HARDWARE.txt)

---

## 🤝 Contributing

### Hardware Contributions Welcome

We're looking for:
- **PCB designs**: Custom board layouts
- **Enclosure variants**: Different sizes, mounting options
- **Sensor adaptations**: Alternative measurement methods
- **Improvements**: Better cable management, cooling, etc.

### How to Contribute

1. Fork the repository
2. Create your hardware modification
3. Document thoroughly (schematics, BOM, photos)
4. Test and verify functionality
5. Submit pull request

See [CONTRIBUTING.md](../CONTRIBUTING.md) for detailed guidelines.

---

## 📞 Support

**Hardware questions?**

- GitHub Issues: Technical problems
- GitHub Discussions: Design questions
- Email: info@processware.nl

**Share your build!**

Post photos and modifications in GitHub Discussions or tag #PowderSense on social media.

---

## 🎯 Future Development

### Planned Hardware Improvements

**Short-term**:
- [ ] Custom PCB design (integrate ESP32 + ADS1115)
- [ ] Professional schematics in KiCad
- [ ] Multiple enclosure size options
- [ ] Improved cable management

**Long-term**:
- [ ] Battery-powered version
- [ ] Industrial-grade sensors
- [ ] Multi-sensor support
- [ ] Modular expansion system

See [ROADMAP.md](../docs/ROADMAP.md) for full development plan.

---

## 🙏 Acknowledgments

**Component Manufacturers**:
- Waveshare - ESP32-C6-LCD development board
- Texas Instruments - ADS1115 ADC
- Bourns - High-quality potentiometers

**Design Tools**:
- Autodesk Fusion 360 - Mechanical design
- KiCad - Electronics design (planned)

---

*Hardware designed by Processware - November 2025*  
*Part of the PowderSense Open Source Project*
