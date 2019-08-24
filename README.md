# Waveshare ILI9486 Arduino

This is an Arduino library for the Waveshare ILI9486 SPI touch shield displays.
- [Waveshare 4" Touch LCD Shield for Arduino](https://www.waveshare.com/product/modules/oleds-lcds/arduino-lcd/4inch-tft-touch-shield.htm) (SKU: 13587)
- [Waveshare 3.5" Touch LCD Shield for Arduino](https://www.waveshare.com/product/modules/oleds-lcds/arduino-lcd/3.5inch-tft-touch-shield.htm) (SKU: 13506)

The driver from official Waveshare touch shield demo code has been extracted and repackaged as an Arduino
library to make it easier for users to install and reuse in their projects.

A GFX-style wrapper has been added (Waveshare_ILI9486_GFX) to provide some additional compatibility.
Note that only a minimal subset of the APIs have been incorporated at this time.

## References

Wiki support pages:
- [Waveshare 4inch TFT Touch Shield Wiki](https://www.waveshare.com/wiki/4inch_TFT_Touch_Shield)
- [Waveshare 3.5inch TFT Touch Shield Wiki](https://www.waveshare.com/wiki/3.5inch_TFT_Touch_Shield)

Original demo code examples:
- The `/original` folder contains the Waveshare official demo driver code.
- [4inch_TFT_Touch_Shield_Code.7z](https://www.waveshare.com/wiki/File:4inch_TFT_Touch_Shield_Code.7z)
- [3.5inch_TFT_Touch_Shield_Code.7z](https://www.waveshare.com/wiki/File:3.5inch_TFT_Touch_Shield_Code.7z)

## Examples
- A ported GFX "graphicstest" example using the Waveshare_ILI9486_GFX API can be found in Examples -> ws_graphicstest_
- Original example driver code is provided in Examples -> 3.5inch and Examples -> 4.0inch
- A GUI can be built with the [GUIslice](https://github.com/ImpulseAdventure/GUIslice) library
  using the following configuration: `ard-shld-waveshare_40_xpt2046`

## Authors

* **Waveshare team** - *Original driver code* - [Waveshare](https://www.waveshare.com/)
* **ImpulseAdventure** - *Arduino library adaptation* - [ImpulseAdventure](https://github.com/ImpulseAdventure/)

## License

- This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
- The Waveshare team has requested that their code retain a note indicating that the original code is from Waveshare.
- The Waveshare_ILI9486_GFX file leverages the Adafruit-GFX API, with a BSD license.
