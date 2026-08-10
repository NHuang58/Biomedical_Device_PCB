#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>

#define BLE_SENSE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")

const int VERSION = 0x00000000;

// ========================================================================
// --- SENSOR CONFIGURATION ---
// Set your desired sample rate for each sensor here (in Hz).
// Example: 1.0f = 1 time per second. 50.0f = 50 times per second.
// ========================================================================

const float TEMP_HZ     = 1.0f;
const float HUMIDITY_HZ = 1.0f;
const float PRESSURE_HZ = 1.0f;
const float BSEC_HZ     = 1.0f;
const float GAS_HZ      = 1.0f;

const float GYRO_HZ     = 5.0f;   // e.g., 5 Hz
const float ACCEL_HZ    = 50.0f;  // e.g., 50 Hz
const float QUAT_HZ     = 50.0f;  // e.g., 50 Hz

// --- Custom biomedical sensors (added on top of the stock Nicla Sense ME hub) ---
// These are not part of the BHY2 sensor hub, so they are read directly from the
// PCB's analog front end. Update the pin numbers and calibration constants below
// to match the actual Rev. 6/7 wiring once the sensors are attached and calibrated.
const float HEART_RATE_HZ       = 1.0f;   // beats-per-minute readout rate
const float INTERNAL_TEMP_HZ    = 1.0f;   // onboard/enclosure temperature (distinct from ambient env temp above)
const float STRAIN_HZ           = 50.0f;  // strain gauge, sampled fast enough to catch impact events
const float FORCE_HZ            = 50.0f;  // force (derived from strain / charge amplifier), same cadence as strain

// --- BLE TRANSMISSION INTERVALS ---
// This automatically calculates how many milliseconds the loop should wait
// between sending Bluetooth packets based on the Hz you chose above.
const unsigned long GYRO_INTERVAL   = 1000 / GYRO_HZ;
const unsigned long ACCEL_INTERVAL  = 1000 / ACCEL_HZ;
const unsigned long QUAT_INTERVAL   = 1000 / QUAT_HZ;
const unsigned long STRAIN_INTERVAL = 1000 / STRAIN_HZ;
const unsigned long FORCE_INTERVAL  = 1000 / FORCE_HZ;

// Variables to track the last time we sent data
unsigned long lastGyroTime   = 0;
unsigned long lastAccelTime  = 0;
unsigned long lastQuatTime   = 0;
unsigned long lastStrainTime = 0;
unsigned long lastForceTime  = 0;

// --- Custom biomedical sensor pins ---
// TODO: confirm these against the Rev. 6/7 schematic before flashing to real hardware.
const int HEART_RATE_PIN    = A0; // analog front-end output for the heart-rate sensor
const int INTERNAL_TEMP_PIN = A1; // onboard thermistor / temp IC output
const int STRAIN_PIN        = A2; // AD7150 capacitance-to-digital / strain gauge output
const int FORCE_PIN         = A3; // charge amplifier output feeding the force sensor

// --- Custom biomedical sensor calibration ---
// TODO: replace these placeholder linear approximations with real calibration curves
// once the sensors are wired up (e.g., AD7150 datasheet conversion for strain, and a
// load-cell / charge-amplifier calibration sweep for force).
const float STRAIN_CAL_SLOPE  = 1.0f;   // raw ADC counts -> microstrain
const float STRAIN_CAL_OFFSET = 0.0f;
const float FORCE_CAL_SLOPE   = 0.05f;  // raw ADC counts -> Newtons
const float FORCE_CAL_OFFSET  = 0.0f;

// ========================================================================

BLEService service(BLE_SENSE_UUID("0000"));

BLEUnsignedIntCharacteristic versionCharacteristic(BLE_SENSE_UUID("1001"), BLERead);
BLEFloatCharacteristic temperatureCharacteristic(BLE_SENSE_UUID("2001"), BLERead);
BLEUnsignedIntCharacteristic humidityCharacteristic(BLE_SENSE_UUID("3001"), BLERead);
BLEFloatCharacteristic pressureCharacteristic(BLE_SENSE_UUID("4001"), BLERead);

BLECharacteristic accelerometerCharacteristic(BLE_SENSE_UUID("5001"), BLERead | BLENotify, 3 * sizeof(float));
BLECharacteristic gyroscopeCharacteristic(BLE_SENSE_UUID("6001"), BLERead | BLENotify, 3 * sizeof(float));
BLECharacteristic quaternionCharacteristic(BLE_SENSE_UUID("7001"), BLERead | BLENotify, 4 * sizeof(float));

BLECharacteristic rgbLedCharacteristic(BLE_SENSE_UUID("8001"), BLERead | BLEWrite, 3 * sizeof(byte));

BLEFloatCharacteristic bsecCharacteristic(BLE_SENSE_UUID("9001"), BLERead);
BLEIntCharacteristic  co2Characteristic(BLE_SENSE_UUID("9002"), BLERead);
BLEUnsignedIntCharacteristic gasCharacteristic(BLE_SENSE_UUID("9003"), BLERead);

// --- Custom biomedical characteristics ---
// Heart rate and internal temperature change slowly, so like temperature/humidity/pressure
// above they are simple BLERead characteristics refreshed whenever the central polls them.
BLEUnsignedIntCharacteristic heartRateCharacteristic(BLE_SENSE_UUID("a001"), BLERead);
BLEFloatCharacteristic internalTemperatureCharacteristic(BLE_SENSE_UUID("a002"), BLERead);
// Strain and force can spike quickly (impact events), so like accelerometer/gyroscope
// above they stream via BLENotify on a fixed interval from loop().
BLEFloatCharacteristic strainCharacteristic(BLE_SENSE_UUID("a003"), BLERead | BLENotify);
BLEFloatCharacteristic forceCharacteristic(BLE_SENSE_UUID("a004"), BLERead | BLENotify);

String name;

Sensor temperature(SENSOR_ID_TEMP);
Sensor humidity(SENSOR_ID_HUM);
Sensor pressure(SENSOR_ID_BARO);
Sensor gas(SENSOR_ID_GAS);
SensorXYZ gyroscope(SENSOR_ID_GYRO);
SensorXYZ accelerometer(SENSOR_ID_ACC);
SensorQuaternion quaternion(SENSOR_ID_RV);
SensorBSEC bsec(SENSOR_ID_BSEC);

void setup(){
  Serial.begin(115200);
  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(red);

  // Initialize the BHY2 hub first
  BHY2.begin(NICLA_STANDALONE);

  // Initialize sensors with your custom rates
  temperature.begin(TEMP_HZ);
  humidity.begin(HUMIDITY_HZ);
  pressure.begin(PRESSURE_HZ);
  bsec.begin(BSEC_HZ);
  gas.begin(GAS_HZ);

  // High rate sensors configured with custom rate and 0ms latency
  gyroscope.begin(GYRO_HZ, 0);
  accelerometer.begin(ACCEL_HZ, 0);
  quaternion.begin(QUAT_HZ, 0);

  // Custom biomedical sensor pins (analog front end, not part of the BHY2 hub)
  pinMode(HEART_RATE_PIN, INPUT);
  pinMode(INTERNAL_TEMP_PIN, INPUT);
  pinMode(STRAIN_PIN, INPUT);
  pinMode(FORCE_PIN, INPUT);

  if (!BLE.begin()){
    Serial.println("Failed to initialized BLE!");
    while (1);
  }

  String address = BLE.address();
  address.toUpperCase();
  name = "BLESense-" + address.substring(address.length() - 5, address.length() - 3) + address.substring(address.length() - 2);

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
  service.addCharacteristic(heartRateCharacteristic);
  service.addCharacteristic(internalTemperatureCharacteristic);
  service.addCharacteristic(strainCharacteristic);
  service.addCharacteristic(forceCharacteristic);

  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureCharacteristicRead);
  humidityCharacteristic.setEventHandler(BLERead, onHumidityCharacteristicRead);
  pressureCharacteristic.setEventHandler(BLERead, onPressureCharacteristicRead);
  bsecCharacteristic.setEventHandler(BLERead, onBsecCharacteristicRead);
  co2Characteristic.setEventHandler(BLERead, onCo2CharacteristicRead);
  gasCharacteristic.setEventHandler(BLERead, onGasCharacteristicRead);
  rgbLedCharacteristic.setEventHandler(BLEWritten, onRgbLedCharacteristicWrite);
  heartRateCharacteristic.setEventHandler(BLERead, onHeartRateCharacteristicRead);
  internalTemperatureCharacteristic.setEventHandler(BLERead, onInternalTemperatureCharacteristicRead);

  versionCharacteristic.setValue(VERSION);
  BLE.addService(service);
  BLE.advertise();
}

void loop(){
  // BHY2.update() MUST run as fast as possible to clear the sensor hub FIFO
  BHY2.update();

  if (BLE.connected()){
    unsigned long currentMillis = millis();

    // Gyroscope
    if (gyroscopeCharacteristic.subscribed() && (currentMillis - lastGyroTime >= GYRO_INTERVAL)){
      lastGyroTime = currentMillis;
      float gyroscopeValues[3] = {gyroscope.x(), gyroscope.y(), gyroscope.z()};
      gyroscopeCharacteristic.writeValue(gyroscopeValues, sizeof(gyroscopeValues));
    }

    // Accelerometer
    if (accelerometerCharacteristic.subscribed() && (currentMillis - lastAccelTime >= ACCEL_INTERVAL)){
      lastAccelTime = currentMillis;
      float accelerometerValues[3] = {accelerometer.x(), accelerometer.y(), accelerometer.z()};
      accelerometerCharacteristic.writeValue(accelerometerValues, sizeof(accelerometerValues));
    }

    // Quaternion
    if(quaternionCharacteristic.subscribed() && (currentMillis - lastQuatTime >= QUAT_INTERVAL)){
      lastQuatTime = currentMillis;
      float quaternionValues[4] = {quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w()};
      quaternionCharacteristic.writeValue(quaternionValues, sizeof(quaternionValues));
    }

    // Strain
    if (strainCharacteristic.subscribed() && (currentMillis - lastStrainTime >= STRAIN_INTERVAL)){
      lastStrainTime = currentMillis;
      strainCharacteristic.writeValue(readStrain());
    }

    // Force
    if (forceCharacteristic.subscribed() && (currentMillis - lastForceTime >= FORCE_INTERVAL)){
      lastForceTime = currentMillis;
      forceCharacteristic.writeValue(readForce());
    }
  }
}

// --- Event Handlers ---
void blePeripheralDisconnectHandler(BLEDevice central){
  nicla::leds.setColor(green);
}

void onTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  temperatureCharacteristic.writeValue(temperature.value());
}

void onHumidityCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  uint8_t humidityValue = (uint8_t)(humidity.value() + 0.5f);
  humidityCharacteristic.writeValue(humidityValue);
}

void onPressureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  pressureCharacteristic.writeValue(pressure.value());
}

void onBsecCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  bsecCharacteristic.writeValue((float)bsec.iaq());
}

void onCo2CharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  co2Characteristic.writeValue((uint32_t)bsec.co2_eq());
}

void onGasCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  gasCharacteristic.writeValue((unsigned int)gas.value());
}

void onRgbLedCharacteristicWrite(BLEDevice central, BLECharacteristic characteristic){
  nicla::leds.setColor(rgbLedCharacteristic[0], rgbLedCharacteristic[1], rgbLedCharacteristic[2]);
}

void onHeartRateCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  heartRateCharacteristic.writeValue(readHeartRate());
}

void onInternalTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  internalTemperatureCharacteristic.writeValue(readInternalTemperature());
}

// --- Custom biomedical sensor readers ---
// Each of these is a placeholder analog read + linear approximation. Replace with the
// real sensor driver / calibration curve for the part actually populated on the PCB.

uint32_t readHeartRate(){
  // TODO: Replace with real pulse detection (peak-to-peak interval from the PPG/ECG
  // front end) once the heart-rate sensor is wired in. This placeholder just maps the
  // raw ADC reading into a plausible resting bpm range so the BLE characteristic and
  // web dashboard can be exercised end-to-end before the sensor is calibrated.
  int raw = analogRead(HEART_RATE_PIN);
  return (uint32_t)map(raw, 0, 4095, 50, 120);
}

float readInternalTemperature(){
  // TODO: Replace with the calibrated conversion for whichever internal temp sensor
  // (thermistor/IC) ends up on the PCB. Placeholder assumes a TMP36-style linear
  // analog output (500mV offset, 10mV/degC) on a 3.3V/12-bit ADC.
  int raw = analogRead(INTERNAL_TEMP_PIN);
  float voltage = raw * (3.3f / 4095.0f);
  return (voltage - 0.5f) * 100.0f;
}

float readStrain(){
  // TODO: Replace with an AD7150 capacitance-to-digital I2C read once the strain
  // gauge front end is wired in; this placeholder scales the raw ADC counts linearly.
  int raw = analogRead(STRAIN_PIN);
  return raw * STRAIN_CAL_SLOPE + STRAIN_CAL_OFFSET;
}

float readForce(){
  // TODO: Replace with the charge-amplifier calibration curve (raw ADC -> Newtons)
  // once the force sensor is calibrated against known loads.
  int raw = analogRead(FORCE_PIN);
  return raw * FORCE_CAL_SLOPE + FORCE_CAL_OFFSET;
}
