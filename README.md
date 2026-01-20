# MCU Transport Display: Coconut Groove Studio

A high-visibility hardware status display for **PreSonus Studio One**. This project uses an **Adafruit Feather M0 WiFi** to emulate a **Mackie Control (MCU)** surface, intercepting transport commands via **rtpMIDI** to drive an 8x32 NeoPixel matrix and internal tally lights.

---

## 1. System Logic & Algorithm
The firmware is designed with a priority-state hierarchy to ensure the display is always accurate and responsive:

* **Stage 1: Connection Health:** The system verifies Wi-Fi status via the ATWINC1500 module. If disconnected, it triggers "Recovery Mode," displaying a pulsing `No Wifi` message while attempting to reconnect.
* **Stage 2: Static Networking:** Uses a hardcoded Static IP (**192.168.1.202**) to ensure rtpMIDI sessions survive power outages without DHCP IP changes.
* **Stage 3: Session Monitoring:** It monitors rtpMIDI activity. If no MIDI data is received for 10 seconds, the display blanks automatically to prevent LED burn-in.
* **Stage 4: MCU MIDI Parsing:** The Feather listens for specific MIDI Note-On triggers:
    * **Note 0x5F (95):** Set State to **RECORD** (Red Pulse + PPM Meters).
    * **Note 0x5E (94):** Set State to **PLAYBACK** (Green Scrolling Text).
    * **Note 0x5D (93):** Set State to **IDLE/STOP** (Blue Scrolling "Coconut Groove").
* **Stage 5: Rendering Engine:** Translates (X, Y) coordinates to a **Serpentine Layout** for the 8x32 matrix and drives a secondary internal strip for studio-wide tally.

---

## 2. Construction Manual

### Hardware Wiring
1.  **Matrix (Outside):** Connect Data Input (DI) to **Pin 6**.
2.  **Status Strip (Inside):** Connect Data Input (DI) to **Pin 11**.
3.  **Power:** Connect a dedicated **5V 3A power supply** to the LED rails. **Important:** Ensure the Feather GND and LED PSU GND are tied together (Common Ground).

### Software & Code
1.  **The Sketch:** Download the [studio_sign.ino](./studio_sign.ino) file.
2.  **Configuration:** Update the `ssid`, `pass`, `local_IP`, and `studioOneIP` variables at the top of the sketch to match your studio network.

### Programming the Feather M0
1.  **Install Arduino IDE:** [Download here](https://www.arduino.cc/en/software).
2.  **Add Adafruit Support:** * Open **File > Preferences**.
    * Add to Board Manager URLs: `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
    * Go to **Tools > Board > Boards Manager**, search for **Adafruit SAMD**, and install.
3.  **Install Required Libraries:** * **WiFi101** (Required for Feather M0 WiFi)
    * **Adafruit NeoPixel**
    * **AppleMIDI**
4.  **Connect & Upload:** Select **Adafruit Feather M0** under Tools > Board and click **Upload**.

---

## 3. DAW Connectivity

#### **Network Configuration**
This sketch uses **Port 5006** to avoid conflicts with default rtpMIDI sessions.

#### **Windows Users (rtpMIDI)**
1.  Install **rtpMIDI** (by Tobias Erichsen).
2.  Add a **Manual Participant**: `192.168.1.202:5006`.
3.  Click **Connect**.

#### **Studio One Setup**
In **Options > External Devices**, add a **Mackie Control**. Set the Receive/Send ports to your newly created Network Session.

---

## 4. Bill of Materials (BOM)

| Component | Specification | Quantity | Purpose |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | Adafruit Feather M0 WiFi | 1 | System Brain (ATSAMD21) |
| **LED Matrix** | WS2812B 8x32 Flexible | 1 | External Signage |
| **Internal LEDs** | WS2812B LED Strip | ~24 LEDs | Internal Studio Tally |
| **Power Supply** | 5V DC / 4A | 1 | High-current LED Power |

---

## 5. 📸 Media & Gallery
View the build process and the final animations in the [Media Gallery](./Media).

---

## 6. Troubleshooting

| Issue | Likely Cause | Solution |
| :--- | :--- | :--- |
| **"No Wifi" Message** | Band Mismatch | Feather M0 requires **2.4GHz** Wi-Fi. |
| **rtpMIDI Not Connecting** | Port Mismatch | Ensure rtpMIDI is targeting **Port 5006**. |
| **Text is Mirrored** | Wiring Direction | Adjust the `drawPixel` logic for Serpentine vs. Progressive layouts. |
| **Matrix is Flickering** | Voltage Drop | Ensure 5V is injected at the start of the matrix; check Common Ground. |

---

## 7. Licence & Support
This project is shared as **Open Source** under the **MIT Licence**. 

**Notice:** This project is provided "as-is." No technical support is provided. You are free to fork and adapt for your own studio needs.

*Designed for Coconut Groove Studio | 2026*
