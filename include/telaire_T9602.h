#ifndef TELAIRE_T9602_H
#define TELAIRE_T9602_H

#include <Arduino.h>
#include <Wire.h>

class T9602 {
public:
    T9602();
    void begin(uint8_t ADR_ = 0x28);
    void updateMeasurements();
    float getHumidity();
    float getTemperature();
    String getString(bool takeNewReadings = false);
    String getHeader();

private:
    uint8_t ADR;
    float RH;
    float Temp;
};

#endif // TELAIRE_T9602_H
