//
// GFX style API wrapper for Waveshare_ILI9486
// - Intended to work with the following displays:
//   - Waveshare 4.0" Touch Shield for Arduino
//   - Waveshare 3.5" Touch Shield for Arduino
//
// - Referenced API attribution at bottom of file
//

#include "Waveshare_ILI9486.h"
#include "Waveshare_ILI9486_GFX.h"

Waveshare_ILI9486_GFX::Waveshare_ILI9486_GFX(void)
{
	// Assign display dimensions
	// - TODO: Support rotation
	_width = LCD_WIDTH;
	_height = LCD_HEIGHT;

	_cursor_x = 0;
	_cursor_y = 0;

	_textsize_x = 1;
	_textsize_y = 1;
	_textcolor = 0xFFFF;
	_textbgcolor = 0xFFFF;
	_font = NULL;
	_wrap = true;

}

void Waveshare_ILI9486_GFX::begin()
{
	System_Init();

	LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
	LCD_Init(Lcd_ScanDir, 100);

}

void Waveshare_ILI9486_GFX::drawPixel(int16_t x, int16_t y, uint16_t color)
{
	GUI_DrawPoint((POINT)x, (POINT)y, color,
		DOT_PIXEL_1X1, DOT_FILL_AROUND);
}

void Waveshare_ILI9486_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	GUI_DrawLine(x0, y0, x1, y1, color, LINE_SOLID, DOT_PIXEL_1X1);
}

void Waveshare_ILI9486_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	GUI_DrawRectangle(x, y, x + w - 1, y + h, color, DRAW_EMPTY, DOT_PIXEL_1X1);
}

void Waveshare_ILI9486_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	GUI_DrawRectangle(x, y, x + w - 1, y + h, color, DRAW_FULL, DOT_PIXEL_1X1);
}

void Waveshare_ILI9486_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	GUI_DrawCircle(x0, y0, r, color, DRAW_EMPTY, DOT_PIXEL_1X1);
}

void Waveshare_ILI9486_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	GUI_DrawCircle(x0, y0, r, color, DRAW_FULL, DOT_PIXEL_1X1);
}

void Waveshare_ILI9486_GFX::fillScreen(uint16_t color)
{
	fillRect(0, 0, _width, _height, color);
}

void Waveshare_ILI9486_GFX::setFont(const sFONT *f)
{
	_font = f;
}

void Waveshare_ILI9486_GFX::setCursor(int16_t x, int16_t y)
{
	_cursor_x = x;
	_cursor_y = y;
}

void Waveshare_ILI9486_GFX::setTextColor(uint16_t c)
{
	_textcolor = c;
	_textbgcolor = c;
}

void Waveshare_ILI9486_GFX::setTextColor(uint16_t c, uint16_t bg)
{
	_textcolor = c;
	_textbgcolor = bg;
}

void Waveshare_ILI9486_GFX::setTextSize(uint8_t s)
{
	setTextSize(s, s);
}

void Waveshare_ILI9486_GFX::setTextSize(uint8_t s_x, uint8_t s_y)
{
	// TODO: Since the Waveshare GUI_DisChar() doesn't support
	//       pixel expansion during rendering, we will disable
	//       updates to the text sizing parameters for now.
	//       They will remain as the default (1,1).

	//_textsize_x = (s_x > 0) ? s_x : 1;
	//_textsize_y = (s_y > 0) ? s_y : 1;
}

void Waveshare_ILI9486_GFX::setTextWrap(bool w)
{
	_wrap = w;
}

size_t Waveshare_ILI9486_GFX::write(uint8_t c)
{
	// Default font sizing (Waveshare "font8")
	// - The Waveshare API only supports monospaced fonts.
	// - Note that Waveshare's default "font8" is actually sized
	//   5x8, but we are extending it slightly to 6x8 to match
	//   the default GFX built-in font.
	int8_t nCharW = 6;
	int8_t nCharH = 8;

	// If a font has been assigned, fetch the dimensions
	// from the font record.
	if (_font != NULL) {
		nCharW = _font->Width;
		nCharH = _font->Height;
	}

	// Render each character and advance the cursor
	if (c == '\n') {                             // Newline?
		_cursor_x = 0;                           // Reset x to zero,
		_cursor_y += _textsize_y * nCharH;       // advance y one line
	}
	else if (c != '\r') {                        // Ignore carriage returns
		if (_wrap && ((_cursor_x + _textsize_x * nCharW) > _width)) { // Off right?
			_cursor_x = 0;                       // Reset x to zero,
			_cursor_y += _textsize_y * nCharH;   // advance y one line
		}
		drawChar(_cursor_x, _cursor_y, c, _textcolor, _textbgcolor, _textsize_x, _textsize_y);
		_cursor_x += _textsize_x * nCharW;       // Advance x one char
	}
	return 1;
}

void Waveshare_ILI9486_GFX::drawChar(int16_t x, int16_t y, unsigned char c,
	uint16_t color, uint16_t bg, uint8_t size) {
	drawChar(x, y, c, color, bg, size, size);
}

void Waveshare_ILI9486_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
	uint16_t bg, uint8_t size_x, uint8_t size_y)
{
	// NOTE: No support for textsize
	// - Instead, alternate sized fonts should be selected via setFont()
	// - Alternately, the Waveshare API (GUI_DisChar) could be enhanced
	//   to use pixel replication to support the textsize API.

	// Specify default font if none assigned with setFont()
	const sFONT* pFont = _font;
	if (pFont == NULL) {
		pFont = &Font8;
	}

	// Note that Waveshare convention for "transparent" text handling is
	// different than the GFX convention. The Waveshare convention is to
	// indicate transparent text handling when the backround color equals
	// the constant FONT_BACKGROUND, which is WHITE (0xFFFF). This
	// is not a very useful convention since it prevents one from
	// using background erase mode with a true white background. In the interest
	// of offering unchanged Waveshare APIs, the Waveshare GUI_DisChar()
	// implementation was not changed, but this GFX API will attempt
	// to convert between the methodologies.

	if (color == bg) {
		// GFX: transparent mode
		if (bg == WHITE) {
			// GFX: white text, transparent
			// No adjustment required for waveshare
		}
		else {
			// GFX: colored text, transparent
			// Waveshare needs to force background to WHITE
			bg = FONT_BACKGROUND; // Hardcoded to WHITE in Waveshare_ILI9486.h
		}
	}
	else {
		// GFX: erase mode
		if (bg == WHITE) {
			// GFX: colored text, erase white background
			// For waveshare, we must not use WHITE background
			// color as otherwise it will resort to transparent mode.
			// Therefore, we force a minor change to the white background
			// so that it isn't detected as FONT_BACKGROUND (WHITE / 0xFFFF).
			bg = 0xFFFE; // Force almost white but don't match WHITE
		}
		else {
			// GFX: colored text, erase other color background
			// For waveshare we can leave it as is
		}
	}

	// Note that GUI_DisChar() drops the const attribute on the font param
	GUI_DisChar(x, y, c, (sFONT*)pFont, bg, color);
}

// ----------------------------------------------------------------------------

//
// The above API is based on the Adafruit-GFX API
// The copyright notice associated with the Adafruit-GFX library
// has been reproduced below.
//
/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).
Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!
Copyright (c) 2013 Adafruit Industries.  All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
