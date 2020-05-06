var Gpio = require('onoff').Gpio; //require onoff to control GPIO
var mqtt = require("mqtt");

var options = {
  clientId: "Your-Client-ID",           //Client Id of your device from smarnest
  username: "Your-username",            //Username from smarnest
  password: "Your-password",            //Password or You can also use the API key. 
  port: 1883,                           //Port of the broker
  clean: true
};
var client = mqtt.connect("mqtt://smartnest.cz", options);

var LEDPin = new Gpio(4, 'out');        //declare GPIO4 an output

client.on("connect", function() {       //Connect
    console.log("Connected To Smartnest");
    subscribeToDevice();
});

client.on("error", function(error) {    //Handle Errors
    console.log("Can't connect" + error);
    process.exit(1);
});

client.on("message", function(topic, message, packet) { //Handle new messages

    console.log("Message received. Topic:",topic,"Message:",message.toString());
   if(topic.split("/")[1]=="directive"){
        if(topic.split("/")[2]=="powerState"){
            if(message.toString()=="ON"){
                console.log("Turning pin ON")
                LEDPin.writeSync(1);
                client.publish(options.clientId + "/report/powerState", "ON");
            } else {
                console.log("Turning pin OFF")
                LEDPin.writeSync(0);
                client.publish(options.clientId + "/report/powerState", "OFF");
            }
        
        }
   }
  
});

function subscribeToDevice() {          //Subscribe
    client.subscribe(options.clientId + "/#", { qos: 1 });
    console.log("Subscribed to Device topic", options.clientId +"/#");
}