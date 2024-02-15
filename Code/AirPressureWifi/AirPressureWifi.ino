#include <Wire.h>
#include "Adafruit_MPRLS.h"
//Connectivity imports
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" // Wifi and MQTT secrets 

#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);




//Constants
const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;   
const char* mqtt_server = MQTT_URL;
const int mqtt_port = MQTT_PORT;

//Internal fields
WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin (9600);
  Serial.println("Hi");

  //Connect to an SSID and print local IP address, taken from CASA plant monitoring class
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  //Initialise MQTT connection
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 
  
  if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Found MPRLS sensor");


}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int readingNum = 24;
  float readings[readingNum];
  
  for(int i = 0; i< readingNum ; i++){
    delay(250);
    float pressure_hPa = mpr.readPressure();
    readings[i] = pressure_hPa;
    Serial.print("Pressure (Pa): "); Serial.println(pressure_hPa*100);
  } 

  float sum = 0;
  for(int i = 0; i< readingNum ; i++){
    sum += readings[i];
  }
  float averagePa = (sum/readingNum)*100;
  Serial.print("------ Pressure average (Pa): "); Serial.println((int) averagePa);
  sendMQTT(String((int) averagePa));
}

void sendMQTT(String pressure_hPa) {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  char msg[50];
  pressure_hPa.toCharArray(msg,pressure_hPa.length()+1);
  //client.publish("student/NetZero/OPS/206/sensor1/pressure_hPa", msg);
  client.publish("student/CASA0014/plant/ucfnmyr/pressure_hPa4", msg);
}



//MQTT reconnection, taken from CASA plant monitoring class
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {    // while not (!) connected....
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "NetZero-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
      // ... and subscribe to messages on broker
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//MQTT callback event, taken from CASA plant monitoring class
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


}
