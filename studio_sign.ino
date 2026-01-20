/*
 * Studio One MIDI Monitor - Universal Edition
 * -----------------------------------------------
 * Target: Adafruit Feather M0 WiFi (ATWINC1500)
 * Function: Monitors Studio One via rtpMIDI for transport status.
 *
 * Wiring:
 * - Matrix (Outside): Pin 6 - Physically wired to Matrix
 * - Tally Strip (Inside): Pin 11 - Physically wired to Internal Strip
 */

#include <WiFi101.h>
#include <WiFiUdp.h>
#include <AppleMIDI.h>
#include <Adafruit_NeoPixel.h>

// =============================================================================
//                             USER CONFIGURATION
// =============================================================================
char const ssid[] = "YOUR_WIFI_NAME";         // Replace with your WiFi SSID
char const pass[] = "YOUR_WIFI_PASSWORD";     // Replace with your WiFi Password

// Network Settings (Adjust to match your router's range)
IPAddress local_IP(192, 168, 1, 202);         // Desired Static IP for this device
IPAddress gateway(192, 168, 1, 1);            // Your Router's IP Address
IPAddress subnet(255, 255, 255, 0);
IPAddress studioOneIP(192, 168, 1, 64);       // The IP of the PC running Studio One

// Display Settings
const char* STUDIO_NAME = "YOUR STUDIO NAME "; // Scrolling text for IDLE mode
// =============================================================================

// --- PIN & PIXEL CONFIG ---
#define MATRIX_PIN      18    
#define INTERNAL_PIN    19   
#define MATRIX_COUNT    256 
#define INTERNAL_COUNT  24   

Adafruit_NeoPixel matrix(MATRIX_COUNT, MATRIX_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel internal(INTERNAL_COUNT, INTERNAL_PIN, NEO_GRB + NEO_KHZ800);

// MIDI Instance on Port 5006
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "Studio-Display", 5006); 

// --- HEARTBEAT & STATUS ---
unsigned long lastMidiActivity = 0;
const unsigned long timeoutLimit = 10000; 
int currentStatus = 0; 

// --- ANIMATION VARIABLES ---
unsigned long lastPPMUpdate = 0;
unsigned long lastDisplayUpdate = 0;
int ppmLevels[4] = {0,0,0,0};
float scannerPos = 3.0;
float scannerDir = 0.56;
uint16_t rainbowHue = 0;
int scrollX = 32;

// --- FONT DATA ---
const uint8_t font8[][5] = {
  {0x7f, 0x02, 0x04, 0x08, 0x7f}, {0x38, 0x44, 0x44, 0x38, 0x00}, {0x7f, 0x10, 0x08, 0x10, 0x7f},
  {0x00, 0x44, 0x7d, 0x40, 0x00}, {0x08, 0x7e, 0x09, 0x01, 0x00}, {0x3e, 0x41, 0x41, 0x41, 0x22},
  {0x3e, 0x41, 0x41, 0x41, 0x3e}, {0x7f, 0x02, 0x04, 0x08, 0x7f}, {0x3f, 0x40, 0x40, 0x40, 0x3f},
  {0x01, 0x01, 0x7f, 0x01, 0x01}, {0x7f, 0x09, 0x09, 0x09, 0x06}, {0x7f, 0x40, 0x40, 0x40, 0x40},
  {0x7e, 0x11, 0x11, 0x11, 0x7e}, {0x03, 0x04, 0x78, 0x04, 0x03}, {0x7f, 0x49, 0x49, 0x49, 0x36},
  {0x7f, 0x08, 0x14, 0x22, 0x41}, {0x3e, 0x41, 0x49, 0x49, 0x7a}, {0x7f, 0x09, 0x19, 0x29, 0x46},
  {0x1f, 0x20, 0x40, 0x20, 0x1f}, {0x7f, 0x49, 0x49, 0x49, 0x41}, {0x00, 0x00, 0x00, 0x00, 0x00}
};

void drawPixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= 32 || y < 0 || y >= 8) return;
  int index = (x % 2 == 0) ? (x * 8) + y : (x * 8) + (7 - y);
  matrix.setPixelColor(index, color);
}

void drawChar(char c, int x, int y, uint32_t color) {
  int idx = 20; 
  if (c == 'N') idx = 0;       else if (c == 'o') idx = 1; 
  else if (c == 'W') idx = 2; else if (c == 'i') idx = 3; 
  else if (c == 'f') idx = 4; else if (c == 'C') idx = 5; 
  else if (c == 'O') idx = 6; else if (c == 'U') idx = 8; 
  else if (c == 'T') idx = 9; else if (c == 'P') idx = 10;
  else if (c == 'L') idx = 11;else if (c == 'A') idx = 12;
  else if (c == 'Y') idx = 13;else if (c == 'B' || c == 'S') idx = 14;
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

void doNoteOn(byte channel, byte pitch, byte velocity) {
  lastMidiActivity = millis();
  if (velocity == 0) { currentStatus = 0; } 
  else {
    if (pitch == 0x5F) currentStatus = 2;      
    else if (pitch == 0x5E) currentStatus = 1; 
    else if (pitch == 0x5D) { currentStatus = 0; scrollX = 32; }
  }
}

void setup() {
  WiFi.setPins(8, 7, 4, 2); 
  matrix.begin();
  matrix.setBrightness(40);
  internal.begin();
  internal.setBrightness(150);
  
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, pass);
  
  MIDI.begin(1);
  MIDI.setHandleNoteOn(doNoteOn);

  AppleMIDI.setHandleConnected([](uint32_t ssrc, const char* name) {
    isPCConnected = true;
  });
  AppleMIDI.setHandleDisconnected([](uint32_t ssrc) {
    isPCConnected = false;
  });

  lastMidiActivity = millis();
}

void loop() {
  unsigned long now = millis();
  int wifiStatus = WiFi.status();

  if (wifiStatus != WL_CONNECTED) {
    isPCConnected = false;
    wasConnected = false;
    matrix.clear();
    float pulse = (sin(now / 100.0) * 100.0) + 100.0; 
    uint32_t purple = matrix.Color((int)pulse, 0, (int)pulse);
    drawChar('N', 1, 1, purple); drawChar('o', 7, 1, purple); drawChar('W', 14, 1, purple); 
    drawChar('i', 19, 1, purple); drawChar('f', 23, 1, purple); drawChar('i', 27, 1, purple); 
    matrix.show();
    
    if (now - lastWiFiCheck > 10000) {
      WiFi.begin(ssid, pass);
      lastWiFiCheck = now;
    }
    return;
  } 

  if (wifiStatus == WL_CONNECTED && !wasConnected) {
    wasConnected = true;
    MIDI.begin(1); 
  }

  MIDI.read();
  
  if (now - lastMidiActivity > timeoutLimit) {
    matrix.clear();
    matrix.show();
    internal.clear();
    internal.show();
    return; 
  }

  if (now - lastDisplayUpdate > 25) { 
    lastDisplayUpdate = now;
    matrix.clear();
    if (currentStatus == 2) { 
      drawStudioRecord(); 
      float pulse = (sin(now / 125.0) * 127.5) + 127.5;
      for(int i=0; i<INTERNAL_COUNT; i++) internal.setPixelColor(i, internal.Color(pulse, 0, 0));
    } 
    else if (currentStatus == 1) { 
      scrollX--;
      if (scrollX < -54) scrollX = 32; 
      int xPos = scrollX;
      const char* t = "PLAYBACK ";
      for (int i = 0; i < 9; i++) { drawChar(t[i], xPos, 1, 0x00FF00); xPos += 6; }
      for(int i=0; i<INTERNAL_COUNT; i++) internal.setPixelColor(i, 0x00FF00);
    } 
    else { 
      scrollX--;
      // Calculate scroll length dynamically based on STUDIO_NAME length
      int textWidth = strlen(STUDIO_NAME) * 6;
      if (scrollX < -textWidth) scrollX = 32;
      int xPos = scrollX;
      for (int i = 0; i < strlen(STUDIO_NAME); i++) { 
          drawChar(STUDIO_NAME[i], xPos, 1, 0x0000FF); 
          xPos += 6; 
      }
      internal.clear();
    }
    matrix.show();
    internal.show();
  }
}

