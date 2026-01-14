# MCU Transport Display: Coconut Groove Studio

A high-visibility hardware status display for **PreSonus Studio One**. This project uses an **ESP32** to emulate a **Mackie Control (MCU)** surface, intercepting transport commands via **rtpMIDI** to drive an 8x32 NeoPixel matrix.

---

## 1. System Logic & Algorithm
The firmware is designed with a priority-state hierarchy to ensure the display is always accurate and energy-efficient:

* **Stage 1: Connection Health:** The system verifies Wi-Fi status. If disconnected, it triggers "Recovery Mode," pulsing a `No Wifi` message while attempting to reconnect every 5 seconds.
* **Stage 2: Session Monitoring:** It uses rtpMIDI "Heartbeats" to see if the DAW is active. If the session is closed, the display blanks automatically to prevent LED burn-in.
* **Stage 3: MCU MIDI Parsing:** The ESP32 listens for specific MIDI Note-On triggers sent by the DAW:
    * **Note 0x5F (95):** Set State to **RECORD**.
    * **Note 0x5E (94):** Set State to **PLAYBACK**.
    * **Note 0x5D (93):** Set State to **IDLE/STOP**.
* **Stage 4: Rendering Engine:** The code translates (X, Y) coordinates to a **Serpentine Layout** to ensure text scrolls correctly across the zigzag wiring of the matrix.

---

## 2. Construction Manual

### Hardware Wiring
1.  **Matrix (Outside):** Connect Data Input (DI) to **GPIO 18**.
2.  **Status Strip (Inside):** Connect Data Input (DI) to **GPIO 19**.
3.  **Power:** Connect a dedicated **5V 4A power supply** to the LED rails. Ensure the ESP32 and LEDs share a **Common Ground** (GND) connection.

### Software & Code
1.  **The Sketch:** Download the [studio_sign.ino](./studio_sign.ino) file from this repository.
2.  **Configuration:** Open the sketch in the Arduino IDE and customise the `ssid`, `pass`, and `STUDIO_NAME` variables at the top of the file.

### Programming the ESP32
To get the code onto your device, follow these steps:

1.  **Install Arduino IDE:** Download the latest version from the [Arduino website](https://www.arduino.cc/en/software).
2.  **Add ESP32 Support:** * Open **File > Preferences**.
    * In "Additional Boards Manager URLs", paste: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    * Go to **Tools > Board > Boards Manager**, search for **ESP32**, and click **Install**.
3.  **Install Required Libraries:** * Go to **Sketch > Include Library > Manage Libraries**.
    * Search for and install **Adafruit NeoPixel**.
    * Search for and install **AppleMIDI**.
4.  **Connect & Upload:**
    * Connect your ESP32 via USB.
    * Select **ESP32 Dev Module** under Tools > Board.
    * Select your board's COM/USB port under Tools > Port.
    * Click the **Upload** arrow icon.

---

## 3. DAW Connectivity

#### **For Mac Users (Native Setup)**
macOS handles this natively via the "MIDI Network Driver":
1.  Open **Audio MIDI Setup** (Applications > Utilities).
2.  Go to **Window > Show MIDI Studio**.
3.  Double-click the **Network** icon (the globe).
4.  In the **Directory**, select your ESP32 and click **Connect**. Ensure the session is **Enabled**.

#### **For Windows Users (rtpMIDI)**
1.  Install **rtpMIDI** (by Tobias Erichsen).
2.  Create a session, find your device in the directory, and click **Connect**.

#### **Studio One Setup**
In **Options > External Devices**, add a **Mackie Control**. Set the Receive/Send ports to your newly created Network Session.

---

## 4. Bill of Materials (BOM)

| Component | Specification | Quantity | Purpose |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | ESP32 DevBoard | 1 | System Brain |
| **LED Matrix** | WS2812B 8x32 Flexible | 1 | External Signage |
| **Internal LEDs** | WS2812B LED Strip | ~24 LEDs | Internal Studio Tally |
| **Power Supply** | 5V DC / 4A | 1 | Reliable Power |

---

## 5. 📸 Media & Gallery
Want to see the sign in action? You can view high-resolution build photos and a video demonstration in our [Media Gallery](./Media).

* **Construction Photos:** Detailed shots of the internal wiring and 5V power injection.
* **Video Demo:** The sign synchronising with Studio One transport controls in real-time.

---

## 6. Troubleshooting

| Issue | Likely Cause | Solution |
| :--- | :--- | :--- |
| **"No Wifi" Message** | Network Band Mismatch | Ensure you are using 2.4GHz Wi-Fi. |
| **Blank Display** | Session Inactive | Verify the device is "Connected" in rtpMIDI or Audio MIDI Setup. |
| **Garbled Text** | Layout Mismatch | Ensure the matrix is wired in a Serpentine/Zig-Zag pattern. |
| **Flickering** | Power/Ground | Check common ground and add a 330Ω resistor to data lines. |

---

## 7. Licence & Support
This project is shared as **Open Source** under the **MIT Licence**. 

**Important Notice:**
* This project is provided "as-is" for the community. 
* **No technical support or troubleshooting assistance is provided.** * You are free to fork the repository and adapt the code for your own studio needs.

*Designed for Coconut Groove Studio | 2026*
