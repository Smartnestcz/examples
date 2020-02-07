#include <ESP8266WiFi.h>
#include <PubSubClient.h>// Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>

#define SSID_NAME "Your-WIFI-name"            // Changue to your Wifi Network name
#define SSID_PASSWORD "Your-WIFI-Password"     // Change to your Wifi network password
#define MQTT_BROKER "smartnest.cz" //
#define MQTT_PORT 1883 //
#define MQTT_USERNAME "Your-username-from-smartnest" //
#define MQTT_PASSWORD "your-password " //
#define MQTT_CLIENT "Your-device-ID" //

WiFiClient espClient;
PubSubClient client(espClient); 
int sensorPin=0;
bool sensorOn=true;
bool sensorTriggered=false;
int sensorReportSent=0;

void startWifi();
void startMqtt();
void turnOff();
void turnOn();
void sendReport(bool value);
void checkSensor();
void checkMqtt();
int splitTopic(char* topic, char* tokens[] ,int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  pinMode(sensorPin, INPUT);
  Serial.begin(115200);
  startWifi();
  startMqtt();
}

void loop() {
  client.loop();
  checkSensor();
  checkMqtt();
}

void callback(char* topic, byte* payload, unsigned int length) { //A new message has been received
  Serial.print("Topic:");
  Serial.println(topic);
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
    
     char reportChange[100];
     sprintf(reportChange,"%s/report/powerState",MQTT_CLIENT);
 
//------------------ACTIONS HERE---------------------------------

if(strcmp(tokens[1],"directive")==0 && strcmp(tokens[2],"powerState")==0){
  if(strcmp(message,"ON")==0){
    turnOn();
    }
  else if(strcmp(message,"OFF")==0){
    turnOff();
    }
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
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(0x7530);
    }
  }
  
  char reportTopic[100];
  char publishTopic[100];
  sprintf(reportTopic,"%s/report/online",MQTT_CLIENT);
  sprintf(publishTopic,"%s/#",MQTT_CLIENT);
  client.subscribe(publishTopic);
  client.publish(reportTopic, "true");
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

    void checkSensor(){
  int buttonState = digitalRead(sensorPin);
  if (buttonState == LOW && !sensorTriggered)
  {
    return;
  }
  else if (buttonState == LOW && sensorTriggered)
  {
    sensorTriggered = false;
    sendReport(false);
  }
  else if (buttonState == HIGH && !sensorTriggered)
  {
    sensorTriggered = true;
    sendReport(true);
  }
  else if (buttonState == HIGH && sensorTriggered)
  {
    return;
  }
}

void turnOff() {

      sensorOn=false;
      char reportTopic[100];
      sprintf(reportTopic,"%s/report/powerState",MQTT_CLIENT);
      client.publish(reportTopic, "OFF");

}
void turnOn() {
    sensorOn=true;
    char reportTopic[100];
    sprintf(reportTopic,"%s/report/powerState",MQTT_CLIENT);
    client.publish(reportTopic, "ON");
}

void sendReport(bool value){
      if(millis()-sensorReportSent>500 && sensorOn){
          char reportTopic[100];
           sprintf(reportTopic,"%s/report/detectionState",MQTT_CLIENT);
          if(value) client.publish(reportTopic, "true");
          else client.publish(reportTopic, "false");
           sensorReportSent=millis();
        } 
  }
