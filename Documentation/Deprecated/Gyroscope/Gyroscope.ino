#include <Arduino_BHY2.h>

SensorXYZ gyroscope(SENSOR_ID_GYRO);

void setup() {
  Serial.begin(115200);
  BHY2.begin();
  gyroscope.begin();
}

void loop() {
  BHY2.update();

  int16_t valueX = gyroscope.x();
  int16_t valueY = gyroscope.y();
  int16_t valueZ = gyroscope.z();

  // Print all values in one line
  Serial.println(gyroscope.toString());

  // Print individual values
  Serial.print("  X: "); Serial.print(valueX);
  Serial.print("  Y: "); Serial.print(valueY);
  Serial.print("  Z: "); Serial.println(valueZ);

  delay(400); // add small delay for readability
}
