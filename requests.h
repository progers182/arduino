#ifndef REQUESTS_H
#define REQUESTS_H

#include<SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

WiFiClient client;
char server[] = "www.phrogers.com";

/*
 * returns json object of 
 * data given a specific action
 * to call
 * 
 */
StaticJsonDocument<512> getRequest(String action) {
    // Define the JSON document
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = 512;
    StaticJsonDocument<capacity> data;
  
   // build GET request
  String request = "GET /ac/api/data/" + action + ".php HTTP/1.0";
  
  Serial.println("\nStarting connection to server...");
  
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    // Send HTTP request
    client.println(request);
    client.println(F("Host: phrogers.com"));
    client.println(F("Connection: close"));
    if (client.println() == 0) {
      Serial.println(F("Failed to send request"));
      data["error"] = "Failed to send request";
      return data;
    }
  
    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      data["error"] = "Unexpected response: ";
      return data;
    }
  
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      data["error"] = "Invalid Response";
      return data;
    }
  
    // Parse JSON object
    DeserializationError err = deserializeJson(data, client);
    if (err) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(err.c_str());
      data["error"] = "deserializeJson() failed: ";
      return data;
    }

    // Disconnect
    client.stop();
    return data;
  }
}


/*
 * POSTs new servo state to server
 * 
 */
void postRequest(unsigned short newState, unsigned short deviceId, String action, unsigned short table = 0) {
  // Define Post data
  StaticJsonDocument<64> data;
  if (table == 1) {
    data["curr_state"] = newState;
    data["device_id"] = deviceId;  
  }

  else if (table == 2) {
    data["command"] = newState;
    data["device_id"] = deviceId; 
  }

  // build POST request
  String request = "POST /ac/api/data/" + action + ".php" + ((action == "create" && table != 0) ? "?table=" + String(table) : "") + " HTTP/1.1"; 
  
  Serial.println("\nStarting connection to server...");
  
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    // Send HTTP request
    client.println(request);
    client.println(F("Host: phrogers.com"));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/json"));
    client.print("Content-Length: ");
    client.println(measureJson(data));
    client.println();
    serializeJson(data, client);

    Serial.println(request);
    Serial.println(F("Host: phrogers.com"));
    Serial.println(F("Connection: close"));
    Serial.println(F("Content-Type: application/json"));
    Serial.print("Content-Length: ");
    Serial.println(measureJson(data));
    Serial.println();
    serializeJson(data, Serial);
    if (client.println() == 0) {
      Serial.println(F("Failed to send request"));
      return;
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) { // TODO fix not getting Response from server
      char c = client.read();
      Serial.write(c);
    }
  
    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting from server.");
      client.stop();
    }
    return;
  }
}

#endif
