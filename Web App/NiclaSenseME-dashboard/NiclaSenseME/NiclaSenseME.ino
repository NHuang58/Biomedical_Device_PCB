/*
  Arduino Nicla Sense ME - simplified BLE sketch for MongoDB data logging

  Streams four fields to the paired web dashboard, which persists them
  to MongoDB: temperature, accelerometer (IMU), gyroscope, and an
  analog reading from pin A1.

  Hardware required: https://store.arduino.cc/nicla-sense-me
*/

#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>

#define BLE_SENSE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")

const int VERSION = 0x00000001;

BLEService service(BLE_SENSE_UUID("0000"));

BLEUnsignedIntCharacteristic versionCharacteristic(BLE_SENSE_UUID("1001"), BLERead);
BLEFloatCharacteristic temperatureCharacteristic(BLE_SENSE_UUID("2001"), BLERead);
BLECharacteristic accelerometerCharacteristic(BLE_SENSE_UUID("5001"), BLERead | BLENotify, 3 * sizeof(float));  // Array of 3x float, XYZ
BLECharacteristic gyroscopeCharacteristic(BLE_SENSE_UUID("6001"), BLERead | BLENotify, 3 * sizeof(float));    // Array of 3x float, XYZ
BLEUnsignedIntCharacteristic analogA1Characteristic(BLE_SENSE_UUID("A001"), BLERead);

String name;

Sensor temperature(SENSOR_ID_TEMP);
SensorXYZ gyroscope(SENSOR_ID_GYRO);
SensorXYZ accelerometer(SENSOR_ID_ACC);

void setup() {
  Serial.begin(115200);

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(red);

  pinMode(A1, INPUT);

  BHY2.begin(NICLA_STANDALONE);
  temperature.begin();
  // High rate sensors configured to 200Hz with 0ms latency for immediate delivery
  gyroscope.begin(200, 0);
  accelerometer.begin(200, 0);

  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1)
      ;
  }

  String address = BLE.address();
  address.toUpperCase();
  name = "BLESense-" + address.substring(address.length() - 5, address.length() - 3) + address.substring(address.length() - 2);

  BLE.setLocalName(name.c_str());
  BLE.setDeviceName(name.c_str());
  BLE.setAdvertisedService(service);

  service.addCharacteristic(versionCharacteristic);
  service.addCharacteristic(temperatureCharacteristic);
  service.addCharacteristic(accelerometerCharacteristic);
  service.addCharacteristic(gyroscopeCharacteristic);
  service.addCharacteristic(analogA1Characteristic);

  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureCharacteristicRead);
  analogA1Characteristic.setEventHandler(BLERead, onAnalogA1CharacteristicRead);

  versionCharacteristic.setValue(VERSION);

  BLE.addService(service);
  BLE.advertise();
}

void loop() {
  // BHY2.update() must run as fast as possible to clear the sensor hub FIFO
  BHY2.update();

  if (BLE.connected()) {
    if (accelerometerCharacteristic.subscribed()) {
      float accelerometerValues[3] = { accelerometer.x(), accelerometer.y(), accelerometer.z() };
      accelerometerCharacteristic.writeValue(accelerometerValues, sizeof(accelerometerValues));
    }

    if (gyroscopeCharacteristic.subscribed()) {
      float gyroscopeValues[3] = { gyroscope.x(), gyroscope.y(), gyroscope.z() };
      gyroscopeCharacteristic.writeValue(gyroscopeValues, sizeof(gyroscopeValues));
    }
  }
}

void blePeripheralConnectHandler(BLEDevice central) {
  nicla::leds.setColor(green);
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  nicla::leds.setColor(red);
}

void onTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  temperatureCharacteristic.writeValue(temperature.value());
}

void onAnalogA1CharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  analogA1Characteristic.writeValue((uint32_t)analogRead(A1));
}
