//*******************************************************************************
//	SmartThings Arduino Ethernet Library 
//
//	License
//	(C) Copyright 2017 Dan Ogorchock
//
//	History
//	2017-02-04  Dan Ogorchock  Created
//*******************************************************************************
#if defined ARDUINO_ARCH_AVR

#include "SmartThingsEthernetW5100.h"

namespace st
{
	//*******************************************************************************
	// SmartThingsEthernet Constructor  
	//*******************************************************************************
	SmartThingsEthernetW5100::SmartThingsEthernetW5100(byte mac[], IPAddress localIP, IPAddress localGateway, IPAddress localSubnetMask, IPAddress localDNSServer, uint16_t serverPort, IPAddress hubIP, uint16_t hubPort, SmartThingsCallout_t *callout, String shieldType, bool enableDebug, int transmitInterval) :
		SmartThingsEthernet(localIP, localGateway, localSubnetMask, localDNSServer, serverPort, hubIP, hubPort, callout, shieldType, enableDebug, transmitInterval),
		st_server(serverPort)
	{
		//make a local copy of the MAC address
		for (byte x = 0; x <= 5; x++)
		{
			st_mac[x] = mac[x];
    	}
	}


	//*****************************************************************************
	//SmartThingsEthernet::~SmartThingsEthernet()
	//*****************************************************************************
	SmartThingsEthernetW5100::~SmartThingsEthernetW5100()
	{

	}

	//*******************************************************************************
	/// Initialize SmartThingsEthernet Library 
	//*******************************************************************************
	void SmartThingsEthernetW5100::init(void)
	{
		// give the ethernet module time to boot up:
		delay(1000);
		Ethernet.begin(st_mac, st_localIP, st_localDNSServer, st_localGateway, st_localSubnetMask);
		st_server.begin();

		if (_isDebugEnabled)
		{
			Serial.print(F("MAC Address = "));
			Serial.print(st_mac[0], HEX);
			Serial.print(F(":"));
			Serial.print(st_mac[1], HEX);
			Serial.print(F(":"));
			Serial.print(st_mac[2], HEX);
			Serial.print(F(":"));
			Serial.print(st_mac[3], HEX);
			Serial.print(F(":"));
			Serial.print(st_mac[4], HEX);
			Serial.print(F(":"));
			Serial.println(st_mac[5], HEX);
			Serial.print(F("hubIP = "));
			Serial.println(st_hubIP);
			Serial.print(F("hubPort = "));
			Serial.println(st_hubPort);

			Serial.println(F("SmartThingsEthernet: Intialized"));
		}
	}

	//*****************************************************************************
	// Run SmartThingsEthernet Library 
	//*****************************************************************************
	void SmartThingsEthernetW5100::run(void)
	{
		String readString;
		String tempString;
		EthernetClient client = st_server.available();
		if (client) {
			boolean currentLineIsBlank = true;
			while (client.connected()) {
				if (client.available()) {
					char c = client.read();
					//read char by char HTTP request
					if (readString.length() < 100) {
						//store characters to string
						readString += c;
					}
					if (c == '\n' && currentLineIsBlank) {
						//now output HTML data header
						tempString = readString.substring(readString.indexOf('/') + 1, readString.indexOf('?'));

						if (tempString.length() > 0) {
							client.println(F("HTTP/1.1 200 OK")); //send new page
							if (_isDebugEnabled)
							{
								Serial.print(F("Handling request from ST. tempString = "));
								Serial.println(tempString);
							}
							_calloutFunction(tempString);
						}
						else {
							client.println(F("HTTP/1.1 204 No Content"));
							client.println();
							client.println();
							if (_isDebugEnabled)
							{
								Serial.println(F("No Valid Data Received"));
							}

						}
						break;
					}
					if (c == '\n') {
						// you're starting a new line
						currentLineIsBlank = true;
					}
					else if (c != '\r') {
						// you've gotten a character on the current line
						currentLineIsBlank = false;
					}
				}
			}
			readString = "";
			tempString = "";

			delay(1);
			//stopping client
			client.stop();
		}
	}

	//*******************************************************************************
	/// Send Message out over Ethernet to the Hub 
	//*******************************************************************************
	void SmartThingsEthernetW5100::send(String message)
	{
		if (st_client.connect(st_hubIP, st_hubPort))
		{
			st_client.println(F("POST / HTTP/1.1"));
			st_client.print(F("HOST: "));
			st_client.print(st_hubIP);
			st_client.print(F(":"));
			st_client.println(st_hubPort);
			//st_client.println(message);

			st_client.println(F("CONTENT-TYPE: text"));
			st_client.print(F("CONTENT-LENGTH: "));
			st_client.println(message.length());
			st_client.println();
			st_client.println(message);
		}
		else
		{
			//connection failed;
			if (_isDebugEnabled)
			{
				Serial.print(F(""));
				Serial.println(F("SmartThingsEthernet::send() - Ethernet Connection Failed"));
				Serial.print(F("hubIP = "));
				Serial.print(st_hubIP);
				Serial.print(F(" "));
				Serial.print(F("hubPort = "));
				Serial.println(st_hubPort);
			}
		}

		// read any data returned from the POST
		while (st_client.connected() && !st_client.available()) delay(1);  //waits for data
		while (st_client.connected() || st_client.available())             //connected or data available
		{
			char c = st_client.read();
		}

		delay(1);
		st_client.stop();
	}

}
#endif