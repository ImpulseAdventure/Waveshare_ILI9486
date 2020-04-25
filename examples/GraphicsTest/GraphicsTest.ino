#include <Arduino.h>

#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Waveshare_ILI9486.h>


// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


namespace
{
	Waveshare_ILI9486 Waveshield;

	//  Provide an alias so I don't have to search-and-replace all the code below.
	Adafruit_GFX &tft = Waveshield;


	//
	//  This is all copied from the original 'Waveshare _ILI9486' project.
	unsigned long testFillScreen();
	unsigned long testText();
	unsigned long testLines(uint16_t color);
	unsigned long testFastLines(uint16_t color1, uint16_t color2);
	unsigned long testRects(uint16_t color);
	unsigned long testFilledRects(uint16_t color1, uint16_t color2);
	unsigned long testFilledCircles(uint8_t radius, uint16_t color);
	unsigned long testCircles(uint8_t radius, uint16_t color);
	unsigned long testTriangles();
	unsigned long testFilledTriangles();
	unsigned long testRoundRects();
	unsigned long testFilledRoundRects();

	uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
	{
		return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
	}

	typedef struct
	{
		PGM_P msg;
		uint32_t ms;
	} TEST;
	TEST result[12];

#define RUNTEST(n, str, test) { result[n].msg = PSTR(str); result[n].ms = test; delay(500); }
	void runtests(void)
	{
		uint8_t i, len = 24;
		uint32_t total;
		RUNTEST(0, "FillScreen               ", testFillScreen());
		yield();
		RUNTEST(1, "Text                     ", testText());
		yield();
		RUNTEST(2, "Lines                    ", testLines(CYAN));
		yield();
		RUNTEST(3, "Horiz/Vert Lines         ", testFastLines(RED, BLUE));
		yield();
		RUNTEST(4, "Rectangles (outline)     ", testRects(GREEN));
		yield();
		RUNTEST(5, "Rectangles (filled)      ", testFilledRects(YELLOW, MAGENTA));
		yield();
		RUNTEST(6, "Circles (filled)         ", testFilledCircles(10, MAGENTA));
		yield();
		RUNTEST(7, "Circles (outline)        ", testCircles(10, WHITE));
		yield();
		RUNTEST(8, "Triangles (outline)      ", testTriangles());
		yield();
		RUNTEST(9, "Triangles (filled)       ", testFilledTriangles());
		yield();
		RUNTEST(10, "Rounded rects (outline)  ", testRoundRects());
		yield();
		RUNTEST(11, "Rounded rects (filled)   ", testFilledRoundRects());

		tft.fillScreen(BLACK);
		tft.setTextColor(GREEN);
		tft.setCursor(0, 0);
		uint16_t wid = tft.width();
		if (wid > 176)
		{
			tft.setTextSize(2);
#if defined(MCUFRIEND_KBV_H_)
			tft.print("MCUFRIEND ");
#if MCUFRIEND_KBV_H_ != 0
			tft.print(0.01 * MCUFRIEND_KBV_H_, 1);
#else
			tft.print("for");
#endif
			tft.println(" UNO");
#else
			tft.println("Adafruit-Style Tests");
#endif
		}
		else len = wid / 6 - 8;
		tft.setTextSize(1);
		total = 0;
		for (i = 0; i < 12; i++)
		{
			PGM_P str = result[i].msg;
			char c;
			if (len > 24)
			{
				if (i < 10) tft.print(" ");
				tft.print(i);
				tft.print(": ");
			}
			uint8_t cnt = len;
			while ((c = pgm_read_byte(str++)) && cnt--) tft.print(c);
			tft.print(" ");
			tft.println(result[i].ms);
			total += result[i].ms;
		}
		tft.setTextSize(2);
		tft.print("Total:");
		tft.print(0.000001 * total);
		tft.println("sec");
		//g_identifier = tft.readID();
		//tft.print("ID: 0x");
		//tft.println(tft.readID(), HEX);
		//tft.print("Reg(00):0x");
		//tft.println(tft.readReg(0x00), HEX);
		tft.print("F_CPU:");
		tft.print(0.000001 * F_CPU);
#if defined(__OPTIMIZE_SIZE__)
		tft.println("MHz -Os");
#else
		tft.println("MHz");
#endif

		delay(10000);
	}


	// Standard Adafruit tests.  will adjust to screen size

	unsigned long testFillScreen()
	{
		unsigned long start = micros();
		tft.fillScreen(BLACK);
		tft.fillScreen(RED);
		tft.fillScreen(GREEN);
		tft.fillScreen(BLUE);
		tft.fillScreen(BLACK);
		return micros() - start;
	}

	unsigned long testText()
	{
		unsigned long start;
		tft.fillScreen(BLACK);
		start = micros();
		tft.setCursor(0, 0);
		tft.setTextColor(WHITE);  tft.setTextSize(1);
		tft.println("Hello World!");
		tft.setTextColor(YELLOW); tft.setTextSize(2);
		tft.println(123.45);
		tft.setTextColor(RED);    tft.setTextSize(3);
		tft.println(0xDEADBEEF, HEX);
		tft.println();
		tft.setTextColor(GREEN);
		tft.setTextSize(5);
		tft.println("Groop");
		tft.setTextSize(2);
		tft.println("I implore thee,");
		tft.setTextSize(1);
		tft.println("my foonting turlingdromes.");
		tft.println("And hooptiously drangle me");
		tft.println("with crinkly bindlewurdles,");
		tft.println("Or I will rend thee");
		tft.println("in the gobberwarts");
		tft.println("with my blurglecruncheon,");
		tft.println("see if I don't!");
		return micros() - start;
	}

	unsigned long testLines(uint16_t color)
	{
		unsigned long start, t;
		int           x1, y1, x2, y2,
			w = tft.width(),
			h = tft.height();

		tft.fillScreen(BLACK);

		x1 = y1 = 0;
		y2 = h - 1;
		start = micros();
		for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
		x2 = w - 1;
		for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
		t = micros() - start; // fillScreen doesn't count against timing

		tft.fillScreen(BLACK);

		x1 = w - 1;
		y1 = 0;
		y2 = h - 1;
		start = micros();
		for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
		x2 = 0;
		for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
		t += micros() - start;

		tft.fillScreen(BLACK);

		x1 = 0;
		y1 = h - 1;
		y2 = 0;
		start = micros();
		for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
		x2 = w - 1;
		for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
		t += micros() - start;

		tft.fillScreen(BLACK);

		x1 = w - 1;
		y1 = h - 1;
		y2 = 0;
		start = micros();
		for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
		x2 = 0;
		for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);

		return micros() - start;
	}

	unsigned long testFastLines(uint16_t color1, uint16_t color2)
	{
		unsigned long start;
		int           x, y, w = tft.width(), h = tft.height();

		tft.fillScreen(BLACK);
		start = micros();
		for (y = 0; y < h; y += 5) tft.drawFastHLine(0, y, w, color1);
		for (x = 0; x < w; x += 5) tft.drawFastVLine(x, 0, h, color2);

		return micros() - start;
	}

	unsigned long testRects(uint16_t color)
	{
		unsigned long start;
		int           n, i, i2,
			cx = tft.width() / 2,
			cy = tft.height() / 2;

		tft.fillScreen(BLACK);
		n = min(tft.width(), tft.height());
		start = micros();
		for (i = 2; i < n; i += 6)
		{
			i2 = i / 2;
			tft.drawRect(cx - i2, cy - i2, i, i, color);
		}

		return micros() - start;
	}

	unsigned long testFilledRects(uint16_t color1, uint16_t color2)
	{
		unsigned long start, t = 0;
		int           n, i, i2,
			cx = tft.width() / 2 - 1,
			cy = tft.height() / 2 - 1;

		tft.fillScreen(BLACK);
		n = min(tft.width(), tft.height());
		for (i = n; i > 0; i -= 6)
		{
			i2 = i / 2;
			start = micros();
			tft.fillRect(cx - i2, cy - i2, i, i, color1);
			t += micros() - start;
			// Outlines are not included in timing results
			tft.drawRect(cx - i2, cy - i2, i, i, color2);
			yield();
		}

		return t;
	}

	unsigned long testFilledCircles(uint8_t radius, uint16_t color)
	{
		unsigned long start;
		int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

		tft.fillScreen(BLACK);
		start = micros();
		for (x = radius; x < w; x += r2)
		{
			for (y = radius; y < h; y += r2)
			{
				tft.fillCircle(x, y, radius, color);
			}
		}

		return micros() - start;
	}

	unsigned long testCircles(uint8_t radius, uint16_t color)
	{
		unsigned long start;
		int           x, y, r2 = radius * 2,
			w = tft.width() + radius,
			h = tft.height() + radius;

		// Screen is not cleared for this one -- this is
		// intentional and does not affect the reported time.
		start = micros();
		for (x = 0; x < w; x += r2)
		{
			for (y = 0; y < h; y += r2)
			{
				tft.drawCircle(x, y, radius, color);
			}
		}

		return micros() - start;
	}

	unsigned long testTriangles()
	{
		unsigned long start;
		int           n, i, cx = tft.width() / 2 - 1,
			cy = tft.height() / 2 - 1;

		tft.fillScreen(BLACK);
		n = min(cx, cy);
		start = micros();
		for (i = 0; i < n; i += 5)
		{
			tft.drawTriangle(
				cx, cy - i, // peak
				cx - i, cy + i, // bottom left
				cx + i, cy + i, // bottom right
				color565(0, 0, i));
		}

		return micros() - start;
	}

	unsigned long testFilledTriangles()
	{
		unsigned long start, t = 0;
		int           i, cx = tft.width() / 2 - 1,
			cy = tft.height() / 2 - 1;

		tft.fillScreen(BLACK);
		start = micros();
		for (i = min(cx, cy); i > 10; i -= 5)
		{
			start = micros();
			tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
							 color565(0, i, i));
			t += micros() - start;
			tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
							 color565(i, i, 0));
		}

		return t;
	}

	unsigned long testRoundRects()
	{
		unsigned long start;
		int           w, i, i2, red, step,
			cx = tft.width() / 2 - 1,
			cy = tft.height() / 2 - 1;

		tft.fillScreen(BLACK);
		w = min(tft.width(), tft.height());
		start = micros();
		red = 0;
		step = (256 * 6) / w;
		for (i = 0; i < w; i += 6)
		{
			i2 = i / 2;
			red += step;
			tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, color565(red, 0, 0));
		}

		return micros() - start;
	}

	unsigned long testFilledRoundRects()
	{
		unsigned long start;
		int           i, i2, green, step,
			cx = tft.width() / 2 - 1,
			cy = tft.height() / 2 - 1;

		tft.fillScreen(BLACK);
		start = micros();
		green = 256;
		step = (256 * 6) / min(tft.width(), tft.height());
		for (i = min(tft.width(), tft.height()); i > 20; i -= 6)
		{
			i2 = i / 2;
			green -= step;
			tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, color565(0, green, 0));
			yield();
		}

		return micros() - start;
	}





	void RunSpeedTest()
	{
		unsigned long total = 0;
		unsigned long delta = 0;
		Serial.println(F("Benchmark                Time (microseconds)"));
		delay(10);
		Serial.print(F("Screen fill              "));
		Serial.println(delta = testFillScreen());
		total += delta;
		delay(500);

		Serial.print(F("Text                     "));
		Serial.println(delta = testText());
		total += delta;
		delay(3000);

		Serial.print(F("Lines                    "));
		Serial.println(delta = testLines(CYAN));
		total += delta;
		delay(500);

#if 0
		Serial.print(F("Horiz/Vert Lines         "));
		Serial.println(testFastLines(ILI9486_RED, ILI9486_BLUE));
		delay(500);
#endif

		Serial.print(F("Rectangles (outline)     "));
		Serial.println(delta = testRects(GREEN));
		total += delta;
		delay(500);

		Serial.print(F("Rectangles (filled)      "));
		Serial.println(delta = testFilledRects(YELLOW, MAGENTA));
		total += delta;
		delay(500);

		Serial.print(F("Circles (filled)         "));
		Serial.println(delta = testFilledCircles(10, MAGENTA));
		total += delta;

		Serial.print(F("Circles (outline)        "));
		Serial.println(delta = testCircles(10, WHITE));
		total += delta;
		delay(500);

#if 0
		Serial.print(F("Triangles (outline)      "));
		Serial.println(testTriangles());
		delay(500);

		Serial.print(F("Triangles (filled)       "));
		Serial.println(testFilledTriangles());
		delay(500);

		Serial.print(F("Rounded rects (outline)  "));
		Serial.println(testRoundRects());
		delay(500);

		Serial.print(F("Rounded rects (filled)   "));
		Serial.println(testFilledRoundRects());
		delay(500);
#endif

		Serial.println(F("Done!"));
		Serial.println(total);
	}

}


void setup()
{
	Serial.begin(115200);
	while (!Serial);

	SPI.begin();
	Waveshield.begin();
}


// the loop function runs over and over again until power down or reset
void loop()
{
	//  Run the Adafruit tests.
	//  We'll run using a different rotation to show that everything works!
	tft.setRotation(2);
	runtests();

	//  Run the smaller set that the original Waveshare driver did.  For comparison, the
	//  original code ran in about 71 seconds on a Mega.
	tft.setRotation(0);
	RunSpeedTest();

	//  Show how text prints under different rotations.
	tft.fillScreen(BLUE);
	for (int i = 0; i < 4; i++)
	{
		//  Changing rotation should NOT affect anything currently on the screen!
		tft.setRotation(i);
		tft.print("Hello World! ");
		tft.println(i);
		delay(1000);
	}

	tft.invertDisplay(true);
	delay(3000);
	tft.invertDisplay(false);

	delay(3000);
}


