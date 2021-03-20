#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>

#define SSID_NAME "Wifi-name"                    // Your Wifi Network name
#define SSID_PASSWORD "Wifi-password"            // Your Wifi network password
#define MQTT_BROKER "smartnest.cz"               // Broker host
#define MQTT_PORT 1883                           // Broker port
#define MQTT_USERNAME "username"                 // Username from Smartnest
#define MQTT_PASSWORD "password"                 // Password from Smartnest (or API key)
#define MQTT_CLIENT "device-Id"                  // Device Id from smartnest
#define FIRMWARE_VERSION "Tutorial-RelayModule"  // Custom name for this program

WiFiClient espClient;
PubSubClient client(espClient);

int switchPin1 = 0;
int switchPin2 = 3;
int switchPin3 = 5;
int switchPin4 = 4;

void startWifi();
void startMqtt();
void checkMqtt();
int splitTopic(char* topic, char* tokens[], int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);
void sendToBroker(char* topic, char* message);

void turnOff(int pin);
void turnOn(int pin);

void setup() {
	pinMode(switchPin1, OUTPUT);
	pinMode(switchPin2, OUTPUT);
	pinMode(switchPin3, OUTPUT);
	pinMode(switchPin4, OUTPUT);
	Serial.begin(115200);
	startWifi();
	startMqtt();
}

void loop() {
	client.loop();
	checkMqtt();
}

void callback(char* topic, byte* payload, unsigned int length) {  //A new message has been received
	Serial.print("Topic:");
	Serial.println(topic);
	int tokensNumber = 10;
	char* tokens[tokensNumber];
	char message[length + 1];
	splitTopic(topic, tokens, tokensNumber);
	sprintf(message, "%c", (char)payload[0]);
	for (int i = 1; i < length; i++) {
		sprintf(message, "%s%c", message, (char)payload[i]);
	}
	Serial.print("Message:");
	Serial.println(message);

	//------------------ACTIONS HERE---------------------------------

	if (strcmp(tokens[1], "directive") == 0) {
		if (strcmp(tokens[2], "powerState1") == 0) {
			if (strcmp(message, "ON") == 0) {
				turnOn(1);
			} else if (strcmp(message, "OFF") == 0) {
				turnOff(1);
			}
		} else if (strcmp(tokens[2], "powerState2") == 0) {
			if (strcmp(message, "ON") == 0) {
				turnOn(2);
			} else if (strcmp(message, "OFF") == 0) {
				turnOff(2);
			}
		} else if (strcmp(tokens[2], "powerState3") == 0) {
			if (strcmp(message, "ON") == 0) {
				turnOn(3);
			} else if (strcmp(message, "OFF") == 0) {
				turnOff(3);
			}
		} else if (strcmp(tokens[2], "powerState4") == 0) {
			if (strcmp(message, "ON") == 0) {
				turnOn(4);
			} else if (strcmp(message, "OFF") == 0) {
				turnOff(4);
			}
		}
	}
}

void startWifi() {
	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID_NAME, SSID_PASSWORD);
	Serial.println("Connecting ...");
	int attempts = 0;
	while (WiFi.status() != WL_CONNECTED && attempts < 10) {
		attempts++;
		delay(500);
		Serial.print(".");
	}

	if (WiFi.status() == WL_CONNECTED) {
		Serial.println('\n');
		Serial.print("Connected to ");
		Serial.println(WiFi.SSID());
		Serial.print("IP address:\t");
		Serial.println(WiFi.localIP());

	} else {
		Serial.println('\n');
		Serial.println('I could not connect to the wifi network after 10 attempts \n');
	}

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
				Serial.println("- Device is already online. Wait some seconds until it appears offline for the broker");
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

void sendToBroker(char* topic, char* message) {
	if (client.connected()) {
		char topicArr[100];
		sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
		client.publish(topicArr, message);
	}
}

void turnOff(int pin) {
	Serial.printf("Turning off pin %d...\n", pin);
	switch (pin) {
	case 1:
		digitalWrite(switchPin1, LOW);
		sendToBroker("report/powerState1", "OFF");
		break;
	case 2:
		digitalWrite(switchPin2, LOW);
		sendToBroker("report/powerState2", "OFF");
		break;
	case 3:
		digitalWrite(switchPin3, LOW);
		sendToBroker("report/powerState3", "OFF");
		break;
	case 4:
		digitalWrite(switchPin4, LOW);
		sendToBroker("report/powerState4", "OFF");
		break;
	}
}

void turnOn(int pin) {
	Serial.printf("Turning on pin %d...\n", pin);
	switch (pin) {
	case 1:
		digitalWrite(switchPin1, HIGH);
		sendToBroker("report/powerState1", "ON");
		break;
	case 2:
		digitalWrite(switchPin2, HIGH);
		sendToBroker("report/powerState2", "ON");
		break;
	case 3:
		digitalWrite(switchPin3, HIGH);
		sendToBroker("report/powerState3", "ON");
		break;
	case 4:
		digitalWrite(switchPin4, HIGH);
		sendToBroker("report/powerState4", "ON");
		break;
	}
}
