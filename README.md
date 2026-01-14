# MCU Transport Display: Coconut Groove Studio

A high-visibility hardware status display for **PreSonus Studio One**. This project uses an **ESP32** to emulate a **Mackie Control (MCU)** surface, intercepting transport commands via **rtpMIDI** to drive an 8x32 NeoPixel matrix.

---

## 1. System Logic & Algorithm
The firmware is designed with a priority-state hierarchy to ensure the display is always accurate and energy-efficient:

* **Stage 1: Connection Health:** The system first verifies Wi-Fi status. If disconnected, it triggers "Recovery Mode," pulsing a `No Wifi` message while attempting to reconnect every 5 seconds.
* **Stage 2: Session Monitoring:** It uses rtpMIDI "Heartbeats" to see if the DAW is active. If the session is closed, the display blanks automatically to prevent LED burn-in.
* **Stage 3: MCU MIDI Parsing:** The ESP32 listens for specific MIDI Note-On triggers sent by the DAW:
    * **Note 0x5F (95):** Set State to **RECORD**.
    * **Note 0x5E (94):** Set State to **PLAYBACK**.
    * **Note 0x5D (93):** Set State to **IDLE/STOP**.
* **Stage 4: Rendering Engine:** The code maps (x, y) coordinates to a **Serpentine Layout** to ensure text scrolls correctly across the zigzag wiring of the matrix.

---

## 2. Construction Manual

### Hardware Wiring


1.  **Matrix (Outside):** Connect Data Input (DI) to **GPIO 18**.
2.  **Status Strip (Inside):** Connect Data Input (DI) to **GPIO 19**.
3.  **Power:** Connect a dedicated **5V 4A power supply** to the LED rails. Ensure the ESP32 and LEDs share a **Common Ground**.

### Software & Code
1.  **Libraries:** Install `Adafruit_NeoPixel` and `AppleMIDI` in the Arduino IDE.
2.  **The Sketch:** Download the [studio_sign.ino](./studio_sign.ino) file from this repository.
3.  **Configuration:** Customise the `ssid`, `pass`, and `STUDIO_NAME` variables at the top of the sketch before uploading to your ESP32.

### DAW Connectivity

#### **For Mac Users (Native Setup)**
macOS handles this natively via the "MIDI Network Driver":
1.  Open **Audio MIDI Setup** > **Show MIDI Studio**.
2.  Click the **Network** (Globe) icon.
3.  In **Directory**, select your ESP32 and click **Connect**. Ensure the session is **Enabled**.

#### **For Windows Users (rtpMIDI)**
1.  Install **rtpMIDI** (by Tobias Erichsen).
2.  Create a session, find your device in the directory, and click **Connect**.

#### **Studio One Setup**
In **Options > External Devices**, add a **Mackie Control**. Set the Receive/Send ports to your newly created Network Session.

---

## 3. Bill of Materials (BOM)

| Component | Specification | Quantity | Purpose |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | ESP32 DevBoard | 1 | System Brain |
| **LED Matrix** | WS2812B 8x32 Flexible | 1 | External Signage |
| **Internal LEDs** | WS2812B LED Strip | ~24 LEDs | Internal Tally Light |
| **Power Supply** | 5V DC / 4A | 1 | Reliable Power |

---

## 4. Troubleshooting

| Issue | Likely Cause | Solution |
| :--- | :--- | :--- |
| **"No Wifi" Message** | Network Band Mismatch | Ensure you are using 2.4GHz Wi-Fi. |
| **Blank Display** | Session Inactive | Verify the device is "Connected" in rtpMIDI or Audio MIDI Setup. |
| **Garbled Text** | Layout Mismatch | Ensure the matrix is wired in a Serpentine/Zig-Zag pattern. |
| **Flickering** | Power/Ground | Check common ground and add a 330Ω resistor to data lines. |

---
*Developed by Coconut Groove Studio | 2026*
