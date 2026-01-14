/* * Project: Universal Studio Transport Display (MCU over rtpMIDI)
 * Hardware: ESP32 Dev Board & 8x32 NeoPixel Matrix (WS2812B)
 * Creator: Coconut Groove Studio
 * * DESCRIPTION:
 * This sketch connects to a DAW (like Studio One) via rtpMIDI using the 
 * Mackie Control Universal (MCU) protocol. It displays real-time 
 * transport status: Idle, Playback, and Recording.
 * * REQUIRED LIBRARIES:
 * 1. AppleMIDI (by Francois Best)
 * 2. Adafruit NeoPixel (by Adafruit)
 * 3. WiFi (Built-in ESP32)
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <AppleMIDI.h>
#include <Adafruit_NeoPixel.h>

// --- USER CONFIGURATION (Change these) ---
char const ssid[] = "YOUR_WIFI_SSID";          // Replace with your WiFi Name
char const pass[] = "YOUR_WIFI_PASSWORD";      // Replace with your WiFi Password
const char* STUDIO_NAME = "YOUR STUDIO NAME ";  // Note the space at the end for scrolling

// --- HARDWARE PIN MAPPING ---
#define INSIDE_PIN     19    // Status LED strip inside the studio
#define OUTSIDE_PIN    18    // 8x32 Matrix outside the door
#define NUM_INSIDE     24    // Number of LEDs in the internal strip
#define LED_COUNT      256   // Total LEDs in 8x32 matrix (8 * 32)

// --- NETWORK CONFIGURATION ---
// Set a static IP to ensure rtpMIDI connects reliably every time
IPAddress local_IP(192, 168, 1, 202); 
IPAddress gateway(192, 168, 1, 1);    
IPAddress subnet(255, 255, 255, 0);   

// --- GLOBAL VARIABLES ---
unsigned long lastPPMUpdate = 0;
unsigned long lastScrollTime = 0;
unsigned long lastWiFiCheck = 0; 
int ppmLevels[4] = {0,0,0,0};
float scannerPos = 3.0;
float scannerDir = 0.56;
uint16_t rainbowHue = 0;
int scrollX = 32;
int activeSessions = 0;  // Tracks if rtpMIDI session is active
int currentStatus = 0;   // 0=Idle, 1=Playback, 2=Recording

uint32_t colBlue  = 0x0000FF; 
uint32_t colGreen = 0x00FF00; 

Adafruit_NeoPixel inside(NUM_INSIDE, INSIDE_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel matrix(LED_COUNT, OUTSIDE_PIN, NEO_GRB + NEO_KHZ800);

// Create MIDI Instance
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "Studio Sign", DEFAULT_CONTROL_PORT);

// --- MATRIX COORDINATE HELPER ---
// Maps X/Y coordinates to a serpentine/zigzag LED matrix layout
void drawPixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= 32 || y < 0 || y >= 8) return;
  int index = (x % 2 == 0) ? (x * 8) + y : (x * 8) + (7 - y);
  matrix.setPixelColor(index, color);
}

// --- FONT DATA ---
// Bitmasks for a 5x7 custom font
const uint8_t font8[][5] = {
  {0x7f, 0x02, 0x04, 0x08, 0x7f}, {0x38, 0x44, 0x44, 0x38, 0x00}, {0x7f, 0x10, 0x08, 0x10, 0x7f},
  {0x00, 0x44, 0x7d, 0x40, 0x00}, {0x08, 0x7e, 0x09, 0x01, 0x00}, {0x3e, 0x41, 0x41, 0x41, 0x22},
  {0x3e, 0x41, 0x41, 0x41, 0x3e}, {0x7f, 0x02, 0x04, 0x08, 0x7f}, {0x3f, 0x40, 0x40, 0x40, 0x3f},
  {0x01, 0x01, 0x7f, 0x01, 0x01}, {0x7f, 0x09, 0x09, 0x09, 0x06}, {0x7f, 0x40, 0x40, 0x40, 0x40},
  {0x7e, 0x11, 0x11, 0x11, 0x7e}, {0x03, 0x04, 0x78, 0x04, 0x03}, {0x7f, 0x49, 0x49, 0x49, 0x36},
  {0x7f, 0x08, 0x14, 0x22, 0x41}, {0x3e, 0x41, 0x49, 0x49, 0x7a}, {0x7f, 0x09, 0x19, 0x29, 0x46},
  {0x1f, 0x20, 0x40, 0x20, 0x1f}, {0x7f, 0x49, 0x49, 0x49, 0x41}, {0x00, 0x00, 0x00, 0x00, 0x00}
};

void drawChar(char c, int x, int y, uint32_t color) {
  int idx = 20; // Default to space
  if (c == 'N') idx = 0;      else if (c == 'o') idx = 1; 
  else if (c == 'W') idx = 2; else if (c == 'i') idx = 3; 
  else if (c == 'f') idx = 4; else if (c == 'C') idx = 5; 
  else if (c == 'O') idx = 6; else if (c == 'U') idx = 8; 
  else if (c == 'T') idx = 9; else if (c == 'P') idx = 10;
  else if (c == 'L') idx = 11;else if (c == 'A') idx = 12;
  else if (c == 'Y') idx = 13;else if (c == 'B') idx = 14;
  else if (c == 'K') idx = 15;else if (c == 'G') idx = 16;
  else if (c == 'R') idx = 17;else if (c == 'V') idx = 18;
  else if (c == 'E') idx = 19;
  for (int col = 0; col < 5; col++) {
    uint8_t line = font8[idx][col];
    for (int row = 0; row < 7; row++) {
      if (line & (1 << row)) drawPixel(x + col, y + row, color);
    }
  }
}

// Visual feedback when WiFi connection is lost
void drawCenteredWiFi() {
  float pulse = (sin(millis() / 400.0) * 100.0) + 100.0; 
  uint32_t purple = matrix.Color((int)pulse, 0, (int)pulse);
  drawChar('N', 1, 1, purple); drawChar('o', 7, 1, purple); drawChar('W', 14, 1, purple); 
  drawChar('i', 19, 1, purple); drawChar('f', 23, 1, purple); drawChar('i', 27, 1, purple); 
}

// --- MIDI HANDLERS ---
// Parses Mackie Control Protocol Notes for Transport
void doNoteOn(byte channel, byte pitch, byte velocity) {
  if (velocity == 0) { currentStatus = 0; } 
  else {
    if (pitch == 0x5F) currentStatus = 2;       // MCU Record Button
    else if (pitch == 0x5E) currentStatus = 1;  // MCU Play Button
    else if (pitch == 0x5D) currentStatus = 0;  // MCU Stop Button
  }
}

void doNoteOff(byte channel, byte pitch, byte velocity) {
  if (pitch == 0x5F || pitch == 0x5E) currentStatus = 0;
}

// --- ANIMATION ROUTINES ---
// Simulates Audio Meters (PPM) on the matrix sides
void drawPPM(int startX, int offset) {
  for (int ch = 0; ch < 2; ch++) {
    int level = ppmLevels[ch + offset];
    for (int h = 0; h < level; h++) {
      uint32_t c = matrix.Color(0, 255, 0);
      if (h > 4) c = matrix.Color(255, 150, 0);
      if (h > 6) c = matrix.Color(255, 0, 0);
      drawPixel(startX + ch, 7 - h, c);
    }
  }
}

// Renders the flashing "RECORD" text and graphics
void drawPulsingRecord(int x, int y) {
  float pulse = (sin(millis() / 125.0) * 127.5) + 127.5; 
  uint32_t col = matrix.Color((int)pulse, 0, 0);
  auto drawR = [&](int ox) {
    for(int i=0; i<5; i++) drawPixel(x+ox, y+i, col);
    drawPixel(x+ox+1, y, col); drawPixel(x+ox+2, y, col); drawPixel(x+ox+3, y+1, col); 
    drawPixel(x+ox+1, y+2, col); drawPixel(x+ox+2, y+2, col); drawPixel(x+ox+3, y+3, col); drawPixel(x+ox+3, y+4, col);
  };
  drawR(0); 
  for(int i=0; i<5; i++) drawPixel(x+5, y+i, col); 
  for(int i=1; i<3; i++) { drawPixel(x+5+i,y,col); drawPixel(x+5+i,y+2,col); drawPixel(x+5+i,y+4,col); }
  for(int i=0; i<5; i++) drawPixel(x+9, y+i, col); 
  for(int i=1; i<3; i++) { drawPixel(x+9+i,y,col); drawPixel(x+9+i,y+4,col); }
  for(int i=0; i<5; i++) { drawPixel(x+13, y+i, col); drawPixel(x+16, y+i, col); } 
  drawPixel(x+14,y,col); drawPixel(x+15,y,col); drawPixel(x+14,y+4,col); drawPixel(x+15,y+4,col);
  drawR(18); 
  
  // Custom drawn 'D' to fit the space
  for(int i=0; i<5; i++) drawPixel(x+23, y+i, col);
  drawPixel(x+24, y, col); drawPixel(x+25, y, col);
  drawPixel(x+26, y+1, col); drawPixel(x+26, y+2, col); drawPixel(x+26, y+3, col);
  drawPixel(x+24, y+4, col); drawPixel(x+25, y+4, col);
}

void drawStudioRecord() {
  uint32_t blueBox = matrix.Color(0, 0, 120); 
  for(int x = 2; x <= 29; x++) { drawPixel(x,0,blueBox); drawPixel(x,6,blueBox); drawPixel(x,7,blueBox); }
  for(int y = 0; y <= 7; y++) { drawPixel(2,y,blueBox); drawPixel(29,y,blueBox); }
  rainbowHue += 500;
  scannerPos += scannerDir;
  if (scannerPos >= 21 || scannerPos <= 3) scannerDir *= -1;
  for (int i = 0; i < 8; i++) {
    uint32_t c = matrix.ColorHSV(rainbowHue + (i * 2000));
    drawPixel((int)scannerPos + i, 6, c); drawPixel((int)scannerPos + i, 7, c);
  }
  if (millis() - lastPPMUpdate > 65) {
    for (int i=0; i<4; i++) {
      int t = random(0, 9);
      if (t > ppmLevels[i]) ppmLevels[i] = t; else if (ppmLevels[i]>0) ppmLevels[i]--;
    }
    lastPPMUpdate = millis();
  }
  drawPPM(0, 0); drawPPM(30, 2);
  drawPulsingRecord(3, 1);
}

// Logic for moving text across the 32-column matrix
void handleScrolling(const char* text, uint32_t color) {
  if (millis() - lastScrollTime > 24) { 
    lastScrollTime = millis(); 
    scrollX--; 
    if (scrollX < (int)(strlen(text) * -6)) scrollX = 32; 
  }
  int xPos = scrollX;
  for (int i = 0; i < (int)strlen(text); i++) { 
    drawChar(text[i], xPos, 1, color); 
    xPos += 6; 
  }
}

// --- SETUP ---
void setup() {
  inside.begin(); matrix.begin();
  inside.setBrightness(150); matrix.setBrightness(40);
  
  // Set Static IP and Connect
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, pass);

  // Initialize MIDI Logic
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(doNoteOn);
  MIDI.setHandleNoteOff(doNoteOff);

  // Monitor rtpMIDI connections
  AppleMIDI.setHandleConnected([](auto, const char* name) { activeSessions++; });
  AppleMIDI.setHandleDisconnected([](auto) { if (activeSessions > 0) activeSessions--; });
}

// --- MAIN LOOP ---
void loop() {
  // 1. Check WiFi Health (Self-Healing)
  if (WiFi.status() != WL_CONNECTED) {
    matrix.clear(); drawCenteredWiFi(); matrix.show();
    inside.clear(); inside.show();
    if (millis() - lastWiFiCheck > 5000) { WiFi.begin(ssid, pass); lastWiFiCheck = millis(); }
    return; 
  }

  // 2. Read incoming MIDI data
  MIDI.read();

  // 3. Auto-Sleep: If no active MIDI session, blank the display
  if (activeSessions <= 0) {
    matrix.clear(); matrix.show();
    inside.clear(); inside.show();
    return; 
  }

  // 4. Handle Visual States
  matrix.clear();
  if (currentStatus == 2) { // RECORDING
    drawStudioRecord(); 
    float pulse = (sin(millis() / 125.0) * 127.5) + 127.5;
    for(int i=0; i<NUM_INSIDE; i++) inside.setPixelColor(i, inside.Color(pulse, 0, 0));
  } 
  else if (currentStatus == 1) { // PLAYBACK
    handleScrolling("PLAYBACK ", colGreen);
    for(int i=0; i<NUM_INSIDE; i++) inside.setPixelColor(i, colGreen);
  } 
  else { // IDLE / STOPPED
    handleScrolling(STUDIO_NAME, colBlue);
    inside.clear();
  }
  matrix.show();
  inside.show();
}