#include <FastLED.h>
#include <BluetoothSerial.h>

#define DATA_PIN 4 
#define NUM_LEDS 256
CRGB leds[NUM_LEDS];

BluetoothSerial SerialBT;
String deviceName = "ESP32 Matrix Controller";

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);  // Lower = less noise
  
  Serial.begin(115200);
  SerialBT.begin(deviceName);
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }

  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(500);
  
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(500);
}