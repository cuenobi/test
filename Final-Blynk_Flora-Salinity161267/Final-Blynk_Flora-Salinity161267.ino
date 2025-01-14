#define BLYNK_TEMPLATE_ID "TMPL6vFEE1Lv0"
#define BLYNK_TEMPLATE_NAME "Mini Salinity"
#define BLYNK_AUTH_TOKEN "GTGohtzQSQoPOVn5_cn3xOjpMSHdd0VN"

#define BLYNK_PRINT Serial // Defines the object that is used for printing
#define BLYNK_DEBUG        // Optional, this enables more detailed prints
#include "BLEDevice.h"
#define FLORA_ADDR "5C:85:7E:B1:1A:A7"
#define BLYNK_NO_BUILTIN   // Disable built-in analog & digital pin operations
#include <TimeLib.h>
#include "WiFi.h"
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

BLEClient* pClient;

//********** WIFI Credentials **********//
char ssid[] = "ucom2";
char pass[] = "qqqqqqqqq9";

//********** BLE Service & Characteristic UUIDs **********//
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");
static BLEAddress floraAddress(FLORA_ADDR);

// Global Variables
static BLERemoteCharacteristic* pRemoteCharacteristic;
float temp = 0;
int moisture = 0;
int light = 0;
int conductivity = 0;
int battery = 0;
float salinity = 0;

void getSensorData(BLEAddress pAddress) {
  btStart();
  Serial.print("Connecting to Flora device ");
  Serial.println(pAddress.toString().c_str());

  if (!pClient->connect(pAddress)) {
    Serial.println("Failed to connect. Restarting...");
    pClient->disconnect();
    ESP.restart();
  }

  Serial.println("Connected to Flora");

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find our service UUID");
    ESP.restart();
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
  uint8_t buf[2] = {0xA0, 0x1F}; 
  pRemoteCharacteristic->writeValue(buf, 2, true);

  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
  std::string value = pRemoteCharacteristic->readValue().c_str();

  if (value.length() < 16) {
    Serial.println("Error: BLE data length is less than 16 bytes");
    ESP.restart();
  }

  temp = (value[0] + value[1] * 256) / 10.0; 
  moisture = value[7];
  light = value[3] + value[4] * 256; 
  conductivity = value[8] + value[9] * 256; 
  battery = value[15];

  if (conductivity > 0) {
    salinity = conductivity * 0.5;
  } else {
    salinity = 0;
  }

  // Show data in Serial Monitor
  printf("\nHELLO WORLD DATA:");
  printf("Temperature: "); printf(temp);
  printf("Moisture: "); printf(moisture);
  printf("Light: "); printf(light);
  printf("Conductivity (EC): "); printf(conductivity);
  printf("Battery: "); printf(battery);
  printf("Salinity: "); printf(salinity);

  pClient->disconnect();
  btStop();
  delay(500);
  sendSensorData();
}

void sendSensorData() {
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.connect();
  
  Blynk.virtualWrite(V6, salinity);
  Blynk.virtualWrite(V4, conductivity);
  Blynk.virtualWrite(V5, temp);
  Blynk.virtualWrite(V7, light);
  Blynk.virtualWrite(V9, battery);

  delay(500);
  Blynk.disconnect();
  WiFi.mode(WIFI_OFF);
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // Set deep sleep timer
  delay(500);
  esp_deep_sleep_start(); // Put ESP32 into deep sleep
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Flora client...");
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  delay(500);
  getSensorData(floraAddress);
}

void loop() {
  // The system goes into deep sleep, so no code here
}
