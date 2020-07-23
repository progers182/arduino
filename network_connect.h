#ifndef NETWORK_CONNECT_H
#define NETWORK_CONNECT_H

#include<SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 

char ssid[] = SECRET_SSID;        // network SSID (name)
char pass[] = SECRET_PASS;        // network password
int status = WL_IDLE_STATUS;


void connectToNetwork() {

// check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
      // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  }
}

void disconnectFromNetwork() {
  WiFi.disconnect();
  Serial.println("disconnecting from wifi");
}


#endif
