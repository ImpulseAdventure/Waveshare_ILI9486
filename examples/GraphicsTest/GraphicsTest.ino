#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Waveshare_ILI9486.h>

Waveshare_ILI9486 Waveshield;
Adafruit_GFX &tft = Waveshield;

const int BACKLIGHT_PIN = 13;

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=== WAVESHARE ILI9486 - WORKING BASE ===");

  // Critical for this shield on ESP32:
  SPI.begin();
  SPI.setFrequency(8000000);        // 8 MHz – do NOT go much higher without testing
  Serial.println("✓ SPI configured at 8 MHz");

  Waveshield.begin();
  Serial.println("✓ Waveshield.begin() completed");

  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);
  Serial.println("✓ Backlight ON");

  // Optional: set rotation (0-3). Try 1 or 3 if the orientation looks wrong.
  tft.setRotation(1);               // Landscape – change as needed

  // Quick test pattern
  tft.fillScreen(0x0000);           // Black
  delay(500);
  tft.fillScreen(0xF800);           // Red
  delay(1000);
  tft.fillScreen(0x07E0);           // Green
  delay(1000);
  tft.fillScreen(0x001F);           // Blue
  delay(1000);

  Serial.println("=== SETUP COMPLETE - Ready for graphics! ===");
}

void loop()
{
  // Your main code goes here
  Serial.println("✓ Loop alive");
  delay(5000);
}