#include <FastLED.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>

#define DATA_PIN 4
#define NUM_LEDS 256
CRGB leds[NUM_LEDS];

BluetoothSerial SerialBT;
String deviceName = "ESP32 Matrix Controller";

// Buffer for incoming data
const size_t BUFFER_SIZE = 8192;  // Increased buffer size
char serialBuffer[BUFFER_SIZE];
size_t bufferPos = 0;

DynamicJsonDocument doc(16384);

void setup() {
  Serial.begin(115200);
  SerialBT.begin(deviceName);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);
  
  // Clear buffer
  memset(serialBuffer, 0, BUFFER_SIZE);
}

int getFlippedIndex(int x, int y) {
  if (y % 2 == 0) {
    return (y + 1) * 16 - 1 - x;  // Even rows: right-to-left
  } else {
    return y * 16 + x;  // Odd rows: left-to-right
  }
}

void displayFrame(JsonArray frameData) {
  for (int y = 0; y < 16; y++) {
    if (y >= frameData.size()) break;  // Safety check
    
    JsonArray row = frameData[y];
    for (int x = 0; x < 16; x++) {
      if (x >= row.size()) break;  // Safety check
      
      JsonArray pixel = row[x];
      if (pixel.size() >= 3) {  // Ensure we have RGB values
        uint8_t r = pixel[0];
        uint8_t g = pixel[1];
        uint8_t b = pixel[2];
        leds[getFlippedIndex(x, y)] = CRGB(r, g, b);
      }
    }
  }
  FastLED.show();
}

void processIncomingData() {
  // Check if we have a complete JSON object (starts with { and ends with })
  char* start = strchr(serialBuffer, '{');
  char* end = strrchr(serialBuffer, '}');
  
  if (start && end && start < end) {
    // Calculate length of the JSON object
    size_t jsonLength = end - start + 1;
    
    // Copy the JSON object to a temporary buffer for parsing
    char jsonBuffer[jsonLength + 1];
    strncpy(jsonBuffer, start, jsonLength);
    jsonBuffer[jsonLength] = '\0';
    
    Serial.println("Received JSON:");
    Serial.println(jsonBuffer);
    
    // Parse JSON
    DeserializationError error = deserializeJson(doc, jsonBuffer);
    if (!error) {
      if (doc.containsKey("cmd") && strcmp(doc["cmd"], "displayFrame") == 0) {
        if (doc.containsKey("frameData")) {
          JsonArray frameData = doc["frameData"];
          Serial.print("Size: ");
          Serial.println(frameData.size());
          displayFrame(frameData);
          Serial.println("Frame displayed successfully");
        }
      }
    } else {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    }
    
    // Remove processed data from buffer
    memmove(serialBuffer, end + 1, bufferPos - (end - serialBuffer + 1));
    bufferPos -= (end - serialBuffer + 1);
    serialBuffer[bufferPos] = '\0';
  } else if (bufferPos >= BUFFER_SIZE - 1) {
    // Buffer full but no complete JSON found - reset buffer
    Serial.println("Buffer full but no complete JSON found. Resetting buffer.");
    bufferPos = 0;
    serialBuffer[0] = '\0';
  }
}

void loop() {
  // Read available data from Bluetooth
  while (SerialBT.available()) {
    if (bufferPos < BUFFER_SIZE - 1) {
      serialBuffer[bufferPos++] = SerialBT.read();
      serialBuffer[bufferPos] = '\0';  // Null-terminate
    } else {
      // Buffer overflow - discard oldest byte
      memmove(serialBuffer, serialBuffer + 1, BUFFER_SIZE - 2);
      bufferPos--;
      serialBuffer[bufferPos++] = SerialBT.read();
      serialBuffer[bufferPos] = '\0';
    }
  }
  
  // Process any complete JSON objects in the buffer
  processIncomingData();
  
  delay(1);  // Small delay to prevent watchdog reset
}