![Smartnest Logo](https://www.smartnest.cz/img/Logo-vector-login.png)
# Doorbell project

## Connect any Doorbell to Alexa

Learn how to connect any regular doorbell to Alexa and receive the alerts in all your echo devices. + Code for door opener

- Project video:
English: https://youtu.be/eGgYHNvwVjY
Spanish: https://youtu.be/cgfVXPfCgkc

- Free web service:
https://www.smartnest.cz

- Example code:
https://github.com/aososam/Smartnest/blob/master/Tutorials/Doorbell/Doorbell-Door.ino

- Stl file of the button for 3D printing:
https://github.com/aososam/Smartnest/blob/master/Tutorials/Doorbell/3DButton.stl

- PubSubClient MQTT library:
https://github.com/knolleary/pubsubclient



### Components:
Node Mcu: https://www.aliexpress.com/item/33040293736.html?spm=a2g0o.productlist.0.0.567954c1JMk2lg
ESP Wifi Relay Module: https://www.aliexpress.com/item/32840806183.html?spm=a2g0o.productlist.0.0.1db74f12UD31Dn
Optocoupler: https://www.aliexpress.com/item/33016844527.html?spm=a2g0o.productlist.0.0.166d3740YTgNwO
Small breadboard
Resistors 200  and 1K ohm


### Other social networks:
Instagram: @smartnestcz
Facebook: @smartnestcz
twitter: @smartnestcz
tiktok: @smartnest
linked-in: @andres-sosa



#define SSID_NAME "Wifi-name"                   // Your Wifi Network name
#define SSID_PASSWORD "Wifi-password"           // Your Wifi network password
#define MQTT_BROKER "smartnest.cz"              // Broker host
#define MQTT_PORT 1883                          // Broker port
#define MQTT_USERNAME "username"                // Username from Smartnest
#define MQTT_PASSWORD "password"                // Password from Smartnest (or API key)
#define MQTT_CLIENT_DOORBELL "doorbell-Id"      // Device Id from smartnest
#define MQTT_CLIENT_DOOR "door-Id"              // Device Id from smartnest