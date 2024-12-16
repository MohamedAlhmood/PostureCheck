#include <ArduinoBLE.h>
#include "Arduino_BMI270_BMM150.h"

// BLE Service and Characteristic UUIDs
#define VALUE_SIZE 20
BLEService postureService("00000000-5EC4-4083-81CD-A10B8D5CF6EC"); // Service UUID
BLECharacteristic postureCharacteristic("00000001-5EC4-4083-81CD-A10B8D5CF6EC", BLERead | BLENotify, VALUE_SIZE); // Characteristic UUID

// Define stricter thresholds for detecting bad posture
const float Z_AXIS_STRICT_THRESHOLD = 0.7; // Minimum Z-axis acceleration for good posture
const float Y_AXIS_STRICT_THRESHOLD = 0.5; // Maximum Y-axis acceleration for good posture

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Started");

  // Initialize IMU sensor
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in G's");
  Serial.println("X\tY\tZ");

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("BLE-POSTURE"); // Advertised device name
  BLE.setDeviceName("BLE-POSTURE");
  
  // Add characteristic to the service
  postureService.addCharacteristic(postureCharacteristic);
  BLE.addService(postureService);
  
  // Start advertising
  BLE.advertise();
  Serial.println("Bluetooth® device active, waiting for connections...");
}

void loop() {
  // Wait for a BLE central device
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      checkPosture();
      delay(500); // Adjust sampling rate as needed
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void checkPosture() {
  float x, y, z;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    // Print acceleration values for debugging
    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);

    // Posture detection logic
    String postureStatus;
    if (z < Z_AXIS_STRICT_THRESHOLD || abs(y) > Y_AXIS_STRICT_THRESHOLD) {
      postureStatus = "⚠ Bad posture";
    } else {
      postureStatus = "✅ Good posture";
    }

    // Send posture status over BLE
    char buffer[VALUE_SIZE];
    int ret = snprintf(buffer, sizeof buffer, "%s", postureStatus.c_str());
    if (ret >= 0) {
      postureCharacteristic.writeValue(buffer);
    }

    // Print to Serial Monitor for debugging
    Serial.println(postureStatus);
  }
}