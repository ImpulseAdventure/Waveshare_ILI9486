# Waveshare ILI9486 Arduino
Graphics and touchscreen drivers for Waveshare 3.5" and 4.0" Shields

This is an Arduino library for the Waveshare ILI9486 SPI touch shield displays and
is a re-implementation of the 'Adafruit_GFX' graphics API. This version is
substantially faster than the original sample software and provides an integrated
touchscreen interface.

- [Waveshare 4" Touch LCD Shield for Arduino](https://www.waveshare.com/product/modules/oleds-lcds/arduino-lcd/4inch-tft-touch-shield.htm) (SKU: 13587)
- [Waveshare 3.5" Touch LCD Shield for Arduino](https://www.waveshare.com/product/modules/oleds-lcds/arduino-lcd/3.5inch-tft-touch-shield.htm) (SKU: 13506)

The touch screen will also calibrate itself automatically, and provides access to the
calibration data so that is can be stored (for example, in EEPROM) and then set, so that
the touch screen will maintain its calibration across reboots.

## Examples
Under examples there is:
 - GraphicsTest
 - TouchTest

 GraphicsTest runs through the Adafruit_GFX test code, and gives a timing speed for it.
 This is about 25 seconds for the whole run.  The original Waveshare library used a cut
 down version of the Adafruit tests - that has been replicated as well so that times can
 be compared.  On a Mega 2560, the original library took about 70 seconds to complete.
 This library is down to just under 16 seconds.

 TouchTest is a *very* simple 'painting' program.  It shows how to read and normalize the
 touch points, and how the screen calibration is handled.  When first started, the drawing
 will not occur under the stylus.  Once the screen is calibrated, the drawng and stylus
 should be very closely aligned.

 To calibrate the screen, simply run the stylus off all four edges of the screen a few
 times.  This calibration can be retrieved using `getTsConfigData()`, saved, and restored
 later using `setTsConfigData()`.

 This library can be installed manually, or it is available from the Arduino IDE Library Manager.
 Search for "waveshare4inch" in the Library Manager.

## References

Wiki support pages:
- [Waveshare 4inch TFT Touch Shield Wiki](https://www.waveshare.com/wiki/4inch_TFT_Touch_Shield)
- [Waveshare 3.5inch TFT Touch Shield Wiki](https://www.waveshare.com/wiki/3.5inch_TFT_Touch_Shield)

## Authors

* **Waveshare team** - *Original driver code* - [Waveshare](https://www.waveshare.com/)
* **ImpulseAdventure** - *Arduino library adaptation* - [ImpulseAdventure](https://github.com/ImpulseAdventure/)
* **M Hotchin** - *Updated Arduino library*

## License

- This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
- The Waveshare team has requested that their code retain a note indicating that the original code is from Waveshare.
- This library leverages the Adafruit-GFX API, with a BSD license.
