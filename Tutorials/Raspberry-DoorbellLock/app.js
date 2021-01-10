var Gpio = require('onoff').Gpio; //require onoff to control GPIO
var mqtt = require("mqtt");

let clientIdDoorbell = "Your-Doorbell-Client-ID";
let clientIdLock = "Your-Lock-Client-ID";

var options = {
    clientId: clientIdDoorbell,           //Client Id of your device from smarnest
    username: "Your-username",            //Username from smarnest
    password: "Your-password",            //Password or You can also use the API key. 
    port: 1883,                           //Port of the broker
    clean: true
};
var options = {
    clientId: clientIdDoorbell,           //Client Id of your device from smarnest
    username: "Your-username",            //Username from smarnest
    password: "Your-password",            //Password or You can also use the API key. 
    port: 1883,                           //Port of the broker
    clean: true
};
var client = mqtt.connect("mqtt://smartnest.cz", options);
var connected = false;

const buttonPin = new Gpio(4, 'in', 'both');
const lockPin = new Gpio(17, 'out');
buttonPin.watch((err, value) => buttonStateChanged(value));

client.on("connect", function () {       //Connect
    console.log("Connected To Smartnest");
    connected = true;

    client.subscribe(clientIdLock + "/#", { qos: 1 });
    client.publish(clientIdLock + "/report/online", "true");

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

        if (topic.split("/")[2] == "lockedState") {

            if (message.toString() == "false") {
                openDoor();
            } else {
                closeDoor();
            }

        }
    }

});

function buttonStateChanged(value) {
    if (connected && value === 1) {
        client.publish(clientIdDoorbell + "/report/detectionState", "true");
    }
}

function openDoor() {
    console.log("Opening Door")
    lockPin.writeSync(1);
    client.publish(clientIdLock + "/report/lockedState", "false");

    setTimeout(() => {
        lockPin.writeSync(0);
        closeDoor();
    }, 5000);
}

function closeDoor() {
    console.log("Close Door")
    lockPin.writeSync(0);
    client.publish(clientIdLock + "/report/lockedState", "true");

}