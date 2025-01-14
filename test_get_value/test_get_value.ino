#include <BLEDevice.h>

// กำหนด MAC Address ของ Flora Sensor
const char* floraMacAddress = "5C:85:7E:B1:19:B6";

// UUID ของค่าต่างๆ ที่ต้องการอ่าน (ตัวอย่าง UUID)
#define UUID_SERVICE "0000FE95-0000-1000-8000-00805F9B34FB" // UUID ของ service
#define UUID_CHARACTERISTIC "00000002-0000-1000-8000-00805F9B34FB" // UUID ของข้อมูล

// ตัวแปรสำหรับเชื่อมต่อ BLE
BLEClient* pClient;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE client...");

  // เริ่มการทำงานของ BLE
  BLEDevice::init("");

  // พยายามเชื่อมต่อไปยัง Flora Sensor
  connectToFlora();
}

void loop() {
  if (pClient && pClient->isConnected()) {
    // อ่านค่าจากเซ็นเซอร์
    BLERemoteService* pService = pClient->getService(UUID_SERVICE);
    if (pService) {
      BLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic(UUID_CHARACTERISTIC);
      if (pCharacteristic) {
        std::string rawValue = pCharacteristic->readValue().c_str();
        String value = String(rawValue.c_str()); // แปลง std::string เป็น String ของ Arduino
        Serial.println("Raw Data: " + value);
        Serial.

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

  // แปลง MAC Address จาก char* เป็น uint8_t array
  uint8_t mac[6];
  sscanf(floraMacAddress, "%02X:%02X:%02X:%02X:%02X:%02X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

  pClient = BLEDevice::createClient();
  BLEAddress address(mac);  // ใช้ BLEAddress เพื่อสร้างอ็อบเจ็กต์ที่รับ mac เป็น uint8_t array
  if (pClient->connect(address)) {
    Serial.println("Connected!");
  } else {
    Serial.println("Failed to connect. Retrying...");
    delay(5000); // รอ 5 วินาที ก่อนเชื่อมต่อใหม่
  }
}
