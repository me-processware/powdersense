# PowderSense - User Manual

**Product**: PowderSense Precision Measurement System  
**Author**: Processware  
**Version**: 2.0  
**Last Updated**: November 2025

---

## 📖 Table of Contents

1. [Introduction](#introduction)
2. [Quick Start Guide](#quick-start-guide)
3. [Device Overview](#device-overview)
4. [First Time Setup](#first-time-setup)
5. [Display Interface](#display-interface)
6. [Web Interface](#web-interface)
7. [Calibration](#calibration)
8. [Measurement Modes](#measurement-modes)
9. [Configuration Profiles](#configuration-profiles)
10. [Data Logging & Export](#data-logging--export)
11. [Settings & Preferences](#settings--preferences)
12. [Troubleshooting](#troubleshooting)
13. [Maintenance](#maintenance)
14. [Safety Information](#safety-information)

---

## 🎯 Introduction

### What is PowderSense?

PowderSense is a precision measurement system designed specifically for ammunition reloading. It provides accurate, real-time measurement of powder depth with sub-millimeter precision, converting measurements to grain weight for consistent ammunition loading.

### Key Features

**Precision Measurement**:
- 16-bit ADC (ADS1115) for high-resolution readings
- ±0.1mm accuracy after calibration
- 0.0146mm theoretical resolution
- 60mm measurement range

**Smart Automation**:
- Auto-measurement with configurable thresholds
- Visual and audible alarms for out-of-spec readings
- Automatic data logging
- Session tracking

**Connectivity**:
- WiFi-enabled web interface
- Real-time WebSocket updates
- Remote monitoring and control
- Data export capabilities

**User-Friendly**:
- 1.47" color TFT display
- Touch-enabled interface (on supported boards)
- Intuitive web dashboard
- Multiple configuration profiles

### Specifications

| Parameter | Value |
|-----------|-------|
| **Measurement Range** | 0-60mm |
| **Resolution** | 0.0146mm (theoretical) |
| **Accuracy** | ±0.1mm (after calibration) |
| **Repeatability** | ±0.05mm |
| **Sampling Rate** | 10 Hz (configurable) |
| **Display** | 1.47" TFT LCD (172x320 pixels) |
| **Connectivity** | WiFi 802.11 b/g/n |
| **Power** | 5V DC via USB-C |
| **Power Consumption** | <2W |

---

## 🚀 Quick Start Guide

### What You Need

- PowderSense device (assembled)
- USB-C cable
- 5V USB power adapter (phone charger works)
- Computer, tablet, or smartphone with WiFi
- Web browser (Chrome, Firefox, Safari, Edge)

### 5-Minute Setup

**Step 1: Power On** (30 seconds)
1. Connect USB-C cable to PowderSense
2. Connect other end to 5V power adapter
3. Device boots automatically
4. Display shows "PowderSense" splash screen

**Step 2: Connect to WiFi** (2 minutes)
1. Device starts in **AP Mode** (Access Point)
2. On your phone/computer, connect to WiFi network: **"PowderSense-XXXX"**
3. Password: **"powdersense"** (default)
4. Browser should open automatically (captive portal)
5. If not, navigate to: **http://192.168.4.1**
6. Enter your home WiFi credentials
7. Click "Save" - device will reboot

**Step 3: Access Web Interface** (1 minute)
1. Device connects to your WiFi network
2. Display shows IP address (e.g., 192.168.1.100)
3. On your computer, open browser
4. Navigate to the IP address shown on display
5. Web dashboard loads

**Step 4: Calibrate** (1.5 minutes)
1. In web interface, click **"Calibration"** tab
2. Follow 2-step calibration wizard:
   - **Step 1**: Set probe at 0mm (fully extended), click "Set Zero"
   - **Step 2**: Set probe at known depth, enter grain weight, click "Calibrate"
3. Calibration complete!

**Step 5: Start Measuring** (immediate)
1. Return to **"Dashboard"** tab
2. Place powder measure under probe
3. Adjust powder level
4. Reading updates in real-time
5. When stable, measurement is automatically recorded

**You're ready to go!** 🎉

---

## 🖥️ Device Overview

### Physical Components

**Front Panel**:
- **1.47" TFT Display**: Shows current measurement, status, and menus
- **Touch Buttons** (on display):
  - **CAL**: Enter calibration mode
  - **PROF**: Select configuration profile
  - **SET**: Access settings menu

**Top/Side**:
- **Measurement Probe**: Extends/retracts to measure powder depth
- **Spring Return**: Automatically returns probe to zero position

**Back Panel**:
- **USB-C Port**: Power input and firmware updates
- **Reset Button**: Hardware reset (if accessible on your model)

**Internal** (visible when opened):
- **ESP32-C6 Board**: Main controller with WiFi
- **ADS1115 Module**: 16-bit ADC for precision measurement
- **Potentiometer**: Linear position sensor (60mm travel)

### Display Screens

PowderSense has three main display screens:

#### 1. Measurement Screen (Default)

**Main Display**:
```
┌─────────────────────────┐
│  45.234 grain          │ ← Current weight (large)
│                         │
│  Target: 45.0 grain    │ ← Target from profile
│  Diff: +0.234 grain    │ ← Difference
│                         │
│  ● Session: 12         │ ← Measurements this session
│  ● Total: 347          │ ← Total measurements
│                         │
│  [CAL] [PROF] [SET]    │ ← Touch buttons
└─────────────────────────┘
```

**Status Indicators**:
- **Green**: Measurement within tolerance
- **Yellow**: Approaching threshold
- **Red**: Out of specification (alarm active)

#### 2. Calibration Screen

```
┌─────────────────────────┐
│ --- CALIBRATION ---     │
│                         │
│ Step 1: Set Zero        │
│ Current: 1234 ADC       │
│                         │
│ Follow instructions     │
│ on web interface        │
│                         │
│ [BACK]                  │
└─────────────────────────┘
```

**Note**: Calibration is primarily controlled via web interface for accuracy.

#### 3. AP Mode Screen

```
┌─────────────────────────┐
│ --- AP MODE ACTIVE ---  │
│                         │
│ Connect to:             │
│ PowderSense-XXXX        │
│                         │
│ Password:               │
│ powdersense             │
│                         │
│ Then visit:             │
│ http://192.168.4.1      │
└─────────────────────────┘
```

Displayed when device is in Access Point mode (no WiFi configured).

---

## 🔧 First Time Setup

### Initial Power-On

**What Happens**:
1. Device performs self-test
2. Initializes display (shows splash screen)
3. Checks for WiFi credentials in memory
4. If no WiFi configured → starts **AP Mode**
5. If WiFi configured → attempts to connect

### AP Mode Configuration

**When device starts in AP Mode**:

1. **Connect to PowderSense WiFi**:
   - SSID: `PowderSense-XXXX` (XXXX = last 4 digits of MAC address)
   - Password: `powdersense`

2. **Access Configuration Page**:
   - Most devices: Captive portal opens automatically
   - Manual: Navigate to `http://192.168.4.1`

3. **Configure WiFi**:
   - Select your WiFi network from dropdown (or enter manually)
   - Enter WiFi password
   - Click **"Save Configuration"**

4. **Device Reboots**:
   - Connects to your WiFi network
   - Display shows assigned IP address
   - AP mode automatically disabled

### Connecting to Web Interface

**Once connected to WiFi**:

1. **Find IP Address**:
   - Displayed on device screen
   - Check your router's DHCP client list
   - Use network scanner app (e.g., Fing)

2. **Open Web Browser**:
   - Enter IP address in address bar
   - Example: `http://192.168.1.100`
   - Press Enter

3. **Web Dashboard Loads**:
   - Main dashboard appears
   - Real-time measurement display
   - Navigation tabs at top

**Bookmark the page** for easy access!

---

## 📱 Web Interface

### Dashboard Overview

The web interface is divided into several tabs:

```
┌─────────────────────────────────────────────────────┐
│  PowderSense 2.0 Dashboard                          │
│  [Dashboard] [Calibration] [Profiles] [Settings]    │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────────┐  ┌──────────────────┐       │
│  │  Current Weight  │  │  Session Stats   │       │
│  │   45.234 grain   │  │  Count: 12       │       │
│  │                  │  │  Avg: 45.1       │       │
│  └──────────────────┘  └──────────────────┘       │
│                                                     │
│  ┌──────────────────────────────────────────┐     │
│  │  Measurement Chart                       │     │
│  │  [Real-time graph of measurements]       │     │
│  └──────────────────────────────────────────┘     │
│                                                     │
│  [Start Session] [Export Data] [Zero]              │
└─────────────────────────────────────────────────────┘
```

### Dashboard Tab

**Main Display**:
- **Current Weight**: Large display of current measurement (updates 10x/second)
- **Target Weight**: From selected profile (if any)
- **Difference**: Shows deviation from target
- **Status Indicator**: Color-coded (green/yellow/red)

**Session Statistics**:
- **Count**: Measurements in current session
- **Average**: Average weight this session
- **Min/Max**: Range of measurements
- **Std Dev**: Standard deviation (consistency indicator)

**Measurement Chart**:
- Real-time line graph
- Last 20-100 measurements (configurable)
- Visual representation of consistency
- Hover for exact values

**Action Buttons**:
- **Start Session**: Begin new measurement session
- **End Session**: Finalize current session (saves to log)
- **Zero**: Reset zero point (quick re-zero)
- **Measure**: Force manual measurement
- **Export Data**: Download measurement history (CSV)

### Calibration Tab

**Two-Step Calibration Wizard**:

```
┌─────────────────────────────────────────────────────┐
│  Calibration Wizard                                 │
├─────────────────────────────────────────────────────┤
│                                                     │
│  Step 1: Set Zero Point                            │
│  ┌───────────────────────────────────────────┐    │
│  │ 1. Fully extend probe (0mm position)      │    │
│  │ 2. Ensure no load on probe                │    │
│  │ 3. Click "Set Zero" when ready            │    │
│  │                                            │    │
│  │ Current ADC: 1234                          │    │
│  │                                            │    │
│  │ [Set Zero]                                 │    │
│  └───────────────────────────────────────────┘    │
│                                                     │
│  Step 2: Set Known Reference                       │
│  ┌───────────────────────────────────────────┐    │
│  │ 1. Place known weight (e.g., 45.0 grains) │    │
│  │ 2. Adjust probe to measure depth          │    │
│  │ 3. Enter known weight below                │    │
│  │ 4. Click "Calibrate" when stable           │    │
│  │                                            │    │
│  │ Known Weight: [45.0] grains                │    │
│  │ Current Depth: 12.34 mm                    │    │
│  │                                            │    │
│  │ [Calibrate]                                │    │
│  └───────────────────────────────────────────┘    │
│                                                     │
│  [Cancel] [Help]                                    │
└─────────────────────────────────────────────────────┘
```

**Calibration Process**:

1. **Step 1: Zero Point**:
   - Fully extend probe (no compression)
   - Click "Set Zero"
   - Device records baseline ADC value
   - Status: "Zero set successfully"

2. **Step 2: Known Reference**:
   - Place known powder charge (weigh on scale first)
   - Adjust probe to measure depth
   - Enter known weight in grains
   - Wait for reading to stabilize
   - Click "Calibrate"
   - Device calculates grains/mm factor

3. **Verification**:
   - Test with known weights
   - Check accuracy
   - Re-calibrate if needed

**Tips for Accurate Calibration**:
- Use a precision scale to weigh reference charge
- Allow readings to stabilize (wait 2-3 seconds)
- Use mid-range weight (not too light or heavy)
- Calibrate at operating temperature
- Re-calibrate if changing powder types significantly

### Profiles Tab

**Configuration Profile Management**:

```
┌─────────────────────────────────────────────────────┐
│  Configuration Profiles                             │
├─────────────────────────────────────────────────────┤
│                                                     │
│  Active Profile: .308 Win - 168gr SMK              │
│                                                     │
│  ┌─────────────────────────────────────────────┐  │
│  │ Profile Name    Caliber  Bullet  Target     │  │
│  ├─────────────────────────────────────────────┤  │
│  │ ● .308 Win      .308     168gr   44.5 gr    │  │
│  │   9mm Luger     9mm      115gr   5.2 gr     │  │
│  │   .223 Rem      .223     55gr    25.0 gr    │  │
│  │   .45 ACP       .45      230gr   6.8 gr     │  │
│  └─────────────────────────────────────────────┘  │
│                                                     │
│  [New Profile] [Edit] [Delete] [Duplicate]         │
│                                                     │
│  Profile Details:                                   │
│  ┌───────────────────────────────────────────┐    │
│  │ Name: .308 Win - 168gr SMK                │    │
│  │ Caliber: .308 Winchester                   │    │
│  │ Bullet Weight: 168 grains                  │    │
│  │ Powder: Varget                             │    │
│  │ Target Charge: 44.5 grains                 │    │
│  │                                            │    │
│  │ Calibration Status: ✓ Calibrated          │    │
│  │ Last Calibrated: 2025-11-10 14:30          │    │
│  │                                            │    │
│  │ Alarm Thresholds:                          │    │
│  │ Low: 44.0 grains  High: 45.0 grains        │    │
│  │                                            │    │
│  │ [Save] [Cancel]                            │    │
│  └───────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────┘
```

**Creating a New Profile**:

1. Click **"New Profile"**
2. Fill in details:
   - **Name**: Descriptive name (e.g., ".308 Win - 168gr SMK")
   - **Caliber**: Cartridge caliber (e.g., ".308 Winchester")
   - **Bullet Weight**: Projectile weight in grains
   - **Powder Name**: Powder type (e.g., "Varget")
   - **Target Charge**: Desired powder weight in grains
3. Set alarm thresholds:
   - **Low Threshold**: Minimum acceptable weight
   - **High Threshold**: Maximum acceptable weight
4. Click **"Save"**
5. **Calibrate** the profile (each profile has its own calibration)

**Using Profiles**:
- Click profile name to activate
- Active profile shown with ● indicator
- Target weight displayed on dashboard
- Alarms use profile thresholds
- Measurements tagged with profile info

**Profile Benefits**:
- Quick switching between loads
- Separate calibration for each powder type
- Automatic alarm configuration
- Historical tracking per profile
- Easy load development

### Settings Tab

**System Configuration**:

```
┌─────────────────────────────────────────────────────┐
│  Settings                                           │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌─ WiFi Settings ─────────────────────────────┐  │
│  │ SSID: MyHomeWiFi                            │  │
│  │ Signal: ████░ (Strong)                      │  │
│  │ IP Address: 192.168.1.100                   │  │
│  │ [Change WiFi] [Reset to AP Mode]            │  │
│  └─────────────────────────────────────────────┘  │
│                                                     │
│  ┌─ Measurement Settings ──────────────────────┐  │
│  │ Auto-Measure: ☑ Enabled                     │  │
│  │ Stable Duration: [750] ms                   │  │
│  │ Tolerance: [±0.1] grains                    │  │
│  │ Cooldown: [10] seconds                      │  │
│  │ Display Update Rate: [100] ms               │  │
│  └─────────────────────────────────────────────┘  │
│                                                     │
│  ┌─ Alarm Settings ────────────────────────────┐  │
│  │ Alarms: ☑ Enabled                           │  │
│  │ Visual Alarm: ☑ Red display flash           │  │
│  │ Audible Alarm: ☐ Beep (if supported)        │  │
│  └─────────────────────────────────────────────┘  │
│                                                     │
│  ┌─ Data Logging ──────────────────────────────┐  │
│  │ Auto-Log Measurements: ☑ Enabled            │  │
│  │ Max History: [100] measurements             │  │
│  │ [Export All Data] [Clear History]           │  │
│  └─────────────────────────────────────────────┘  │
│                                                     │
│  ┌─ System ────────────────────────────────────┐  │
│  │ Firmware Version: 2.0.1                     │  │
│  │ Uptime: 2h 34m 12s                          │  │
│  │ [Firmware Update] [Restart] [Factory Reset] │  │
│  └─────────────────────────────────────────────┘  │
│                                                     │
│  [Save Settings]                                    │
└─────────────────────────────────────────────────────┘
```

**Key Settings**:

**Auto-Measure**:
- **Enabled**: Automatically records stable measurements
- **Stable Duration**: How long reading must be stable (default: 750ms)
- **Tolerance**: Acceptable variation (±0.1 grains)
- **Cooldown**: Delay between auto-measurements (10 seconds)

**Display**:
- **Update Rate**: How often display refreshes (100ms = 10 times/second)
- **Brightness**: Backlight level (if adjustable)

**Alarms**:
- **Enable/Disable**: Master alarm switch
- **Thresholds**: Set per profile or globally
- **Visual**: Red display flash when out of spec
- **Audible**: Beep (if hardware supports)

**Data**:
- **Auto-Log**: Automatically save measurements
- **Max History**: Number of measurements to retain
- **Export**: Download as CSV file
- **Clear**: Delete all measurement history

---

## 🎯 Calibration

### Why Calibrate?

Calibration is **essential** for accurate measurements. It compensates for:
- Variations in potentiometer characteristics
- Mechanical tolerances in assembly
- Different powder densities
- Temperature effects

**Each configuration profile should be calibrated separately** for maximum accuracy.

### When to Calibrate

**Required**:
- First time setup
- After creating new profile
- When switching powder types
- After hardware changes

**Recommended**:
- Weekly if used frequently
- Monthly for occasional use
- When measurements seem inconsistent
- After significant temperature changes

### Calibration Procedure

**Preparation** (5 minutes):
1. Warm up device (power on for 2-3 minutes)
2. Prepare known reference weight:
   - Weigh powder charge on precision scale
   - Use mid-range weight (e.g., 40-50 grains for rifle)
   - Record exact weight
3. Have powder measure ready
4. Ensure stable environment (no vibration)

**Step-by-Step** (2 minutes):

**Step 1: Set Zero Point**
1. Navigate to **Calibration** tab in web interface
2. Fully extend probe (no compression)
3. Ensure no load on probe
4. Wait for ADC reading to stabilize
5. Click **"Set Zero"** button
6. Confirmation: "Zero point set successfully"

**Step 2: Set Known Reference**
1. Place your pre-weighed powder charge in measure
2. Position probe to measure depth
3. Wait for reading to stabilize (2-3 seconds)
4. Enter exact weight in **"Known Weight"** field
   - Example: `45.0` grains
5. Verify current depth reading is stable
6. Click **"Calibrate"** button
7. Device calculates conversion factor
8. Confirmation: "Calibration complete"

**Verification** (1 minute):
1. Remove and replace powder charge
2. Check reading matches known weight
3. Try different weights if available
4. Acceptable accuracy: ±0.1 grains

**Troubleshooting Calibration**:
- **Reading unstable**: Wait longer, reduce vibration
- **Inaccurate after calibration**: Re-calibrate with different reference weight
- **Drifting over time**: Check for mechanical issues, re-calibrate

### Advanced: Multi-Point Calibration

For maximum accuracy across full range:

1. Calibrate at low weight (e.g., 20 grains)
2. Note accuracy at mid-range (e.g., 45 grains)
3. Test at high weight (e.g., 60 grains)
4. If non-linear, use mid-range for calibration
5. Accept slight error at extremes, or create separate profiles

---

## 📊 Measurement Modes

### Manual Measurement

**How it works**:
- You control when measurements are recorded
- Adjust powder level
- Click **"Measure"** button when ready
- Measurement saved to history

**Best for**:
- Load development (testing different charges)
- Verification measurements
- When you want full control
- Learning the system

**Procedure**:
1. Ensure profile is selected (optional)
2. Place powder in measure
3. Adjust to desired level
4. Wait for reading to stabilize
5. Click **"Measure"** button
6. Measurement recorded and displayed

### Auto-Measurement Mode

**How it works**:
- System automatically detects stable measurements
- When reading is stable within tolerance for set duration, measurement is recorded
- Cooldown period prevents duplicate measurements
- Ideal for production reloading

**Best for**:
- High-volume reloading
- Consistent workflow
- Hands-free operation
- Reducing user error

**Configuration**:
- **Stable Duration**: 750ms (default) - How long reading must be stable
- **Tolerance**: ±0.1 grains - Acceptable variation during stable period
- **Cooldown**: 10 seconds - Delay before next auto-measurement

**Workflow**:
1. Enable auto-measure in settings
2. Select profile with target weight
3. Place powder in measure
4. Adjust to target level
5. **Wait** - system detects stability
6. **Green flash** - measurement recorded automatically
7. Remove/dispense powder
8. Repeat for next round

**Status Indicators**:
- **White**: Normal measurement
- **Yellow**: Approaching target (within 0.5 grains)
- **Green**: On target (within tolerance) - stable
- **Red**: Out of specification (alarm active)

### Session Tracking

**What is a Session?**
- A session is a group of related measurements
- Typically one reloading session (e.g., loading 50 rounds)
- Tracks statistics for that batch

**Starting a Session**:
1. Click **"Start Session"** button
2. (Optional) Enter session notes
3. Begin measurements
4. Session counter increments with each measurement

**During Session**:
- **Count**: Number of measurements
- **Average**: Mean weight
- **Min/Max**: Range
- **Std Dev**: Consistency indicator
- **Chart**: Visual representation

**Ending a Session**:
1. Click **"End Session"** button
2. Session saved to log with:
   - Start/end time
   - Bullet count
   - Total weight
   - Statistics
   - Profile used
3. Ready to start new session

**Session Log**:
- View past sessions in **History** tab
- Export session data
- Compare sessions over time
- Track powder lot variations

---

## 🗂️ Configuration Profiles

### Profile Concept

Each profile represents a specific load recipe:
- Caliber (e.g., .308 Winchester)
- Bullet weight (e.g., 168 grains)
- Powder type (e.g., Varget)
- Target charge weight
- Alarm thresholds
- **Independent calibration**

### Creating Profiles

**Example: .308 Winchester Load**

1. Navigate to **Profiles** tab
2. Click **"New Profile"**
3. Fill in details:
   ```
   Name: .308 Win - 168gr SMK
   Caliber: .308 Winchester
   Bullet Weight: 168 gr
   Powder: Varget
   Target Charge: 44.5 gr
   Low Threshold: 44.0 gr
   High Threshold: 45.0 gr
   ```
4. Click **"Save"**
5. Profile created but **not calibrated**

**Calibrating the Profile**:
1. Select the profile (click to activate)
2. Go to **Calibration** tab
3. Follow calibration procedure
4. Calibration saved to this profile
5. Profile now ready to use

### Using Profiles

**Switching Profiles**:
1. Go to **Profiles** tab
2. Click profile name to activate
3. Active profile marked with ●
4. Dashboard updates with profile info

**What Changes**:
- Target weight displayed
- Alarm thresholds applied
- Measurements tagged with profile
- Calibration specific to profile

**Profile Management**:
- **Edit**: Modify profile details
- **Duplicate**: Copy profile as template
- **Delete**: Remove profile (confirmation required)
- **Export**: Save profile to file
- **Import**: Load profile from file

### Best Practices

**Naming Convention**:
- Include caliber, bullet weight, powder
- Example: `.308 Win - 168gr SMK - Varget`
- Makes selection easy

**Thresholds**:
- Set based on load data tolerances
- Typically ±0.5 grains for rifle
- Tighter for precision loads (±0.2 grains)
- Wider for pistol (±0.3 grains)

**Calibration**:
- Calibrate each profile separately
- Different powders have different densities
- Ensures maximum accuracy
- Re-calibrate if powder lot changes significantly

**Organization**:
- Group by caliber
- Archive old loads
- Document load development in profile notes

---

## 📈 Data Logging & Export

### Automatic Logging

**What is Logged**:
- Timestamp (date and time)
- Measurement value (grains)
- Active profile (if any)
- Session ID
- ADC raw value (for diagnostics)

**Storage**:
- Measurements stored in device memory
- Up to 100 measurements (configurable)
- Oldest measurements overwritten when full
- Persistent across power cycles

### Viewing History

**In Web Interface**:
1. Navigate to **History** tab
2. View table of measurements:
   ```
   Time        Weight    Profile           Session
   14:32:15    44.523    .308 Win - 168gr  Session 3
   14:32:45    44.487    .308 Win - 168gr  Session 3
   14:33:12    44.501    .308 Win - 168gr  Session 3
   ```
3. Sort by column (click header)
4. Filter by profile or session

**Statistics**:
- Average weight
- Standard deviation
- Min/Max values
- Count
- Trend analysis

### Exporting Data

**CSV Export**:
1. Click **"Export Data"** button
2. Choose export options:
   - All measurements
   - Current session only
   - Date range
   - Specific profile
3. Click **"Download"**
4. File saved: `powdersense_data_YYYYMMDD_HHMMSS.csv`

**CSV Format**:
```csv
Timestamp,Weight_Grains,Profile_Name,Caliber,Bullet_Weight,Powder,Target,Session_ID,ADC_Value
2025-11-10 14:32:15,44.523,.308 Win - 168gr,.308 Winchester,168,Varget,44.5,3,15234
2025-11-10 14:32:45,44.487,.308 Win - 168gr,.308 Winchester,168,Varget,44.5,3,15198
```

**Using Exported Data**:
- Open in Excel, Google Sheets, or LibreOffice
- Create charts and graphs
- Statistical analysis
- Load development documentation
- Share with reloading community

### Data Management

**Clearing History**:
1. Settings → Data Logging
2. Click **"Clear History"**
3. Confirm action
4. All measurements deleted (irreversible!)

**Backup**:
- Export data regularly
- Save CSV files to cloud storage
- Keep records for load development

---

## ⚙️ Settings & Preferences

### WiFi Settings

**Changing WiFi Network**:
1. Settings → WiFi Settings
2. Click **"Change WiFi"**
3. Enter new SSID and password
4. Click **"Save"**
5. Device reboots and connects

**Reset to AP Mode**:
1. Settings → WiFi Settings
2. Click **"Reset to AP Mode"**
3. Confirm action
4. Device reboots in AP mode
5. Reconfigure WiFi as in first setup

### Measurement Settings

**Auto-Measure Configuration**:
- **Enable/Disable**: Toggle auto-measurement
- **Stable Duration**: 750ms recommended (faster response) to 2000ms (more stable)
- **Tolerance**: ±0.1 grains typical, ±0.05 for precision
- **Cooldown**: 10 seconds prevents accidental duplicates

**Display Settings**:
- **Update Rate**: 100ms (10 Hz) for smooth updates
- **Brightness**: Adjust backlight (if supported)
- **Rotation**: Change display orientation (if needed)

### Alarm Configuration

**Alarm Settings**:
- **Master Enable**: Turn all alarms on/off
- **Visual Alarm**: Red display flash when out of spec
- **Audible Alarm**: Beep (if hardware supports)
- **Threshold Mode**:
  - **Profile-based**: Use thresholds from active profile
  - **Global**: Use same thresholds for all measurements

**Testing Alarms**:
1. Set very narrow thresholds (e.g., 40.0-40.5 grains)
2. Measure outside range
3. Verify alarm activates
4. Reset thresholds to normal

### System Settings

**Firmware Update**:
1. Settings → System
2. Click **"Firmware Update"**
3. Select `.bin` file
4. Click **"Upload"**
5. Wait for update (do not power off!)
6. Device reboots with new firmware

**Factory Reset**:
1. Settings → System
2. Click **"Factory Reset"**
3. **Warning**: Deletes all data!
4. Confirm action
5. Device resets to defaults
6. Reconfigure as new device

**Restart**:
1. Settings → System
2. Click **"Restart"**
3. Device reboots (settings preserved)

---

## 🔧 Troubleshooting

### Display Issues

**Display is Blank**:
- Check power connection (USB-C cable)
- Try different power adapter (needs 5V, >500mA)
- Check for loose connections inside (if comfortable opening)
- Hardware reset (unplug for 30 seconds)

**Display Shows Garbage/Artifacts**:
- Firmware issue - try re-uploading firmware
- Display ribbon cable loose - check connection
- Hardware defect - contact support

**Display Too Dim/Bright**:
- Adjust brightness in settings (if available)
- Check backlight connection
- May be hardware limitation

### Measurement Issues

**Readings are Erratic/Jumping**:
- Check for vibration (place on stable surface)
- Potentiometer connection loose - check wiring
- ADC issue - verify ADS1115 is detected
- Electrical noise - check grounding
- Re-calibrate

**Readings Don't Change**:
- Potentiometer not connected - check wiring
- Rod not moving - check mechanical assembly
- ADS1115 not responding - check I2C connection
- Firmware issue - check serial console for errors

**Readings are Inaccurate**:
- **Not calibrated** - perform calibration
- **Wrong calibration** - re-calibrate with accurate reference
- **Mechanical issue** - check for binding, friction
- **Temperature drift** - allow warmup time
- **Powder density changed** - re-calibrate for new powder

**Auto-Measure Not Working**:
- Check if enabled in settings
- Readings may not be stable enough - increase stable duration
- Tolerance too tight - increase tolerance
- Cooldown period active - wait 10 seconds after last measurement

### WiFi Issues

**Can't Connect to PowderSense AP**:
- Verify SSID: `PowderSense-XXXX`
- Verify password: `powdersense`
- Move closer to device
- Restart device
- Check if your device supports 2.4GHz WiFi (5GHz not supported)

**Device Won't Connect to Home WiFi**:
- Verify SSID and password are correct
- Check WiFi signal strength at device location
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Check router settings (MAC filtering, etc.)
- Reset to AP mode and reconfigure

**Can't Access Web Interface**:
- Verify IP address on display
- Ensure computer is on same network
- Try `http://` prefix (not `https://`)
- Clear browser cache
- Try different browser
- Ping IP address to verify connectivity

**WebSocket Disconnects Frequently**:
- WiFi signal weak - move device closer to router
- Router overloaded - reduce network traffic
- Interference - change WiFi channel
- Firmware issue - update to latest version

### Calibration Issues

**Can't Complete Calibration**:
- Readings not stable - reduce vibration
- Reference weight unknown - weigh accurately first
- Mechanical binding - check probe movement
- Follow exact procedure in calibration section

**Calibration Not Accurate**:
- Reference weight inaccurate - verify with precision scale
- Used wrong weight value - double-check entry
- Powder density different - re-calibrate for each powder type
- Temperature changed - allow warmup, re-calibrate

### Data Issues

**Measurements Not Logging**:
- Auto-log disabled - enable in settings
- Memory full - export and clear history
- Firmware bug - restart device

**Can't Export Data**:
- Browser blocking download - check permissions
- No measurements to export
- Try different browser

---

## 🛠️ Maintenance

### Regular Maintenance

**Daily** (if used frequently):
- Wipe display with soft cloth
- Check probe for powder residue
- Verify smooth probe movement

**Weekly**:
- Clean probe and housing
- Check for loose screws
- Verify calibration accuracy

**Monthly**:
- Deep clean (compressed air)
- Check all connections (if comfortable opening)
- Backup measurement data
- Update firmware if available

### Cleaning

**Display**:
- Use soft, lint-free cloth
- Slightly damp if needed (water only)
- No harsh chemicals or solvents
- Avoid scratching screen

**Probe**:
- Wipe with dry cloth
- Remove powder residue
- Check for burrs or damage
- Lubricate bearings (dry lubricant only)

**Housing**:
- Wipe exterior with damp cloth
- Compressed air for vents
- Don't submerge in water
- Keep away from solvents

### Storage

**Short-term** (daily):
- Power off when not in use
- Cover to prevent dust
- Store in dry location

**Long-term** (weeks/months):
- Export and backup all data
- Power off and unplug
- Store in protective case
- Avoid extreme temperatures
- Remove from humid environments

### Firmware Updates

**Checking for Updates**:
1. Visit project GitHub page
2. Check releases section
3. Read changelog
4. Download `.bin` file if update available

**Updating Firmware**:
1. Settings → System → Firmware Update
2. Select downloaded `.bin` file
3. Click **"Upload"**
4. **Do not power off during update!**
5. Wait for completion (1-2 minutes)
6. Device reboots automatically
7. Verify new version in settings

**If Update Fails**:
- Try again
- Use USB serial update method (see advanced documentation)
- Contact support if persistent

---

## ⚠️ Safety Information

### General Safety

**Electrical Safety**:
- Use only 5V DC power supply
- Don't use damaged USB cables
- Keep away from water and liquids
- Don't open device while powered
- Unplug during thunderstorms

**Reloading Safety**:
- **This device is a tool, not a safety device**
- Always verify charges with a precision scale
- Never exceed maximum load data
- Follow published reloading manuals
- Start low and work up carefully
- Inspect every round visually

### Important Warnings

**⚠️ MEASUREMENT ACCURACY**:
- Device provides **approximate** measurements
- **Always verify** critical charges with a scale
- Calibration can drift over time
- Mechanical wear affects accuracy
- Environmental factors influence readings

**⚠️ NOT A SAFETY DEVICE**:
- Do not rely solely on this device for safety
- Use proper load data from reputable sources
- Understand pressure signs
- Start with minimum loads
- Work up gradually

**⚠️ USER RESPONSIBILITY**:
- You are responsible for safe reloading practices
- Verify all measurements independently
- Maintain device properly
- Re-calibrate regularly
- Use common sense

### Liability Disclaimer

**Use at your own risk**. The manufacturer and developer are not responsible for:
- Inaccurate measurements
- Equipment failure
- Improper use
- Injury or property damage
- Ammunition malfunctions

**Reloading ammunition is inherently dangerous**. Proper training, equipment, and procedures are essential. This device is a convenience tool, not a replacement for proper reloading practices.

---

## 📞 Support & Resources

### Getting Help

**Documentation**:
- User Manual (this document)
- Assembly Guide
- Calibration Guide
- Troubleshooting Guide

**Online Resources**:
- GitHub Repository: https://github.com/me-processware/powdersense
- GitHub Issues: Report bugs and problems
- GitHub Discussions: Ask questions, share tips

**Contact**:
- Email: info@processware.nl
- Response time: 24-48 hours

### Community

**Share Your Experience**:
- Post build photos
- Share calibration tips
- Document load development
- Help other users
- Contribute improvements

**Contributing**:
- Report bugs on GitHub
- Suggest features
- Submit code improvements
- Improve documentation
- Share custom profiles

---

## 📚 Appendix

### Glossary

**ADC**: Analog-to-Digital Converter - converts analog voltage to digital number

**Calibration**: Process of establishing relationship between sensor reading and actual measurement

**Grain**: Unit of weight (1 grain = 64.79891 milligrams)

**Profile**: Configuration preset for specific load recipe

**Session**: Group of related measurements (e.g., one reloading batch)

**WebSocket**: Real-time communication protocol for instant updates

### Specifications Summary

| Parameter | Value |
|-----------|-------|
| Measurement Range | 0-60mm |
| Resolution | 0.0146mm |
| Accuracy | ±0.1mm |
| ADC | 16-bit (ADS1115) |
| Display | 1.47" TFT (172x320) |
| WiFi | 802.11 b/g/n (2.4GHz) |
| Power | 5V DC, <2W |
| Dimensions | ~120 × 80 × 45 mm |
| Weight | ~200g |

### Keyboard Shortcuts (Web Interface)

| Key | Action |
|-----|--------|
| `Space` | Measure |
| `Z` | Zero |
| `C` | Calibration |
| `P` | Profiles |
| `S` | Settings |
| `E` | Export |

### Quick Reference Card

**Power On**:
1. Connect USB-C
2. Wait for boot
3. Note IP address on display

**Connect**:
1. Open browser
2. Enter IP address
3. Bookmark page

**Calibrate**:
1. Calibration tab
2. Set zero (probe extended)
3. Set known reference
4. Verify accuracy

**Measure**:
1. Select profile
2. Place powder
3. Wait for stable reading
4. Auto-records or click Measure

**Export**:
1. History tab
2. Export Data button
3. Save CSV file

---

**End of User Manual**

*For additional support, visit the GitHub repository or contact info@processware.nl*

---

*User Manual by Processware - November 2025*  
*Part of the PowderSense Open Source Project*  
*Version 2.0*
