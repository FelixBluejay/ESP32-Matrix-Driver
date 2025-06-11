#include <FastLED.h>
#include <BluetoothSerial.h>

#define DATA_PIN 4
#define NUM_LEDS 256
CRGB leds[NUM_LEDS];

BluetoothSerial SerialBT;
String deviceName = "ESP32 Matrix Controller";

volatile bool newDataAvailable = false;
String receivedData = "";

// Task handles
TaskHandle_t bluetoothTaskHandle = NULL;

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

void loop() {

  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(500);

  if (newDataAvailable) {
    Serial.print("Received: ");
    Serial.println(receivedData);
    newDataAvailable = false;
  }
}

void bluetoothTask(void *pvParameters) {
  while (1) {
    // Forward data between Serial and Bluetooth
    if (Serial.available()) {
      SerialBT.write(Serial.read());
    }

    if (SerialBT.available()) {
      receivedData = SerialBT.readStringUntil('\n');
      newDataAvailable = true;
      // Serial.write(SerialBT.read());
      // Serial.write(receivedData);
    }

    delay(20);  // Small delay to prevent task starvation
  }
}