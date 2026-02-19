# 🛡️ Smart Security System (ESP32-CAM + Arduino Uno)

A complete, DIY smart home security system that uses a "Two-Brain" architecture. It combines the reliable 5V logic of an Arduino Uno for keypad access control with the powerful Wi-Fi and camera capabilities of an ESP32-CAM for remote monitoring. 

When armed, the system detects motion, triggers local deterrents (a buzzer siren and warning LEDs), and sends a photo of the intruder directly to your phone via Telegram.

## ✨ Features
* **Keypad Access Control:** Arm and disarm the system using a secure 4x3 matrix keypad.
* **Motion Detection:** HC-SR501 PIR sensor accurately detects human movement.
* **Local Deterrents:** Triggers an active buzzer and high-visibility LEDs immediately upon intrusion.
* **Remote Notification:** Captures a high-quality photo and sends it to a Telegram bot.
* **Censorship Bypass:** Includes a Cloudflare Worker reverse-proxy configuration to ensure Telegram notifications arrive even on restricted or filtered networks.
* **Hardware Protection:** Implements a custom voltage divider to safely bridge 5V and 3.3V logic between the two microcontrollers.

## 🧰 Hardware Requirements
* 1x ESP32-CAM (AI-Thinker module) + FTDI Programmer
* 1x Arduino Uno
* 1x HC-SR501 PIR Motion Sensor
* 1x 4x3 Matrix Keypad (HX_543)
* 1x Active Buzzer (TMB12A05)
* 2x Standard LEDs (Alert & Normal/Flash states)
* 3x 220Ω Resistors (to build the 5V to 3.3V voltage divider)
* Breadboards and Jumper Wires

## 🔌 Wiring Guide

### Arduino Uno (Access Control Panel)
| Component | Pin on Component | Pin on Arduino Uno |
| :--- | :--- | :--- |
| **Keypad** | Row 1 - 4 | Pins 9, 8, 7, 6 |
| **Keypad** | Col 1 - 3 | Pins 5, 4, 3 |
| **Signal Out** | To Voltage Divider | Pin 10 |
| **Ground** | Shared System Ground | GND |

### ESP32-CAM (Main Alarm System)
| Component | Pin on Component | Pin on ESP32-CAM |
| :--- | :--- | :--- |
| **Signal In** | From Voltage Divider | GPIO 2 |
| **PIR Sensor** | OUT | GPIO 13 |
| **Buzzer** | Positive (+) | GPIO 14 |
| **Alert LED** | Anode (+) | GPIO 12 |
| **Normal LED** | Anode (+) | GPIO 4 |

⚠️ **CRITICAL WARNING:** Do not connect Arduino Uno Pin 10 directly to ESP32-CAM GPIO 2. You must build a voltage divider using the three 220Ω resistors to step the 5V signal down to 3.3V to prevent frying the ESP32-CAM. Ensure both boards share a common GND wire.

## 💻 Software Installation

### 1. Arduino Uno Setup (`/Access_Control_Uno`)
1. Open the code in the Arduino IDE.
2. Go to **Sketch > Include Library > Manage Libraries**.
3. Search for and install the **Keypad** library (by Mark Stanley, Alexander Brevig).
4. Select the Arduino Uno board and upload the code.

### 2. ESP32-CAM Setup (`/Main_Alarm_ESP32CAM`)
1. Ensure the `esp32` board manager is installed in your Arduino IDE.
2. Select the **AI Thinker ESP32-CAM** or **ESP32 Wrover Module** board.
3. Update the following credentials in the code:
   * `ssid` & `password`: Your Wi-Fi network.
   * `BOT_TOKEN` & `CHAT_ID`: Your Telegram credentials.
   * `CF_WORKER`: Your Cloudflare Worker URL (omit the `https://`).
4. Ground `GPIO 0`, reset the board, and upload the code at 115200 baud.

### 3. Cloudflare Worker (Telegram Proxy)
If Telegram is blocked on your network, create a free Cloudflare Worker and paste the proxy script provided in the documentation. Point the ESP32-CAM HTTP request to your Worker's URL.

##  Contributors
* Mobina Heidari
* Atefeh Ghandehari
* Nika Ghaderi
