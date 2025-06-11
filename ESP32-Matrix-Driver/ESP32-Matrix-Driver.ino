#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#define PIN 16
#define NUMPIXELS 256

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 100  // Time (in milliseconds) to pause between pixels

int getFlippedIndex(int x, int y) {
  if (y % 2 == 0) {
    return y * 16 + x;  // Even rows: left-to-right
  } else {
    return (y + 1) * 16 - 1 - x;  // Odd rows: right-to-left
  }
}

void setup() {
  pixels.begin();
  pixels.setBrightness(10);
}

void loop() {
  pixels.clear();

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      pixels.setPixelColor(getFlippedIndex(x, y), pixels.Color(x * 16, y * 16, 0));
    }
  }

  pixels.show();

  delay(DELAYVAL);
}
