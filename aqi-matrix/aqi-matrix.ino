// Contains code by Marc MERLIN <marc_soft@merlins.org> (c) Adafruit, license BSD
#include <Adafruit_NeoMatrix.h>

#include "healthy.h"
#include "moderate.h"
#include "unhealthy_for_sensitive_groups.h"
#include "unhealthy.h"
#include "very_unhealthy.h"
#include "hazardous.h"

// Data IN pin for the WS2812B device
#define PIN 6

// Maximum is 255. Set to low value to prevent excessive current draw
#define BRIGHTNESS 8

// Size of each tile (matrix) in pixels
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16

// Number of tiles in horizontal and vertical direction
#define TILE_WIDTH 1
#define TILE_HEIGHT 1

/*
For a 16x16 display made of 4 8x8 matrices arranged like this:

+---+---+
| 0 | 1 |
+---+---+
| 3 | 2 |
+---+---+

# define MATRIX_WIDTH 8
# define MATRIX_HEIGHT 8
# define TILE_WIDTH 2
# define TILE_HEIGHT 2

*/

// Full display width and height
#define mw (MATRIX_WIDTH * TILE_WIDTH)
#define mh (MATRIX_HEIGHT * TILE_HEIGHT)



Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT,
                                                    TILE_WIDTH, TILE_HEIGHT,
                                                    PIN,
                                                    NEO_MATRIX_TOP
                                                      + NEO_MATRIX_RIGHT
                                                      + NEO_MATRIX_ROWS
                                                      + NEO_MATRIX_ZIGZAG
                                                      + NEO_TILE_TOP
                                                      + NEO_TILE_LEFT
                                                      + NEO_TILE_ZIGZAG,
                                                    NEO_GRB + NEO_KHZ800);

void displayBitmap(uint8_t bitmapSize, const uint16_t *bitmap16) {
	int16_t xf = max(0, (mw - bitmapSize) / 2) << 4;
	int16_t yf = max(0, (mh - bitmapSize) / 2) << 4;

	int16_t x = xf >> 4;
	int16_t y = yf >> 4;
	matrix->drawRGBBitmap(x, y, (const uint16_t *)bitmap16, bitmapSize, bitmapSize);
}

void drawDigit(int digit, int x, int y, uint16_t color) {
	// (x, y) is coordinates of top-left corner of digit.
	switch (digit) {
		case 0:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x, y + 4, x + 2, y + 4, color);
			matrix->drawLine(x, y, x, y + 4, color);
			matrix->drawLine(x + 2, y, x + 2, y + 4, color);
			return;
		case 1:
			matrix->drawLine(x + 2, y, x + 2, y + 4, color);
			return;
		case 2:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x + 2, y, x + 2, y + 2, color);
			matrix->drawLine(x, y + 2, x + 2, y + 2, color);
			matrix->drawLine(x, y + 2, x, y + 4, color);
			matrix->drawLine(x, y + 4, x + 2, y + 4, color);
			return;
		case 3:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x + 2, y, x + 2, y + 4, color);
			matrix->drawLine(x, y + 2, x + 2, y + 2, color);
			matrix->drawLine(x, y + 4, x + 2, y + 4, color);
			return;
		case 4:
			matrix->drawLine(x, y, x, y + 2, color);
			matrix->drawLine(x + 2, y, x + 2, y + 4, color);
			matrix->drawLine(x, y + 2, x + 2, y + 2, color);
			return;
		case 5:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x, y, x, y + 2, color);
			matrix->drawLine(x, y + 2, x + 2, y + 2, color);
			matrix->drawLine(x + 2, y + 2, x + 2, y + 4, color);
			matrix->drawLine(x, y + 4, x + 2, y + 4, color);
			return;
		case 6:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x, y, x, y + 4, color);
			matrix->drawLine(x + 2, y + 2, x + 2, y + 4, color);
			matrix->drawLine(x, y + 2, x + 2, y + 2, color);
			matrix->drawLine(x, y + 4, x + 2, y + 4, color);
			return;
		case 7:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x + 2, y, x + 2, y + 4, color);
			return;
		case 8:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x, y, x, y + 4, color);
			matrix->drawLine(x + 2, y, x + 2, y + 4, color);
			matrix->drawLine(x, y + 2, x + 2, y + 2, color);
			matrix->drawLine(x, y + 4, x + 2, y + 4, color);
			return;
		case 9:
			matrix->drawLine(x, y, x + 2, y, color);
			matrix->drawLine(x, y, x, y + 2, color);
			matrix->drawLine(x + 2, y, x + 2, y + 4, color);
			matrix->drawLine(x, y + 2, x + 2, y + 2, color);
			matrix->drawLine(x, y + 4, x + 2, y + 4, color);
			return;
		default:
			matrix->drawRect(x, y, 3, 5, color);
			return;
	}
}

void displayAqi(long aqi) {
	if (aqi < 0) {
		return;
	}

	bool thousandOrMore = aqi > 999;
	aqi = thousandOrMore ? 999 : aqi;
	int x = mw - 2;
	int y = mh - 5;

	if (aqi == 0) {
		drawDigit(0, x - 2, y, matrix->Color(255, 255, 255));  // Edge case for 0
		return;
	}
	while (aqi > 0) {
		int digit = aqi % 10;
		drawDigit(digit, x - 2, y, matrix->Color(255, 255, 255));
		aqi /= 10;
		x -= 4;
	}

	if (thousandOrMore) {
		// Draw '>' sign to indicate 1000+.
		matrix->drawLine(x - 2, y, x, y + 2, matrix->Color(255, 255, 255));
		matrix->drawLine(x - 2, y + 4, x, y + 2, matrix->Color(255, 255, 255));
	}
}

void displayErrorText(int x, int y) {
	// Draw a red 'E'.
	matrix->drawLine(x, y, x + 2, y, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y, x, y + 4, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y + 2, x + 2, y + 2, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y + 4, x + 2, y + 4, matrix->Color(255, 0, 0));
	x += 4;
	// Draw a red 'r'.
	matrix->drawLine(x, y + 2, x, y + 4, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y + 2, x + 1, y + 2, matrix->Color(255, 0, 0));
	x += 3;
	// Draw a red 'r'.
	matrix->drawLine(x, y + 2, x, y + 4, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y + 2, x + 1, y + 2, matrix->Color(255, 0, 0));
	x += 3;
	// Draw a red 'o'.
	matrix->drawLine(x, y + 2, x + 2, y + 2, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y + 2, x, y + 4, matrix->Color(255, 0, 0));
	matrix->drawLine(x + 2, y + 2, x + 2, y + 4, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y + 4, x + 2, y + 4, matrix->Color(255, 0, 0));
	x += 4;
	// Draw a red 'r'.
	matrix->drawLine(x, y + 2, x, y + 4, matrix->Color(255, 0, 0));
	matrix->drawLine(x, y + 2, x + 1, y + 2, matrix->Color(255, 0, 0));
}

struct AQILevel {
	float min_aqi;
	float max_aqi;
	const char *name;
};

const AQILevel aqi_levels[] = {
	{ 0, 50, "HEALTHY" },                            // green
	{ 51, 100, "MODERATE" },                         // yellow
	{ 101, 150, "UNHEALTHY_FOR_SENSITIVE_GROUPS" },  // orange
	{ 151, 200, "UNHEALTHY" },                       // red
	{ 201, 300, "VERY_UNHEALTHY" },                  // purple
	{ 301, 99999, "HAZARDOUS" }                      // blue
};

const int NUM_LEVELS = sizeof(aqi_levels) / sizeof(aqi_levels[0]);


const char *get_level_name(float aqi) {
	if (aqi < 0) return "ERROR";

	for (int i = 0; i < NUM_LEVELS; i++) {
		if (aqi >= aqi_levels[i].min_aqi && aqi <= aqi_levels[i].max_aqi) {
			return aqi_levels[i].name;
		}
	}
	return aqi_levels[NUM_LEVELS - 1].name;
}


void setup() {
	Serial.begin(9600);
	Serial.flush();
	matrix->begin();
	matrix->setTextWrap(false);
	matrix->setBrightness(BRIGHTNESS);
	matrix->clear();
	matrix->show();
}

void loop() {
	if (Serial.available() <= 0) {
		return;
	}
	const String input = Serial.readStringUntil("\n");
	if (input.length() == 0) {
		return;  // .toInt() would return 0 for empty string, which is not desired
	}
	long aqi = input.toInt();
	if (aqi == 0) {
		return;  // "-1" sent from pyserial means AQI 0. "0" sent from pyserial is no-op.
	}

	matrix->clear();

	bool isError = false;

	if (input.length() > 7 || aqi < -1) {
		displayErrorText(0, 5);
		isError = true;
	} else if (aqi == -1 || (0 < aqi && aqi <= 50)) {
		displayBitmap(16, healthy);  // Green
	} else if (51 <= aqi && aqi <= 100) {
		displayBitmap(16, moderate);  // Yellow
	} else if (101 <= aqi && aqi <= 150) {
		displayBitmap(16, unhealthy_for_sensitive_groups);  // Orange
	} else if (151 <= aqi && aqi <= 200) {
		displayBitmap(16, unhealthy);  // Red
	} else if (201 <= aqi && aqi <= 300) {
		displayBitmap(16, very_unhealthy);  // Purple
	} else {
		displayBitmap(16, hazardous);  // Blue because Maroon is hard to display on WS2812B
	}

	if (!isError) {
		aqi = (aqi == -1) ? 0 : aqi;  // Special case for 0
		displayAqi(aqi);
		// Serial.print(F("AQI: "));
		// Serial.print(aqi);
		// Serial.print(F(" | Level: "));
		// Serial.println(get_level_name(aqi));
	}

	matrix->show();
}
