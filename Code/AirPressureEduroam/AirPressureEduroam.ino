//Code is based on https://github.com/martinius96/ESP32-eduroam/blob/master/2022/eduroam/eduroam.ino

/*|----------------------------------------------------------|*/
/*|Connection sketch to eduroam network (WPA/WPA2) Enteprise |*/
/*|Suitable for almost any ESP32 microcontroller with WiFi   |*/
/*|Raspberry or Arduino WiFi CAN'T USE THIS LIBRARY!!!       |*/
/*|Edited by: Martin Chlebovec (martinius96)                 |*/
/*|Compilation under 2.0.3 Arduino Core and higher worked    |*/
/*|Compilation can be done only using STABLE releases        |*/
/*|Dev releases WILL NOT WORK. (Check your Ard. Core .json)  |*/
/*|WiFi.begin() have more parameters for PEAP connection     |*/
/*|----------------------------------------------------------|*/

//WITHOUT certificate option connection is WORKING (if there is exception set on RADIUS server that will let connect devices without certificate)
//It is DEPRECATED function and standardly turned off, so it must be turned on by your eduroam management at university / organisation

//Connection with certificate WASN'T CONFIRMED ever, so probably that option is NOT WORKING
#include <Wire.h>
#include "Adafruit_MPRLS.h"

#include <WiFi.h> //Wifi library
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks
//Connectivity imports
#include <PubSubClient.h>
#include "MedianFilterLib.h" //Median library: https://www.arduino.cc/reference/en/libraries/medianfilterlib/
#include "arduino_secrets.h" // Wifi and MQTT secrets 

#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);
const int readingNum = 25;

MedianFilter<float> medianFilter(readingNum); //size of window

//Identity for user with password related to his realm (organization)
//Available option of anonymous identity for federation of RADIUS servers or 1st Domain RADIUS servers
const char* wifi_identity = SECRET_EAP_IDENTITY;
const char* wifi_password = SECRET_EAP_PASSWORD;
const char* wifi_username = SECRET_EAP_USERNAME;

const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;   
const char* mqtt_server = SECRET_MQTT_BROKER;
int mqtt_port = SECRET_MQTT_PORT;

//SSID NAME
const char* ssid = SECRET_SSID; // eduroam SSID

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  ConnectWifi();

  client.setServer(mqtt_server, mqtt_port);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqttuser, mqttpass)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Found MPRLS sensor");
  
    // Publish and subscribe
    client.publish("student/CASA0014/plant/ucfnmyr/esp32", "Hi, I'm ESP32 ^^");
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

    
  float Pressure_Pa_median;
  
  for(int i = 0; i< readingNum ; i++){
    float pressure_hPa = mpr.readPressure();
    Serial.print("Pressure (hPa): "); Serial.println(pressure_hPa);
    float pressure_Pa = pressure_hPa*100;
    Serial.print("Pressure (Pa): "); Serial.println(pressure_Pa);
    Pressure_Pa_median = medianFilter.AddValue(pressure_Pa);
    Serial.print("Pressure (Pa) median: "); Serial.println(Pressure_Pa_median);
    delay(600);    
  } 

  Serial.print("------ Pressure (Pa) median: "); Serial.println((int) Pressure_Pa_median);
  sendMQTT(String((int) Pressure_Pa_median));
  }
}

void sendMQTT(String pressure_hPa) {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  char msg[50];
  pressure_hPa.toCharArray(msg,pressure_hPa.length()+1);
  //client.publish("student/NetZero/OPS/206/sensor1/pressure_Pa", msg);
  client.publish("student/CASA0014/plant/ucfnmyr/medianPressure_Pa2", msg);
}

//MQTT reconnection, taken from CASA plant monitoring class
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected() && WiFi.status() == WL_CONNECTED) {    // while not (!) connected....
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
void ConnectWifi(){
  delay(1000);
  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD, test_root_ca); //with CERTIFICATE 
  WiFi.begin(ssid, WPA2_AUTH_PEAP, wifi_identity, wifi_username, wifi_password); // without CERTIFICATE, RADIUS server EXCEPTION "for old devices" required

  // Example: a cert-file WPA2 Enterprise with PEAP - client certificate and client key required
  //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD, test_root_ca, client_cert, client_key);

  // Example: TLS with cert-files and no password - client certificate and client key required
  //WiFi.begin(ssid, WPA2_AUTH_TLS, EAP_IDENTITY, NULL, NULL, test_root_ca, client_cert, client_key);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F(".c"));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP()); //print LAN IP
  
}
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  delay(1000);
  WiFi.reconnect();
}
