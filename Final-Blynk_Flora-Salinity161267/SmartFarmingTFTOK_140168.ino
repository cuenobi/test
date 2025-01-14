#define BLYNK_TEMPLATE_ID "TMPL6nrzDF3WG"
#define BLYNK_TEMPLATE_NAME "Smart Farming"
#define BLYNK_AUTH_TOKEN "-1JDhsoDqjpKDTNXKdcibu-VVhNj7vLq"

#define BLYNK_PRINT Serial
#include <BLEDevice.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Screen dimensions
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// BLE Definitions
#define FLORA_ADDR "5C:85:7E:B1:1B:20"
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");
static BLEAddress floraAddress(FLORA_ADDR);
BLEClient* pClient;



// TFT Display initialization
TFT_eSPI tft = TFT_eSPI(SCREEN_WIDTH, SCREEN_HEIGHT); // Initialize TFT display

// Blynk configuration
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "ucom2";
char pass[] = "qqqqqqqqq9";

// Sensor data variables
float temp = 26.0;       // Example temperature data
int moisture = 0;       // Example soil moisture data
int light = 300;         // Example light intensity data
int conductivity = 600;  // Example soil conductivity data
int battery = 90;        // Example battery level

void displaySensorData() {
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(2); // Rotate display orientation 90 degrees
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  // Header text
  tft.setCursor(60, 10);
  tft.printf("Smart Farming Data");

  // Display sensor data
  tft.setCursor(20, 50);
  tft.printf("Temp: %.1f C", temp);

  tft.setCursor(20, 80);
  tft.printf("Moisture: %d %%", moisture);

  tft.setCursor(20, 110);
  tft.printf("Light: %d lux", light);

  tft.setCursor(20, 140);
  tft.printf("Conductivity: %d uS/cm", conductivity);

  tft.setCursor(20, 170);
  tft.printf("Battery: %d %%", battery);
}

void displayGradientText() {
  static uint16_t color = 0; // Start with the first color in the palette

  // Clear the screen
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(2); // Rotate display orientation 90 degrees

  // Set dynamic text color
  tft.setTextColor(color);
  tft.setTextSize(3);

  // Display "SmartFarming" centered
  int x_center_smartfarming = (SCREEN_WIDTH - (12 * 12)) / 2; // Calculate center X for "SmartFarming"
  int y_center_smartfarming = SCREEN_HEIGHT / 2 - 40;        // Adjust Y for first line

  tft.setCursor(x_center_smartfarming, y_center_smartfarming);
  tft.println("SmartFarming");

  // Display "KMUTT" centered on a new line
  int x_center_kmutt = (SCREEN_WIDTH - (12 * 5)) / 2; // Calculate center X for "KMUTT"
  int y_center_kmutt = y_center_smartfarming + 40;    // Adjust Y for second line

  tft.setCursor(x_center_kmutt, y_center_kmutt);
  tft.println("KMUTT");

  // Update color (cycling through a palette)
  color += 100; // Increment the color value for gradient effect

  // Wrap around if the color exceeds the 16-bit range
  if (color > 0xFFFF) {
    color = 0;
  }

  delay(2000); // Delay for effect
}

void getSensorData(BLEAddress pAddress) {
  BLEDevice::init("");
  pClient = BLEDevice::createClient();

  if (!pClient->connect(pAddress)) {
    Serial.println("Failed to connect to Flora device");
    return;
  }

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find our service UUID");
    pClient->disconnect();
    return;
  }

  BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
  uint8_t buf[2] = {0xA0, 0x1F};
  pRemoteCharacteristic->writeValue(buf, 2, true);

  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("Failed to find our characteristic UUID");
    pClient->disconnect();
    return;
  }

  std::string value = pRemoteCharacteristic->readValue().c_str();
  const char* val = value.c_str();

  // ตรวจสอบความยาวข้อมูลก่อนประมวลผล
  if (value.length() >= 10) {
    temp = (val[0] + val[1] * 256) / 10.0;
    moisture = val[7];
    light = val[3] + val[4] * 256;
    conductivity = val[8] + val[9] * 256;
    battery = val[6];  // อ่านค่าปริมาณแบตเตอรี่จากตำแหน่งที่ถูกต้อง

    // ตรวจสอบและปรับค่า moisture
    if (moisture <= 0) {
      moisture = 1;  // ตั้งค่าขั้นต่ำ
    } else if (moisture > 100) {
      moisture = 100;  // จำกัดค่าไม่ให้เกิน 100
    }

    Serial.printf("Temp: %.1f C, Moisture: %d%%, Light: %d lux, Conductivity: %d uS/cm, Battery: %d%%\n", 
                   temp, moisture, light, conductivity, battery);
  } else {
    Serial.println("Invalid sensor data received");
  }

  pClient->disconnect();
}



void sendSensorData() {
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.connect();

  if (Blynk.connected()) {
    Blynk.virtualWrite(V5, temp);
    Blynk.virtualWrite(V6, moisture);
    Blynk.virtualWrite(V7, light);
    Blynk.virtualWrite(V8, conductivity);
    Blynk.virtualWrite(V9, battery);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Smart Farming...");

  // Initialize the TFT display
  tft.init();
  tft.setRotation(2); // Rotate display orientation 90 degrees

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  if (Blynk.connected()) {
    Serial.println("Blynk connected");
  } else {
    Serial.println("Blynk connection failed");
  }

  // Retrieve initial sensor data
  getSensorData(floraAddress);
}

void loop() {
  // Alternate between gradient text and sensor data
  displayGradientText();
  delay(2000); // Pause for effect

  displaySensorData();
  delay(5000); // Display sensor data for 5 seconds

  // Send sensor data to Blynk
  sendSensorData();

  // Run Blynk to keep connection active
  Blynk.run();
}



