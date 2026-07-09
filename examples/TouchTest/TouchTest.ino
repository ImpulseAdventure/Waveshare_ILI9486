// ================================================
//  Clean Touch Test for Waveshare 4" ILI9486 on TTgo D1 R32
//  Backlight + SPI fixed + Persistent Calibration
// ================================================

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Waveshare_ILI9486.h>
#include <Preferences.h>

// Colors
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Waveshare_ILI9486 Waveshield;
Adafruit_GFX &tft = Waveshield;
Preferences prefs;

const int BACKLIGHT_PIN = 13;

void setup()
{
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n\n=== WAVESHARE 4\" TOUCH TEST - CLEAN VERSION ===");

    // Your proven display setup
    SPI.begin();
    SPI.setFrequency(8000000);
    Serial.println("✓ SPI 8 MHz");

    Waveshield.begin();
    Serial.println("✓ Display initialized");

    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, HIGH);
    Serial.println("✓ Backlight ON");

    tft.setRotation(1);                 // Change to 0, 2 or 3 if orientation wrong

    // Load saved calibration
    prefs.begin("touchcal", false);
    TSConfigData savedConfig;
    if (prefs.getBytes("calib", &savedConfig, sizeof(TSConfigData)) == sizeof(TSConfigData)) {
        Waveshield.setTsConfigData(savedConfig);
        Serial.println("✅ Calibration LOADED");
    } else {
        Serial.println("No calibration saved yet");
    }

    // Check for force recalibration (hold BOOT button while resetting)
    pinMode(0, INPUT_PULLUP);
    bool forceCal = (digitalRead(0) == LOW);

    tft.fillScreen(BLACK);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);

    if (forceCal) {
        tft.setCursor(25, 70);
        tft.println("CALIBRATING...");
        tft.setTextSize(1);
        tft.setCursor(20, 120);
        tft.println("Run stylus FIRMLY off ALL 4 edges");
        tft.setCursor(20, 150);
        tft.println("several times until drawing is accurate");
        Serial.println("Force calibration started");
        delay(3000);
    } else {
        tft.setCursor(30, 80);
        tft.println("Touch Ready");
        tft.setTextSize(1);
        tft.setCursor(20, 130);
        tft.println("Draw with stylus");
        tft.setCursor(20, 160);
        tft.println("Hold BOOT + Reset to recalibrate");
        delay(3000);
    }

    tft.fillScreen(BLACK);
    Serial.println("Ready - Start drawing!");
}

uint32_t lastDraw = 0;
bool calibratedThisRun = false;

void loop()
{
    TSPoint p = Waveshield.getPoint();

    if (p.z > 80) {                                 // Firm press required for resistive touch
        Waveshield.normalizeTsPoint(p);

        if (millis() - lastDraw > 8) {
            tft.fillCircle(p.x, p.y, 4, BLUE);
            lastDraw = millis();
        }

        // Auto-save calibration after some use
        if (!calibratedThisRun && millis() > 7000) {
            TSConfigData current = Waveshield.getTsConfigData();
            prefs.putBytes("calib", &current, sizeof(TSConfigData));
            Serial.println("💾 Calibration SAVED");
            calibratedThisRun = true;

            tft.setCursor(10, 8);
            tft.setTextColor(GREEN, BLACK);
            tft.setTextSize(1);
            tft.print("Calibration Saved ✓");
        }
    }

    delay(1);
}