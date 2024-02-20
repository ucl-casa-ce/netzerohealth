/*
  Based on official Arduino code by Giampaolo Mancini
*/

#include <MKRNB.h>
#include "arduino_secrets.h" // SIM and MQTT secrets 
#include <PubSubClient.h>
#include "Adafruit_MPRLS.h"

#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

const char pin[]      = SIM_PIN;
const char apn[]      = SIM_APN;
const char login[]    = SIM_APN_LOGIN;
const char password[] = SIM_APN_PASS;

const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;   
const char* mqtt_server = MQTT_URL;
const int mqtt_port = MQTT_PORT;

NBClient nbClient;
NB nbAccess;
PubSubClient client(nbClient);

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

    if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Found MPRLS sensor");

  //Wait for the modem to initialise
  MODEM.begin();
  while (!MODEM.noop());

  /*  Set radio preference, just first time flashing the board, 
   *  these settings will be written into the flash memory of the modem:
   *  7     => CAT M1 only
   *  8     => NB IoT only
   *  7,8   => CAT M1 preferred, NB IoT as failover (default)
   *  8,7   => NB IoT preferred, CAT M1 as failover
   */
  //setNetworkPreference("7,8");

  client.setServer(mqtt_server, mqtt_port);
  connectLTENetwork();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /***** Sensor reading *****/
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

  /***** Publising to MQTT *****/
  sendMQTT(String((int) averagePa));
}

void sendMQTT(String pressure_Pa) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  char msg[50];
  pressure_Pa.toCharArray(msg,pressure_Pa.length()+1);
  client.publish("student/CASA0014/plant/ucfnmyr/pressureLTE_Pa", msg);
}

//MQTT reconnection, taken from @djdunc
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

void connectLTENetwork() {
  // connection state
  bool connected = false;

  Serial.print("connecting to cellular network ...");

  while (!connected) {
    if ((nbAccess.begin(pin, apn, login, password) == NB_READY)) {
      connected = true;
    } else {
      Serial.print(".");
      delay(1000);
    }
  }

  //Serial.println(nbAccess.readPDPparameters());
}
  
void setNetworkPreference(String uratChoice){
  setRAT(uratChoice);
  apply();
}

bool setRAT(String choice)
{
  String response;

  Serial.print("Disconnecting from network: ");
  MODEM.sendf("AT+COPS=2");
  MODEM.waitForResponse(2000);
  Serial.println("done.");

  Serial.print("Setting Radio Access Technology: ");
  MODEM.sendf("AT+URAT=%s", choice.c_str());
  MODEM.waitForResponse(2000, &response);
  Serial.println("done.");

  return true;
}

bool apply()
{
  Serial.print("Applying changes and saving configuration: ");
  MODEM.send("AT+CFUN=15");
  MODEM.waitForResponse(5000);
  delay(5000);

  do {
    delay(1000);
    MODEM.noop();
  } while (MODEM.waitForResponse(1000) != 1);

  Serial.println("done.");

  return true;
}
