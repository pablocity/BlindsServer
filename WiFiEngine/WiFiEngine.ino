/*
 Name:		WiFiEngine.ino
 Created:	8/3/2020 12:52:57 AM
 Author:	Pawel
*/

#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <string>
#include "HTTPServer.cpp"

#define PIN0 D0
#define PIN1 D1
#define PIN2 D2
#define PIN3 D3
#define PIN4 D4
#define PIN5 D5
#define PIN6 D6
#define PIN7 D7
#define PIN8 D8

LiquidCrystal display(PIN5, PIN4, PIN3, PIN2, PIN1, PIN0);
WiFiClient client;
WiFiManager manager;
HTTPServer server(WiFi);
State currentEngineState;
int currentEngineSpeed;

void configModeCallback(WiFiManager* myWiFiManager) {
	Serial.println("Entered config mode\n");
	Serial.println(WiFi.localIP());
	Serial.println("\n");
	Serial.println(WiFi.softAPIP());

	Serial.println(myWiFiManager->getConfigPortalSSID());
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);

	pinMode(PIN8, OUTPUT);
	//digitalWrite(PIN8, HIGH);

	pinMode(PIN7, OUTPUT);
	pinMode(PIN6, OUTPUT);

	manager.setConnectTimeout(180);
	//manager.setAPStaticIPConfig(IPAddress(192, 168, 0, 2), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
	manager.setSTAStaticIPConfig(IPAddress(192, 168, 0, 5), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
	manager.setAPCallback(configModeCallback);
	manager.autoConnect("Smart Windblind WiFi", "Abcdefg123");

	//display.print("GOPA forever <3");

	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("Waiting for WiFi connection...");
		delay(3000);
	}

	if (!server.SetupServer())
	{
		Serial.println("Server encountered unexpected problem, please reset and debug");
	}
	
	/*
	display.clear();
	display.print("Created WiFi network, connect using SmartWindblinds mobile or dektop app");*/

}

// the loop function runs over and over again until power down or reset
void loop() {

	server.HandleClient();

	if (currentEngineState != server.engine.state)
	{
		currentEngineState = server.engine.state;

		if (currentEngineState == STOPPED)
		{
			digitalWrite(PIN7, LOW);
			digitalWrite(PIN6, LOW);
			Serial.println("Engine stopped");
		}
		else if (currentEngineState == RUNNING_LEFT)
		{
			digitalWrite(PIN7, LOW);
			digitalWrite(PIN6, HIGH);
			Serial.println("Engine spinning left");
		}
		else
		{
			digitalWrite(PIN7, HIGH);
			digitalWrite(PIN6, LOW);
			Serial.println("Engine spinning right");
		}
	}

	if (currentEngineSpeed != server.engine.speed)
	{
		Serial.println("Engine last speed: " + String(currentEngineSpeed));

		currentEngineSpeed = server.engine.speed;

		Serial.println("Engine current speed: " + String(currentEngineSpeed));

		analogWrite(PIN8, currentEngineSpeed);
	}
}
