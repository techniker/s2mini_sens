#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "telaire_T9602.h"
#include "telaire_T9602.cpp"
#include <Wire.h>
#include <esp_task_wdt.h>  // Include watchdog library

//WEMOS s2 mini board pins for i2c(was wrong in official documentation!): 
//SCL - IO 35
//SDA - IO 33

// WiFi credentials
const char* ssid = "";
const char* password = "";

// MQTT Broker
const char* mqtt_server = "";
const int mqtt_port = 1883;
// MQTT Broker credentials
const char* mqtt_user = "";
const char* mqtt_password = "";

WiFiClient espClient;
PubSubClient client(espClient);
T9602 telairesensor;

// Watchdog timeout in seconds
const int WDT_TIMEOUT = 30; // 30 seconds

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int wifi_attempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) { // Attempt to connect for 10 seconds max
        delay(500);
        Serial.print(".");
        wifi_attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi connection failed, rebooting...");
        ESP.restart(); // Reboot if WiFi fails to connect
    }
}

void reconnect_mqtt() {
    int mqtt_attempts = 0;
    while (!client.connected() && mqtt_attempts < 6) { // Attempt to connect for 30 seconds max
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect with MQTT username and password
        if (client.connect("Turm-Sens03", mqtt_user, mqtt_password)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
            mqtt_attempts++;
        }
    }

    if (!client.connected()) {
        Serial.println("MQTT connection failed, rebooting...");
        ESP.restart(); // Reboot if MQTT fails to connect
    }
}

void power_cycle_sensor() {
    Serial.println("Power cycling sensor...");
    pinMode(GPIO_NUM_17, OUTPUT);
    digitalWrite(GPIO_NUM_17, LOW); // Power off the sensor
    delay(1000);                    // Wait for a second
    digitalWrite(GPIO_NUM_17, HIGH); // Power on the sensor
    delay(1000);                    // Wait for the sensor to stabilize
    telairesensor.begin();          // Reinitialize the sensor
    Serial.println("Power cycle complete!");
}

bool validate_readings(float temperature, float humidity) {
    Serial.print("Validating readings - Temerature: ");
    Serial.print(temperature);
    Serial.print(", Humidity: ");
    Serial.println(humidity);
    
    // Check for out-of-range values or unrealistic readings
    bool temp_valid = (temperature >= -40.0 && temperature < 124.0);
    bool hum_valid = (humidity >= 0.0 && humidity < 99.0);
    
    Serial.print("Temperature valid: ");
    Serial.println(temp_valid);
    Serial.print("Humidity valid: ");
    Serial.println(hum_valid);
    
    return temp_valid && hum_valid;
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    pinMode(GPIO_NUM_12, OUTPUT);
    pinMode(GPIO_NUM_15, OUTPUT);
    pinMode(GPIO_NUM_4, OUTPUT);
    power_cycle_sensor(); 
    // Ensure the sensor is powered on

    // Initialize the watchdog timer
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL); // Add current thread to watchdog
}

void loop() {
    esp_task_wdt_reset(); // Reset watchdog timer

    if (WiFi.status() != WL_CONNECTED) {
        setup_wifi();
    }

    if (!client.connected()) {
        reconnect_mqtt();
    }

    client.loop();

    int retries = 3;
    bool valid_data = false;
    float temperature = 0.0;
    float humidity = 0.0;

    while (retries > 0 && !valid_data) {
        telairesensor.updateMeasurements();
        temperature = telairesensor.getTemperature();
        humidity = telairesensor.getHumidity();

        valid_data = validate_readings(temperature, humidity);
        if (!valid_data) {
            Serial.println("Invalid sensor data, retrying...");
            delay(1000);
        }
        retries--;
    }

    if (!valid_data) {
        Serial.println("Sensor data invalid after retries, power cycling sensor...");
        power_cycle_sensor();
        return; 
    // Skip this loop iteration and retry in the next loop
    }

    char tempStr[8];
    dtostrf(temperature, 6, 2, tempStr);
    char humStr[8];
    dtostrf(humidity, 6, 2, humStr);

    // Transmit only if the data is valid
    if (validate_readings(temperature, humidity)) {
        client.publish("/Turm-Sens03/temp", tempStr);
        client.publish("/Turm-Sens03/humidity", humStr);

    // Print the sensor data to the serial monitor
        Serial.print("Temperature: ");
        Serial.print(tempStr);
        Serial.print("; Humidity: ");
        Serial.println(humStr);
    } else {
        Serial.println("Invalid data, not transmitting.");
    }

    digitalWrite(GPIO_NUM_15, HIGH); // LED Indicate activity
    digitalWrite(GPIO_NUM_4, HIGH);
    delay(100);
    digitalWrite(GPIO_NUM_15, LOW);
    digitalWrite(GPIO_NUM_4, LOW);

    delay(1000); // Delay before next sensor reading
}