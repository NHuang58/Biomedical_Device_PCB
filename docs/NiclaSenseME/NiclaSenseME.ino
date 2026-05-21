/*
  Nicla Sense ME Firmware
  Optimized 200Hz Telemetry Streaming Channels Engine
*/

#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>

#define BLE_SENSE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")

const int VERSION = 0x00000000;

BLEService service(BLE_SENSE_UUID("0000"));

// All characteristics remain registered to avoid breaking GATT re-negotiation checks
BLEUnsignedIntCharacteristic versionCharacteristic(BLE_SENSE_UUID("1001"), BLERead);
BLEFloatCharacteristic       temperatureCharacteristic(BLE_SENSE_UUID("2001"), BLERead);
BLEUnsignedIntCharacteristic humidityCharacteristic(BLE_SENSE_UUID("3001"), BLERead);
BLEFloatCharacteristic       pressureCharacteristic(BLE_SENSE_UUID("4001"), BLERead);

BLECharacteristic            accelerometerCharacteristic(BLE_SENSE_UUID("5001"), BLERead | BLENotify, 3 * sizeof(float));
BLECharacteristic            gyroscopeCharacteristic(BLE_SENSE_UUID("6001"), BLERead | BLENotify, 3 * sizeof(float));
BLECharacteristic            quaternionCharacteristic(BLE_SENSE_UUID("7001"), BLERead | BLENotify, 4 * sizeof(float));

BLECharacteristic            rgbLedCharacteristic(BLE_SENSE_UUID("8001"), BLERead | BLEWrite, 3 * sizeof(byte));

BLEFloatCharacteristic       bsecCharacteristic(BLE_SENSE_UUID("9001"), BLERead);
BLEIntCharacteristic         co2Characteristic(BLE_SENSE_UUID("9002"), BLERead);
BLEUnsignedIntCharacteristic gasCharacteristic(BLE_SENSE_UUID("9003"), BLERead); 

String name;

Sensor           temperature(SENSOR_ID_TEMP);
Sensor           humidity(SENSOR_ID_HUM);
Sensor           pressure(SENSOR_ID_BARO);
Sensor           gas(SENSOR_ID_GAS);
SensorXYZ        gyroscope(SENSOR_ID_GYRO);
SensorXYZ        accelerometer(SENSOR_ID_ACC);
SensorQuaternion quaternion(SENSOR_ID_RV);
SensorBSEC       bsec(SENSOR_ID_BSEC);

void setup(){
  Serial.begin(115200);
  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(green);

  BHY2.begin(NICLA_STANDALONE);
  
  // High-performance frequency mode tracking settings injection
  temperature.begin();
  humidity.begin();
  pressure.begin();
  gyroscope.begin(200, 0);
  accelerometer.begin(200, 0);
  quaternion.begin(200, 0);
  bsec.begin();
  gas.begin();

  if (!BLE.begin()){
    Serial.println("Failled to initialized BLE!");
    while (1);
  }

  String address = BLE.address();
  address.toUpperCase();

  name = "NiclaSenseME-";
  name += address[address.length() - 5];
  name += address[address.length() - 4];
  name += address[address.length() - 2];
  name += address[address.length() - 1];

  BLE.setLocalName(name.c_str());
  BLE.setDeviceName(name.c_str());
  BLE.setAdvertisedService(service);

  service.addCharacteristic(temperatureCharacteristic);
  service.addCharacteristic(humidityCharacteristic);
  service.addCharacteristic(pressureCharacteristic);
  service.addCharacteristic(versionCharacteristic);
  service.addCharacteristic(accelerometerCharacteristic);
  service.addCharacteristic(gyroscopeCharacteristic);
  service.addCharacteristic(quaternionCharacteristic);
  service.addCharacteristic(bsecCharacteristic);
  service.addCharacteristic(co2Characteristic);
  service.addCharacteristic(gasCharacteristic);
  service.addCharacteristic(rgbLedCharacteristic);

  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureCharacteristicRead);
  humidityCharacteristic.setEventHandler(BLERead, onHumidityCharacteristicRead);
  pressureCharacteristic.setEventHandler(BLERead, onPressureCharacteristicRead);
  bsecCharacteristic.setEventHandler(BLERead, onBsecCharacteristicRead);
  co2Characteristic.setEventHandler(BLERead, onCo2CharacteristicRead);
  gasCharacteristic.setEventHandler(BLERead, onGasCharacteristicRead);
  rgbLedCharacteristic.setEventHandler(BLEWritten, onRgbLedCharacteristicWrite);

  versionCharacteristic.setValue(VERSION);
  BLE.addService(service);
  BLE.advertise();
}

void loop(){
  while (BLE.connected()){
    BHY2.update();

    if (gyroscopeCharacteristic.subscribed()){
      float gyroscopeValues[3] = {gyroscope.x(), gyroscope.y(), gyroscope.z()};
      gyroscopeCharacteristic.writeValue(gyroscopeValues, sizeof(gyroscopeValues));
    }

    if (accelerometerCharacteristic.subscribed()){
      float accelerometerValues[3] = {accelerometer.x(), accelerometer.y(), accelerometer.z()};
      accelerometerCharacteristic.writeValue(accelerometerValues, sizeof(accelerometerValues));
    }

    if(quaternionCharacteristic.subscribed()){
      float quaternionValues[4] = {quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w()};
      quaternionCharacteristic.writeValue(quaternionValues, sizeof(quaternionValues));
    }
  }
}

void blePeripheralDisconnectHandler(BLEDevice central){
  nicla::leds.setColor(red);
}

void onTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  temperatureCharacteristic.writeValue(temperature.value());
}

void onHumidityCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  uint8_t humidityValue = humidity.value() + 0.5f;
  humidityCharacteristic.writeValue(humidityValue);
}

void onPressureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  pressureCharacteristic.writeValue(pressure.value());
}

void onBsecCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  bsecCharacteristic.writeValue(float(bsec.iaq()));
}

void onCo2CharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  co2Characteristic.writeValue(bsec.co2_eq());
}

void onGasCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  gasCharacteristic.writeValue(gas.value());
}

void onRgbLedCharacteristicWrite(BLEDevice central, BLECharacteristic characteristic){
  nicla::leds.setColor(rgbLedCharacteristic[0], rgbLedCharacteristic[1], rgbLedCharacteristic[2]);
}