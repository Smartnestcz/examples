var Gpio = require('onoff').Gpio; //require onoff to control GPIO
var mqtt = require("mqtt");

var deviceId1 = "YOUR-DEVICEID-1";
var deviceId2 = "YOUR-DEVICEID-2";
var deviceId3 = "YOUR-DEVICEID-3";

var options = {
    clientId: deviceId1,           //Client Id of your device from smarnest
    username: "Your-username",            //Username from smarnest
    password: "Your-password",            //Password or You can also use the API key. 
    port: 1883,                           //Port of the broker
    clean: true
};
var client = mqtt.connect("mqtt://smartnest.cz", options);

var device1Pin = new Gpio(4, 'out');        //declare GPIO4 an output
var device2Pin = new Gpio(17, 'out');        //declare GPIO4 an output
var device3Pin = new Gpio(18, 'out');        //declare GPIO4 an output

client.on("connect", function () {       //Connect
    console.log("Connected To Smartnest");
    subscribeToDevice();
});

client.on("error", function (error) {    //Handle Errors
    console.log("Can't connect. error:" + error);
    console.log("Connection not allowed by broker, possible reasons:");
    console.log("- Device is already online. Wait some seconds until it appears offline for the broker");
    console.log("- Wrong Username or password. Check credentials");
    console.log("- Client Id does not belong to this username, verify ClientId");
    process.exit(1);
});

client.on("message", function (topic, message, packet) { //Handle new messages

    console.log("Message received. Topic:", topic, "Message:", message.toString());
    if (topic.split("/")[1] == "directive") {
        if (topic.split("/")[2] == "powerState") {
            let device = topic.split("/")[0];

            if (message.toString() == "ON") {
                if (device == deviceId1) {
                    console.log("Turning ON device 1")
                    device1Pin.writeSync(1);
                } else if (device == deviceId2) {
                    console.log("Turning ON device 2")
                    device2Pin.writeSync(1);
                } else if (device == deviceId3) {
                    console.log("Turning ON device 3")
                    device3Pin.writeSync(1);
                }
                client.publish(device + "/report/powerState", "ON");
            } else {

                if (device == deviceId1) {
                    console.log("Turning OFF device 1")
                    device1Pin.writeSync(0);
                } else if (device == deviceId2) {
                    console.log("Turning OFF device 2")
                    device2Pin.writeSync(0);
                } else if (device == deviceId3) {
                    console.log("Turning OFF device 3")
                    device3Pin.writeSync(0);
                }
                client.publish(device + "/report/powerState", "OFF");
            }

        }
    }

});

function subscribeToDevice() {          //Subscribe
    client.publish(deviceId1 + "/report/online", "true");
    client.publish(deviceId2 + "/report/online", "true");
    client.publish(deviceId3 + "/report/online", "true");

    client.subscribe(deviceId1 + "/#", { qos: 1 });
    client.subscribe(deviceId2 + "/#", { qos: 1 });
    client.subscribe(deviceId3 + "/#", { qos: 1 });
    console.log("Subscribed to all Devices");
}