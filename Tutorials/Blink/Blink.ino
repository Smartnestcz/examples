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
int lightPin=2;

void startWifi();
void startMqtt();
void checkMqtt();
int splitTopic(char* topic, char* tokens[] ,int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void setup() {

  pinMode(lightPin, OUTPUT);
  Serial.begin(115200);
  startWifi();
  startMqtt();

}

void loop() {

  client.loop();
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
 
//------------------ACTIONS HERE---------------------------------
  char myTopic[100];
  
  if(strcmp(tokens[1],"directive")==0 && strcmp(tokens[2],"powerState")==0){
    sprintf(myTopic,"%s/report/powerState",MQTT_CLIENT);

    if(strcmp(message,"ON")==0){
      digitalWrite(lightPin, LOW); 
      client.publish(myTopic, "ON") ;            

    } else if(strcmp(message,"OFF")==0){
      digitalWrite(lightPin, HIGH);                   
      client.publish(myTopic, "OFF"); 

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

  
  char topic[100];
  sprintf(topic,"%s/#",MQTT_CLIENT);
  client.subscribe(topic);

  sprintf(topic,"%s/report/online",MQTT_CLIENT);         // Reports that the device is online
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
