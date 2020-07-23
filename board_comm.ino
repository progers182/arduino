#include <ArduinoJson.h>
#include <Servo.h>
#include "network_connect.h"
#include "requests.h"

// mapping for states
const unsigned short LOW_VENT   = 0;
const unsigned short HIGH_VENT  = 1;
const unsigned short LOW_COOL   = 2;
const unsigned short HIGH_COOL  = 3;
const unsigned short PUMP       = 4;
const unsigned short OFF        = 5;
const unsigned short CALIBRATE  = 6;

const unsigned short ARDUINO_ID = 1;

const unsigned short SERVO_STOP = 1500;
const unsigned short SERVO_MOVE = 1600;
const unsigned short MOVE_TIME = 1050;

Servo myServo;


void setSleep(unsigned short numMinutes = 1) {
  const int MIN_IN_MS = 1000 * 60;
  
  delay(numMinutes * MIN_IN_MS);
}

void rotateServo(unsigned short newLocation, unsigned short oldLocation) {
  const unsigned short NUM_POSITIONS = 6;
  unsigned short numTurns = 0;

  if (newLocation < oldLocation) {
    numTurns = oldLocation - newLocation;
  }
  else if (newLocation > oldLocation) {
    numTurns = newLocation - oldLocation;
    numTurns = NUM_POSITIONS - numTurns;
  }
  else if (newLocation == oldLocation) {
    return;
  }
  Serial.println();
  Serial.print(numTurns);

  for (int i = 0; i < numTurns; i++) {
    myServo.writeMicroseconds(SERVO_MOVE);
    delay(MOVE_TIME);
    myServo.writeMicroseconds(SERVO_STOP);
    delay(1000);
  }
  myServo.writeMicroseconds(SERVO_STOP);
}

void calibrate() {
  const int FULL_ROTATION = 8;
  for (int i = 0; i < FULL_ROTATION; i++) {
    myServo.writeMicroseconds(SERVO_MOVE);
    delay(MOVE_TIME);
    myServo.writeMicroseconds(SERVO_STOP);
    delay(1000);
  }
  myServo.writeMicroseconds(SERVO_STOP);
}

void setup() {

  // attach servo to pin 9
  myServo.attach(9);
  
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
}

void loop() {
  // vars
  StaticJsonDocument<512> data;
  const unsigned short WEATHER_TIME = 6;

  // connect to wifi
  connectToNetwork();

  // get current command
  data = getRequest("queue");

  // error handling
  if (data.containsKey("error")) {
    Serial.print(F("ERROR: "));
    Serial.println(data["error"].as<String>());
    return;
  }
  
  unsigned short command = data["command"];
  unsigned short curr_state = data["curr_state"];
  unsigned short curr_time = data["curr_hour"];

  if (command == CALIBRATE) {
      calibrate();
      // create command
      postRequest(curr_state, data["device_id"], "create", 2);
  }
  // update servo if current state is not synced with current command
  else if (! data["is_updated"]) {
      rotateServo(command, curr_state);
      // report new state
      postRequest(command, data["device_id"], "create", 1);
  }
  // only run weather logic after midnight and before 6 AM
  else if (curr_time < WEATHER_TIME) {
    float weatherState = curr_state;
    data = getRequest("weather");
    float temp = data["feels_like"];
    if (temp < 60) {
        weatherState = OFF;
    }
    else if (temp < 70) {
      weatherState = LOW_VENT;
    }
    else if (temp < 80) {
      weatherState = LOW_COOL;
    }
    else {
      weatherState = HIGH_COOL;
    }
    
    rotateServo(weatherState, curr_state);
    postRequest(weatherState, ARDUINO_ID, "create", 1);
    
    // delay for an extra 4 minutes at night (total of 5 minutes)
    setSleep(4);
  }

  //  disconnectFromNetwork();
  
  // sleep for 1 minute
   setSleep(1);
}
