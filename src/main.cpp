#include <Ethernet.h>
#include <MQTT.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient net;
//MQTT client needs some memory. by default it will allocate 128 bytes.
//we can also give it a kilobyte
MQTTClient client(1024);

unsigned long lastMillis = 0;

void connect() {
	Serial.print("connecting...");
	/* clientId, username, password */
	while (!client.connect("arduino", "try", "try")) {
		Serial.print(".");
		delay(1000);
	}

	Serial.println("\nconnected!");

	client.subscribe("/hello");
	// client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
	Serial.println("Incoming: from topic \"" + topic + "\" Payload \"" + payload + "\"");

	// Note: Do not use the client in the callback to publish, subscribe or
	// unsubscribe as it may cause deadlocks when other things arrive while
	// sending and receiving acknowledgments. Instead, change a global variable,
	// or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
	Serial.begin(115200);
	//chip select
	Ethernet.init(PA4);
	//get IP via DHCP
	Ethernet.begin(mac, 10000, 10000);
	auto status = Ethernet.hardwareStatus();
	switch (status) {
	case EthernetNoHardware:
		Serial.println("No hardware detected");
		break;
	case EthernetW5100:
		Serial.println("W5100 detected");
		break;
	case EthernetW5200:
		Serial.println("W5200 detected");
		break;
	case EthernetW5500:
		Serial.println("W5500 detected");
		break;
	default:
		Serial.println("Unknown chip");
		break;
	}

	// Check for Ethernet hardware present
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		Serial.println(
				"Ethernet shield was not found.  Sorry, can't run without hardware. :(");
		while (true) {
			delay(1); // do nothing, no point running without Ethernet hardware
		}
	}
	if (Ethernet.linkStatus() == LinkOFF) {
		Serial.println("Ethernet cable is not connected.");
	}
	Serial.print("Local IP is  ");
	Serial.println(Ethernet.localIP());

	// Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
	// by Arduino. You need to set the IP address directly.
	client.begin("broker.shiftr.io", net);
	client.onMessage(messageReceived);

	connect();
}

void loop() {
	client.loop();

	if (!client.connected()) {
		Serial.println("Reconnecting. Last error " + String(client.lastError()));
		connect();
	}

	// publish a message roughly every second.
	if (millis() - lastMillis > 3000) {
		lastMillis = millis();
		Serial.println("Publishing message to /hello: \"world\"");
		client.publish("/hello", "world");
	}
}
