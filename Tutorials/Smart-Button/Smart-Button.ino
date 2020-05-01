#include <ESP8266WiFi.h>
#include <PubSubClient.h>// Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>

#define SSID_NAME "Wifi-name"                   // Your Wifi Network name
#define SSID_PASSWORD "Wifi-password"           // Your Wifi network password
#define MQTT_BROKER "smartnest.cz"              // Broker host
#define MQTT_PORT 1883                          // Broker port
#define MQTT_USERNAME "username"                // Username from Smartnest
#define MQTT_PASSWORD "password"                // Password from Smartnest (or API key)
#define MQTT_CLIENT "device-Id"                 // Device Id from smartnest

WiFiClient espClient;
PubSubClient client(espClient); 

void startWifi();
void startMqtt();
void checkMqtt();
int splitTopic(char* topic, char* tokens[] ,int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.println("Starting...");
  Serial.begin(115200);
  startWifi();                                              // Tries to connect to wifi 10 attempts
  
  if(WiFi.status() == WL_CONNECTED){                        // If it is connected to wifi connect to MQTT broker

    startMqtt();                  
    char report[100];

    sprintf(report,"%s/report/online",MQTT_CLIENT);         // Reports that the device is online
    client.publish(report, "true");
    delay(200);
    
    sprintf(report,"%s/report/detectionState",MQTT_CLIENT); // Reports that the button press was detected (closed and opened) Make sure to select open in the trigger from Alexa
    client.publish(report, "true");
    delay(200);
    client.publish(report, "false");
    delay(200);

    sprintf(report,"%s/report/online",MQTT_CLIENT);         // Reports that the device is offline 
    client.publish(report, "false");
    delay(200);
    
    Serial.println("Report sent to broker, now going back to sleep.");
    
  } else {
    Serial.println("Connection to wifi was unsuccessful. going back to sleep");
  }
  
  ESP.deepSleep(0);

}

void loop() {

  // Empty Loop

}

void callback(char* topic, byte* payload, unsigned int length) { // This function runs when there is a new message in the subscribed topic
  Serial.print("Topic:");
  Serial.println(topic);

  // Splits the topic and gets the message
  int tokensNumber=10;
  char *tokens[tokensNumber];
  char message[length+1];
  splitTopic(topic, tokens, tokensNumber);
  sprintf(message,"%c",(char)payload[0]);             

  for (int i = 1; i < length; i++) {
    sprintf(message,"%s%c",message,(char)payload[i]);
  }

  Serial.print("Message:");
  Serial.println(message);
}


void startWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  Serial.println("Connecting ...");
  int attempts  = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10 ) {
    attempts++;
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED){

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

void startMqtt(){
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

 while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD )) {
      Serial.println("connected");
    } else {
      if (client.state()==5){

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

  
  char subscribeTopic[100];
  sprintf(subscribeTopic,"%s/#",MQTT_CLIENT);
  client.subscribe(subscribeTopic);

}

int splitTopic(char* topic, char* tokens[],int tokensNumber ){

    const char s[2] = "/";
    int pos=0;
    tokens[0] = strtok(topic, s);

    while(pos<tokensNumber-1 && tokens[pos] != NULL ) {
        pos++;
      tokens[pos] = strtok(NULL, s);
    }

    return pos;   
}


void checkMqtt(){

  if(!client.connected()){
    startMqtt();
  }

}
