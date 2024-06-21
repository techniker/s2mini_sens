#include "telaire_T9602.h"

// Constructor
T9602::T9602() {
    pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED pin as an output
    ADR = 0x28; // Default I2C address
    RH = -9999;
    Temp = -9999;
}

void T9602::begin(uint8_t ADR_) {
    ADR = ADR_;
    Wire.begin();
}

void T9602::updateMeasurements() {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on by making the voltage LOW
    uint8_t data[4] = {0};
    Wire.beginTransmission(ADR);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(ADR, 4);
    for (int i = 0; i < 4; ++i) {
        data[i] = Wire.read();
    }
    RH = static_cast<float>(((data[0] & 0x3F) << 8) + data[1]) / 16384.0 * 100.0;
    Temp = static_cast<float>((static_cast<unsigned>(data[2]) * 64 + (static_cast<unsigned>(data[3]) >> 2)) / 16384.0 * 165.0 - 40.0);
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
}

float T9602::getHumidity() {
    return RH;
}

float T9602::getTemperature() {
    return Temp;
}

String T9602::getString(bool takeNewReadings) {
    if (takeNewReadings) {
        updateMeasurements();
    }
    // Adjusted format to "Temp: value; Humidity: value"
    return "Temp: " + String(Temp) + "; Humidity: " + String(RH);
}

String T9602::getHeader() {
    return "Relative Humidity [%],Temp [C],";
}
