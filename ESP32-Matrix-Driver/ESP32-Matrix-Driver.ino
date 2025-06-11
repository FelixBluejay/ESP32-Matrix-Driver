#include <Adafruit_NeoPixel.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
String deviceName = "ESP32 Matrix Controller";


#define PIN 16
#define NUMPIXELS 256

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 100

int getFlippedIndex(int x, int y) {
  if (y % 2 == 0) {
    return y * 16 + x;  // Even rows: left-to-right
  } else {
    return (y + 1) * 16 - 1 - x;  // Odd rows: right-to-left
  }
}

JsonDocument doc;


void setup() {
  pixels.begin();
  pixels.setBrightness(10);
  Serial.begin(115200);
  SerialBT.begin(deviceName);
  Serial.printf("\"%s\" is started. You can now pair it with Bluetooth!\n", deviceName.c_str());
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
    const char* json = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}"; 

    DeserializationError error = deserializeJson(doc, json);
    if (error) {
      Serial.print("deserializeJson() returned ");
      Serial.println(error.c_str());
      return;
    } else {
      const char* sensor = doc["sensor"];
      Serial.println(sensor);
    }
  }

  pixels.clear();

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      pixels.setPixelColor(getFlippedIndex(x, y), pixels.Color(x * 16, y * 16, 0));
    }
  }

  pixels.show();

  delay(DELAYVAL);
}
