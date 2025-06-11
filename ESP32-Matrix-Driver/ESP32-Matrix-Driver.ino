#include <FastLED.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>

#define DATA_PIN 4
#define NUM_LEDS 256
CRGB leds[NUM_LEDS];

BluetoothSerial SerialBT;
String deviceName = "ESP32 Matrix Controller";

volatile bool newFrameAvailable = false;
String receivedData = "";

// Task handles
TaskHandle_t bluetoothTaskHandle = NULL;

DynamicJsonDocument doc(16384);  // Increased size for large frame data

void setup() {
  Serial.begin(115200);
  SerialBT.begin(deviceName);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);

  xTaskCreatePinnedToCore(
    bluetoothTask,         // Task function
    "BT_Task",             // Name
    8192,                  // Larger stack for BT
    NULL,                  // Parameters
    2,                     // Higher priority
    &bluetoothTaskHandle,  // Task handle
    1                      // Core 1
  );
}

int getFlippedIndex(int x, int y) {
  if (y % 2 == 0) {
    return y * 16 + x;  // Even rows: left-to-right
  } else {
    return (y + 1) * 16 - 1 - x;  // Odd rows: right-to-left
  }
}

void displayFrame(JsonArray frameData) {
  for (int y = 0; y < 16; y++) {
    JsonArray row = frameData[y];
    Serial.println("Frame data: ");
    for (int x = 0; x < 16; x++) {
      JsonArray pixel = row[x];
      uint8_t r = pixel[1];
      uint8_t g = pixel[0];
      uint8_t b = pixel[2];
      Serial.print("[");
      Serial.print(r);
      Serial.print(", ");
      Serial.print(g);
      Serial.print(", ");
      Serial.print(b);
      Serial.print("], ");
      leds[getFlippedIndex(x, y)] = CRGB(r, g, b);
    }
  }
  FastLED.show();
}

void loop() {
  if (newFrameAvailable) {
    DeserializationError error = deserializeJson(doc, receivedData);
    Serial.println(receivedData);
    if (!error) {
      if (doc.containsKey("cmd") && strcmp(doc["cmd"], "displayFrame") == 0) {
        JsonArray frameData = doc["frameData"];
        displayFrame(frameData);
        Serial.println("Frame displayed successfully");
      }
    } else {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    }
    newFrameAvailable = false;
  }

  // Small delay to prevent watchdog reset
  delay(10);
}

void bluetoothTask(void* pvParameters) {
  while (1) {
    if (Serial.available()) {
      SerialBT.write(Serial.read());
    }

    if (SerialBT.available()) {
      receivedData = SerialBT.readStringUntil('\n');
      newFrameAvailable = true;
    }

    delay(20);  // Small delay to prevent task starvation
  }
}