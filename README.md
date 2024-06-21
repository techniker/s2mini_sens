Telaire T9602 MQTT Sensor Node for WEMOS S2 Mini

A sensor node using the Telaire T9602 temperature and humidity sensor with a WEMOS S2 Mini board. 
The node connects to a WiFi network and publishes sensor data to an MQTT broker.
Additionally, it includes functionality to check sensor readings and restart in case of I2C failure.

Features

	•	Connects to a specified WiFi network.
	•	Publishes temperature and humidity readings to an MQTT broker.
	•	Validates sensor readings and retries up to three times if data is invalid.
	•	Power cycles the sensor in case of persistent invalid readings.
	•	Includes visual indicators for activity using LEDs.

Hardware Requirements

	•	WEMOS S2 Mini
	•	Telaire T9602 sensor
	•	Connections(important, public documentation is sometimes not correct!):
	•	I2C SCL -> IO 35
	•	I2C SDA -> IO 33