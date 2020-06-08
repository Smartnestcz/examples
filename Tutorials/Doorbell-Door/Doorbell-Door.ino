#include <ESP8266WiFi.h>
#include <PubSubClient.h>// Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>

#define SSID_NAME "Wifi-name"                   // Your Wifi Network name
#define SSID_PASSWORD "Wifi-password"           // Your Wifi network password
#define MQTT_BROKER "smartnest.cz"              // Broker host
#define MQTT_PORT 1883                          // Broker port
#define MQTT_USERNAME "username"                // Username from Smartnest
#define MQTT_PASSWORD "password"                // Password from Smartnest (or API key)
#define MQTT_CLIENT_DOORBELL "doorbell-Id"      // Device Id from smartnest
#define MQTT_CLIENT_LOCK "Lock-Id"              // Device Id from smartnest Type Lock

WiFiClient espClient;
PubSubClient client(espClient); 
int bellPin=0;
bool bellTriggered=false;
int bellReportSend=0;

int doorPin=0;
int openTime = 5000;                             //Time to keep the door open

void startWifi();
void startMqtt();
void sendBellReport();
void checkBell();
void checkMqtt();
int splitTopic(char* topic, char* tokens[] ,int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void open();
void close();

void setup() {
  pinMode(bellPin, INPUT);
  Serial.begin(115200);
  startWifi();
  startMqtt();
}

void loop() {
  client.loop();
  checkBell();
  checkMqtt();
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

  //------------------ACTIONS HERE---------------------------------

  if(strcmp(tokens[1],"directive")==0 && strcmp(tokens[2],"lockedState")==0){
    if(strcmp(message,"false")==0){
      open();
      }
    else if(strcmp(message,"true")==0){
      close();
      }
  }

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

    if (client.connect(MQTT_CLIENT_DOORBELL, MQTT_USERNAME, MQTT_PASSWORD )) {
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

  
  char topic[100];
  sprintf(topic,"%s/#",MQTT_CLIENT_DOORBELL);
  client.subscribe(topic);
  sprintf(topic,"%s/#",MQTT_CLIENT_LOCK);
  client.subscribe(topic);

  sprintf(topic,"%s/report/online",MQTT_CLIENT_DOORBELL);         // Reports that the device is online
  client.publish(topic, "true");

  sprintf(topic,"%s/report/online",MQTT_CLIENT_LOCK);         // Reports that the device is online
  client.publish(topic, "true");
  delay(200);

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

void checkBell(){

  int buttonState = digitalRead(bellPin);
  if (buttonState == LOW && !bellTriggered) {
    return;

  } else if (buttonState == LOW && bellTriggered) {
    bellTriggered = false;

  } else if (buttonState == HIGH && !bellTriggered) {
    bellTriggered = true;
    sendBellReport();
  
  } else if (buttonState == HIGH && bellTriggered) {
    return;

  }

}


void sendBellReport(){  //Avoids sending repeated reports. only once every 5 seconds.

  if(millis()-bellReportSend>5000){ 
    char topic[100];
    sprintf(topic,"%s/report/detectionState",MQTT_CLIENT_DOORBELL);
    client.publish(topic, "true");
    bellReportSend=millis();
  } 

}

void open(){

  Serial.println("Opening");
  digitalWrite(doorPin, LOW);

  if(client.connected()){
    char topicArr[100];
    sprintf(topicArr,"%s/report/lockedState",MQTT_CLIENT_LOCK);
    client.publish(topicArr, "false");
  }

  delay(openTime);
  close();

}

void close(){

  Serial.println("Closing");
  digitalWrite(doorPin, HIGH);

  if(client.connected()){
    char topicArr[100];
    sprintf(topicArr,"%s/report/lockedState",MQTT_CLIENT_LOCK);
    client.publish(topicArr, "true");
  }

}
