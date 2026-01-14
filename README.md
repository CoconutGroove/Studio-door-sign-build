MCU Transport Display: Coconut Groove Studio
A hardware status display for PreSonus Studio One using an ESP32 and an 8x32 NeoPixel matrix. The system emulates a Mackie Control (MCU) surface to intercept transport state changes over rtpMIDI.

Features
Dynamic Transport States:

Idle: Scrolls COCONUT GROOVE.

Playback: Scrolls PLAYBACK.

Record: Flashing RECORD visual with synchronized graphics.

Robust Connectivity:

Connects via Wi-Fi to the host PC.

Displays NO WIFI if the network is unreachable.

Auto-Reconnection: Persistent polling for rtpMIDI link if the connection drops.

Power Management: Display automatically blanks when the PC is powered down or the MIDI link is inactive.

Technical Stack
Microcontroller: ESP32 Dev Board.

LED Matrix: 8x32 WS2812B (NeoPixel) Addressable LEDs.

Protocol: Mackie Control (MCU) over rtpMIDI (AppleMIDI).

DAW: PreSonus Studio One (configured as a Mackie Control External Device).

Implementation Notes
The ESP32 parses specific MCU MIDI messages (Standard Note On/Off commands for Play and Record buttons) to trigger the corresponding animations on the matrix. By using the MCU protocol, the sign stays perfectly in sync with the DAW's internal clock and transport state.
