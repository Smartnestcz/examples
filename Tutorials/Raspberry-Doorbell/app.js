var Gpio = require('onoff').Gpio; //require onoff to control GPIO
var mqtt = require("mqtt");

var options = {
    clientId: "Your-client-id",           //Client Id of your device from smarnest
    username: "Your-username",            //Username from smarnest
    password: "Your-password",            //Password or You can also use the API key. 
    port: 1883,                           //Port of the broker
    clean: true
};
var client = mqtt.connect("mqtt://smartnest.cz", options);
var connected = false;

const buttonPin = new Gpio(4, 'in', 'both');
const ledPin = new Gpio(17, 'out');
buttonPin.watch((err, value) => buttonStateChanged(value));

client.on("connect", function () {       //Connect
    console.log("Connected To Smartnest");
    connected = true;
});

client.on("error", function (error) {    //Handle Errors
    console.log("Can't connect. error:" + error);
    console.log("Connection not allowed by broker, possible reasons:");
    console.log("- Device is already online. Wait some seconds until it appears offline for the broker");
    console.log("- Wrong Username or password. Check credentials");
    console.log("- Client Id does not belong to this username, verify ClientId");
    process.exit(1);
});

function buttonStateChanged(value) {
    ledPin.writeSync(value);
    if (connected && value === 1) {
        client.publish(options.clientId + "/report/detectionState", "true");
    }
}