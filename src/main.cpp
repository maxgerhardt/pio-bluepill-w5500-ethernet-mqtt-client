/*
 Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi

 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

static int32_t readVref();
static float read_temp();

void setup() {
	// You can use Ethernet.init(pin) to configure the CS pin
	//Ethernet.init(10);  // Most Arduino shields
	//Ethernet.init(5);   // MKR ETH shield
	//Ethernet.init(0);   // Teensy 2.0
	//Ethernet.init(20);  // Teensy++ 2.0
	//Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
	//Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet
	analogReadResolution(ADC_RESOLUTION);
	Ethernet.init(PA4);


	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}
	Serial.println("Ethernet WebServer Example");

	// start the Ethernet connection and the server
	//try to get an IP via the DHCP server of the network.
	//max 10 seconds timeout.
	Ethernet.begin(mac, 10000, 10000);

	auto status = Ethernet.hardwareStatus();
	switch(status) {
		case EthernetNoHardware: Serial.println("No hardware detected"); break;
		case EthernetW5100: Serial.println("W5100 detected"); break;
		case EthernetW5200: Serial.println("W5200 detected"); break;
		case EthernetW5500: Serial.println("W5500 detected"); break;
		default: Serial.println("Unknown chip"); break;
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

	// start the server
	server.begin();
	Serial.print("server is at ");
	Serial.println(Ethernet.localIP());
}

void loop() {
	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) {
		Serial.println("new client");
		// an http request ends with a blank line
		bool currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) {
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close"); // the connection will be closed after completion of the response
					client.println("Refresh: 5"); // refresh the page automatically every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					client.println("<h1> Hello World!!</h1>");
					client.println("<p>Sensor data</p>");
					// output the value of each analog input pin
					/* do NOT use PA4..6 for analogRead, these are the SPI pins to the module! */
					uint8_t analogPins[] = {
							  PA0,  //D20/A0
							  PA1,  //D21/A1
							  PA2,  //D22/A2
							  PA3,  //D23/A3
					};
					for (unsigned analogChannel = 0; analogChannel < sizeof(analogPins) / sizeof(*analogPins);
							analogChannel++) {
						int sensorReading = analogRead(analogPins[analogChannel]);
						client.print("analog input ");
						client.print(analogChannel);
						client.print(" is ");
						client.print(sensorReading);
						client.println("<br />");
					}
					client.println("Supply voltage (VRef): " + String(readVref()) + "mV <br/>");
					client.println("</html>");
					break;
				}
				if (c == '\n') {
					// you're starting a new line
					currentLineIsBlank = true;
				} else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(50);
		// close the connection:
		client.stop();
		Serial.println("client disconnected");
	}
}

/* for internal temperature sensor */
#include "stm32yyxx_ll_adc.h"
/* Analog read resolution */
#if ADC_RESOLUTION == 10
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_10B
#define ADC_RANGE 1024
#else
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE 4096
#endif

/* Values available in datasheet */
#define CALX_TEMP 25
#if defined(STM32F1xx)
#define V25       1430
#define AVG_SLOPE 4300
#define VREFINT   1200
#elif defined(STM32F2xx)
#define V25       760
#define AVG_SLOPE 2500
#define VREFINT   1210
#endif


static int32_t readVref()
{
#ifdef __LL_ADC_CALC_VREFANALOG_VOLTAGE
  return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION));
#else
  return (VREFINT * ADC_RANGE / analogRead(AVREF)); // ADC sample to mV
#endif
}


static int32_t readVoltage(int32_t VRef, uint32_t pin)
{
  return (__LL_ADC_CALC_DATA_TO_VOLTAGE(VRef, analogRead(pin), LL_ADC_RESOLUTION));
}



#ifdef ATEMP
static int32_t readTempSensor(int32_t VRef)
{
#ifdef __LL_ADC_CALC_TEMPERATURE
  return (__LL_ADC_CALC_TEMPERATURE(VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
#elif defined(__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS)
  return (__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(AVG_SLOPE, V25, CALX_TEMP, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
#else
  return 0;
#endif
}
#endif

static float read_temp() {
	int32_t vref = readVref();
	int32_t atemp = readVoltage(vref, ATEMP);
	Serial.println("VREF = " + String(vref));
	Serial.println("ATEMP = " + String(atemp));
	return ((V25 - atemp) * 1000.0f / (float) AVG_SLOPE) + CALX_TEMP;
}
