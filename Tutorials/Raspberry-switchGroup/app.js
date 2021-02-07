var Gpio = require('onoff').Gpio; //require onoff to control GPIO
var mqtt = require("mqtt");

var options = {
    clientId: "YOUR-DEVICE-ID",           //Client Id of your device from smarnest
    username: "YOUR-USERNAME",            //Username from smarnest
    password: "YOUR-PASSWORD",            //Password or You can also use the API key. 
    port: 1883,                           //Port of the broker
    clean: true
};
var client = mqtt.connect("mqtt://smartnest.cz", options);

const pin1 = new Gpio(4, 'out');;
const pin2 = new Gpio(3, 'out');;
const pin3 = new Gpio(2, 'out');;

client.on("connect", function () {       //Connect
    console.log("Connected To Smartnest");
});

client.on("error", function (error) {    //Handle Errors
    console.log("Can't connect" + error);
    process.exit(1);
});

client.on("message", function (topic, message, packet) { //Handle new messages

    console.log("Message received. Topic:", topic, "Message:", message.toString());
    if (topic.split("/")[1] == "directive") {
    
        if (topic.split("/")[2] == "powerState1") {
            if (message.toString() == "ON") {
                console.log("Turning ON pin 1")
                pin1.writeSync(1);
                client.publish(options.clientId + "/report/powerState1", "ON");
            } else {
                console.log("Turning OFF pin 1")
                pin1.writeSync(0);
                client.publish(options.clientId + "/report/powerState1", "OFF");
            }

        } else if (topic.split("/")[2] == "powerState2") {
            if (message.toString() == "ON") {
                console.log("Turning ON pin 2")
                pin2.writeSync(1);
                client.publish(options.clientId + "/report/powerState2", "ON");
            } else {
                console.log("Turning OFF pin 2")
                pin2.writeSync(0);
                client.publish(options.clientId + "/report/powerState2", "OFF");
            }

        } else if (topic.split("/")[2] == "powerState3") {
            if (message.toString() == "ON") {
                console.log("Turning ON pin 3")
                pin3.writeSync(1);
                client.publish(options.clientId + "/report/powerState3", "ON");
            } else {
                console.log("Turning OFF pin 3")
                pin3.writeSync(0);
                client.publish(options.clientId + "/report/powerState3", "OFF");
            }

        }
    }

});