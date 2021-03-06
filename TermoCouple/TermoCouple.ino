#include <max6675.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP_SSD1306.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

#define OLED_RESET 4

int ktcSO = 12;
int ktcCS = 13;
int ktcCLK = 14;

/************************* OLED Definitions **********************************/
#define OLED_RESET  16  // Pin 15 -RESET digital signal

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "<Your SSID>"
#define WLAN_PASS       "<Your Password>"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883 
#define AIO_USERNAME    "<Your Adafruit Username>"
#define AIO_KEY         "<Your AIO Key>"


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_CLIENTID[] PROGMEM = __TIME__ AIO_USERNAME;
const char MQTT_SERVER[] PROGMEM = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

// OLED Stuff
ESP_SSD1306 display(OLED_RESET); // FOR I2C

void setup() {
	Serial.begin(115200);
	delay(1000);

	display.begin(SSD1306_SWITCHCAPVCC);  // Switch OLED
	  //display.clearDisplay();
	display.display();

	WiFi.begin(WLAN_SSID, WLAN_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();

	Serial.println("WiFi connected");
	Serial.println("IP address: "); Serial.println(WiFi.localIP());

}

void loop() {
	MQTT_connect();
	float DC = ktc.readCelsius();
	float DF = ktc.readFahrenheit();

	char buf[100];

	Serial.print("C = ");
	double celsius = ktc.readCelsius();
	Serial.print(celsius);

	//dtostrf(celsius, 4, 2, buf);
	//mqtt.publish("ct/d/111/TEMPC", buf);


	Serial.print("\t F = ");
	double fahrenheit = ktc.readFahrenheit();
	Serial.println(fahrenheit);
	dtostrf(fahrenheit, 4, 2, buf);
	mqtt.publish("<your adafruit username>/f/<Your Feed Name>", buf);
	writeText(buf, 1, 4, true);
	delay(1000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
	int8_t ret;

	// Stop if already connected.
	if (mqtt.connected()) {
		return;
	}

	Serial.print("Connecting to MQTT... ");

	while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
		Serial.println(mqtt.connectErrorString(ret));
		Serial.println("Retrying MQTT connection in 5 seconds...");
		mqtt.disconnect();
		delay(5000);  // wait 5 seconds
	}
	Serial.println("MQTT Connected!");
}

void writeText(String messageString, int row, int textSize, bool clearScreen)
{
	if (clearScreen)
	{
		display.clearDisplay();
		display.setTextSize(textSize);
		display.setTextColor(WHITE);
	}

	display.setCursor(row, 0);
	display.println(messageString);
	display.display();
}

