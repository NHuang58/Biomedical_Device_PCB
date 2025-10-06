#include "Arduino_BHY2.h"

  Sensor temperature(SENSOR_ID_TEMP);
  float temperatureValue = 0;

  void setup(){
    Serial.begin(115200);
    BHY2.begin();
    temperature.begin();
  }

  void loop(){
    BHY2.update();
    temperatureValue = temperature.value();

    Serial.print("Temperature :");
    Serial.println(temperatureValue);
    delay(500);
  }