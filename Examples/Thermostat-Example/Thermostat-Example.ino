//#include "WiFi.h"       //Uncomment for ESP32  
#include <ESP8266WiFi.h> //Comment for ESP-32

#include <PubSubClient.h>// Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>

#define SSID_NAME "Your-WIFI-name"            // Changue to your Wifi Network name
#define SSID_PASSWORD "Your-WIFI-Password"     // Change to your Wifi network password
#define MQTT_BROKER "smartnest.cz" //
#define MQTT_PORT 1883 //
#define MQTT_USERNAME "Your-username-from-smartnest" //
#define MQTT_PASSWORD "your-password " //
#define MQTT_CLIENT "Your-device-ID" //   // Change to the device ID from your device, get it at smartnest.cz/mynest

WiFiClient espClient;
PubSubClient client(espClient);
int heaterPin=2;
int sensorpin=A0;

char topic[100];
char msg[5];
double temp=25;
double setpoint=25;
double lastTimeSent=0;

void startWifi();
void startMqtt();
void checkMqtt();
void mqttLoop();
int splitTopic(char* topic, char* tokens[] ,int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void turnOn();
void turnOff();
void setSetpoint(double temp);
void reportTemperature(double temp);


void setup(){
  pinMode(heaterPin, OUTPUT);
  pinMode(sensorpin, INPUT);
  Serial.begin(115200);
  startWifi();
  startMqtt();
}

void loop(){
  client.loop();
  checkMqtt();
  reportTemperature(temp);
  
}

void callback(char* topic, byte* payload, unsigned int length) { //A new message has been received

  char *tokens[10];
  char message[length+1];

  Serial.print("MQTT Message Received. Topic:");
  Serial.println(topic);
  Serial.print("Message:");
  Serial.println(message);

  splitTopic(topic, tokens, tokensNumber);

  sprintf(message,"%c",(char)payload[0]);
  for (int i = 1; i < length; i++) {
    sprintf(message,"%s%c",message,(char)payload[i]);
  }

  //------------------Decide what to do depending on the topic and message---------------------------------

  if(strcmp(tokens[1],"directive")==0 && strcmp(tokens[2],"powerState")==0){// Turn On or OFF

    if(strcmp(message,"ON")==0){ turnOn(); }
    else if(strcmp(message,"OFF")==0){ turnOff();}
  
  } else if(strcmp(tokens[1],"directive")==0 && strcmp(tokens[2],"setpoint")==0) {// Set Setpoint Temperature

      double value = atof(message);
      if (isnan(value)){ setpoint = 0; }

      setSetpoint(value);
    
      sprintf(topic,"%s/report/setpoint",MQTT_CLIENT);
      client.publish(topic, message);

  }
}

void startWifi(){
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

void startMqtt(){

  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD )) {
      Serial.println("connected");
    } else {
      if (client.state()==-5){

        Serial.print("Connection not allowed by broker, possible reasons:");
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
 
  sprintf(topic,"%s/report/online",MQTT_CLIENT); //Reports to the server that device is online
  client.publish(topic, "true");

  sprintf(topic,"%s/#",MQTT_CLIENT); //Subscribes to all messages send to the device
  client.subscribe(topic);
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
  if(!client.connected()){ startMqtt(); }
}

void turnOff(){

  digitalWrite(heaterPin, HIGH);

  if(client.connected()){ 
    sprintf(topic,"%s/report/powerState",MQTT_CLIENT);
    client.publish(topic, "OFF") ;
  }

}

void turnOn(){
    
  digitalWrite(heaterPin, LOW);

  if(client.connected()){ 
    sprintf(topic,"%s/report/powerState",MQTT_CLIENT);
    client.publish(topic, "ON") ;
  }

}

void setSetpoint(double temp){

  Serial.printf("setpoint changed to %3.2f", temp);
  //Do something with the value, Turn on Heater? turn on AC?

}

void reportTemperature(double temp){

  if(millis()-lastTimeSent>30000 && client.connected()){
    
    sprintf(topic,"%s/report/temperature",MQTT_CLIENT);
    client.publish(topic, "15") ;
    lastTimeSent = millis();

  }

}
