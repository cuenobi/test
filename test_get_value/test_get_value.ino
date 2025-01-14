#include <NimBLEDevice.h>

// กำหนด MAC Address ของ Flora Sensor
// const char* floraMacAddress = "5C:85:7E:B1:1B:20";
const char* floraMacAddress = "5C:85:7E:B1:19:B6";

// UUID ของค่าต่างๆ ที่ต้องการอ่าน (ตัวอย่าง UUID)
#define UUID_SERVICE "00001204-0000-1000-8000-00805f9b34fb" // UUID ของ service
#define UUID_CHARACTERISTIC "00001a01-0000-1000-8000-00805f9b34fb" // UUID ของข้อมูล

// ตัวแปรสำหรับเชื่อมต่อ BLE
NimBLEClient* pClient;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE client...");

  // เริ่มการทำงานของ BLE
  NimBLEDevice::init("");

  // พยายามเชื่อมต่อไปยัง Flora Sensor
  connectToFlora();
}

void loop() {
  if (pClient && pClient->isConnected()) {
    // อ่านค่าจากเซ็นเซอร์
    NimBLERemoteService* pService = pClient->getService(UUID_SERVICE);
    if (pService) {
      NimBLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic(UUID_CHARACTERISTIC);
      if (pCharacteristic) {
        std::string value = pCharacteristic->readValue();
        Serial.println("Raw Data: " + value);

        // ตัวอย่างการแปลงข้อมูล
        if (value.length() >= 4) {
          float temperature = ((value[0] | (value[1] << 8)) / 10.0);
          float humidity = ((value[2] | (value[3] << 8)) / 10.0);
          Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
        }
      }
    }
    delay(2000); // อ่านข้อมูลทุก 2 วินาที
  } else {
    // พยายามเชื่อมต่อใหม่
    connectToFlora();
  }
}

void connectToFlora() {
  Serial.println("Connecting to Flora Sensor...");
  pClient = NimBLEDevice::createClient();
  if (pClient->connect(floraMacAddress)) {
    Serial.println("Connected!");
  } else {
    Serial.println("Failed to connect. Retrying...");
    delay(5000); // รอ 5 วินาที ก่อนเชื่อมต่อใหม่
  }
}
