//#include "WiFi.h"       //Uncomment for ESP32
#include <ESP8266WiFi.h>   //Comment for ESP-32
#include <PubSubClient.h>  // Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>

#define SSID_NAME "Wifi-name"                      // Your Wifi Network name
#define SSID_PASSWORD "Wifi-password"              // Your Wifi network password
#define MQTT_BROKER "smartnest.cz"                 // Broker host
#define MQTT_PORT 1883                             // Broker port
#define MQTT_USERNAME "username"                   // Username from Smartnest
#define MQTT_PASSWORD "password"                   // Password from Smartnest (or API key)
#define MQTT_CLIENT "device-Id"                    // Device Id from smartnest
#define FIRMWARE_VERSION "Example-airConditioner"  // Custom name for this program

WiFiClient espClient;
PubSubClient client(espClient);
int airConditionerPin = 2;
int sensorpin = A0;

char msg[5];
double temp = 25;
double setpoint = 25;
double lastTimeSent = 0;

void startWifi();
void startMqtt();
void checkMqtt();
void mqttLoop();
int splitTopic(char* topic, char* tokens[], int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void turnOn();
void turnOff();
void setSetpoint(double temp);
void reportTemperature(double temp);
void sendToBroker(char* topic, char* message);

void setup() {
	pinMode(airConditionerPin, OUTPUT);
	pinMode(sensorpin, INPUT);
	Serial.begin(115200);
	startWifi();
	startMqtt();
}

void loop() {
	client.loop();
	checkMqtt();
	reportTemperature(temp);
}

void callback(char* topic, byte* payload, unsigned int length) {  //A new message has been received

	int tokensNumber = 10;
	char message[length + 1];
	char* tokens[tokensNumber];

	Serial.print("MQTT Message Received. Topic:");
	Serial.println(topic);
	Serial.print("Message:");
	Serial.println(message);

	splitTopic(topic, tokens, tokensNumber);

	sprintf(message, "%c", (char)payload[0]);
	for (int i = 1; i < length; i++) {
		sprintf(message, "%s%c", message, (char)payload[i]);
	}

	//------------------Decide what to do depending on the topic and message---------------------------------

	if (strcmp(tokens[1], "directive") == 0 && strcmp(tokens[2], "powerState") == 0) {  // Turn On or OFF

		if (strcmp(message, "ON") == 0) {
			turnOn();
		} else if (strcmp(message, "OFF") == 0) {
			turnOff();
		}

	} else if (strcmp(tokens[1], "directive") == 0 && strcmp(tokens[2], "setpoint") == 0) {  // Set Setpoint Temperature

		double value = atof(message);
		if (isnan(value)) {
			setpoint = 0;
		}

		setSetpoint(value);

		sprintf(topic, "%s/report/setpoint", MQTT_CLIENT);
		client.publish(topic, message);
	}
}

void startWifi() {
	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID_NAME, SSID_PASSWORD);
	Serial.println("Connecting ...");

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println('\n');
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address:\t");
	Serial.println(WiFi.localIP());
	delay(500);
}

void startMqtt() {
	client.setServer(MQTT_BROKER, MQTT_PORT);
	client.setCallback(callback);

	while (!client.connected()) {
		Serial.println("Connecting to MQTT...");
		if (client.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD)) {
			Serial.println("connected");
		} else {
			if (client.state() == 5) {
				Serial.println("Connection not allowed by broker, possible reasons:");
				Serial.println("- Device is already online. Wait some seconds until it appears offline");
				Serial.println("- Wrong Username or password. Check credentials");
				Serial.println("- Client Id does not belong to this username, verify ClientId");

			} else {
				Serial.println("Not possible to connect to Broker Error code:");
				Serial.print(client.state());
			}

			delay(0x7530);
		}
	}

	char subscibeTopic[100];
	sprintf(subscibeTopic, "%s/#", MQTT_CLIENT);
	client.subscribe(subscibeTopic);  //Subscribes to all messages send to the device

	sendToBroker("report/online", "true");  // Reports that the device is online
	delay(100);
	sendToBroker("report/firmware", FIRMWARE_VERSION);  // Reports the firmware version
	delay(100);
	sendToBroker("report/ip", (char*)WiFi.localIP().toString().c_str());  // Reports the ip
	delay(100);
	sendToBroker("report/network", (char*)WiFi.SSID().c_str());  // Reports the network name
	delay(100);

	char signal[5];
	sprintf(signal, "%d", WiFi.RSSI());
	sendToBroker("report/signal", signal);  // Reports the signal strength
	delay(100);
}

int splitTopic(char* topic, char* tokens[], int tokensNumber) {
	const char s[2] = "/";
	int pos = 0;
	tokens[0] = strtok(topic, s);
	while (pos < tokensNumber - 1 && tokens[pos] != NULL) {
		pos++;
		tokens[pos] = strtok(NULL, s);
	}
	return pos;
}

void checkMqtt() {
	if (!client.connected()) {
		startMqtt();
	}
}

void turnOff() {
	digitalWrite(airConditionerPin, HIGH);
	sendToBroker("report/powerState", "OFF");
}

void turnOn() {
	digitalWrite(airConditionerPin, LOW);
	sendToBroker("report/powerState", "ON");
}

void setSetpoint(double temp) {
	Serial.printf("setpoint changed to %3.2f", temp);
	//Do something with the value, turn on AC?
}

void reportTemperature(double temp) {
	if (millis() - lastTimeSent > 30000 && client.connected()) {  //Makes sure not to send reports too often
		sprintf(msg, "%d", temp);
		sendToBroker("report/temperature", msg);
		lastTimeSent = millis();
	}
}

void sendToBroker(char* topic, char* message) {
	if (client.connected()) {
		char topicArr[100];
		sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
		client.publish(topicArr, message);
	}
}
