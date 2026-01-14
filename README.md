# MCU Transport Display: Coconut Groove Studio

A hardware status display for **PreSonus Studio One** using an **ESP32** and an **8x32 NeoPixel matrix**. The system emulates a **Mackie Control (MCU)** surface to intercept transport state changes over **rtpMIDI**.

### Features
* **Dynamic Transport States:**
    * **Idle:** Scrolls `COCONUT GROOVE`.
    * **Playback:** Scrolls `PLAYBACK`.
    * **Record:** Flashing `RECORD` visual with synchronized graphics.
* **Robust Connectivity:**
    * Connects via Wi-Fi to the host PC.
    * Displays `NO WIFI` if the network is unreachable.
    * **Auto-Reconnection:** Persistent polling for the rtpMIDI link and/or WiFi if the connection drops.
    * **Power Management:** The display automatically blanks when the PC is powered down or the MIDI link is inactive.

### Technical Stack
* **Microcontroller:** ESP32 Dev Board
* **LED Matrix:** 8x32 WS2812B (NeoPixel) Addressable LEDs
* **Protocol:** Mackie Control (MCU) over rtpMIDI (AppleMIDI)
* **DAW:** PreSonus Studio One - but any DAW with **MCU** shoud be compatible

---
*© 2026 Coconut Groove Studio*
