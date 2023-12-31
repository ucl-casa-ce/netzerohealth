
#include <ArduinoBLE.h>
//#include "ClosedCube_HDC1080.h"

#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>

//BLE Region
BLEService newService("180A"); // creating the service
BLEStringCharacteristic randomReading("2A58", BLERead | BLENotify, 100); // creating the Analog Value characteristic
BLEByteCharacteristic switchChar("2A57", BLERead | BLEWrite); // creating the LED characteristic

const int ledPin = 2;
long previousMillis = 0;
//End BLE Region

#include <Wire.h>
#include "Adafruit_MPRLS.h"
#include <SPI.h>
#include <SD.h>
#include <RTCZero.h>
#include <WiFiNINA.h>  
#include <utility/wifi_drv.h> 


/* Create an rtc object */

RTCZero rtc;
//ClosedCube_HDC1080 hdc1080;
Weather sensor;
const int chipSelect = 4;

// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

void setup() {
  Serial.begin(115200);
  rtc.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

   // RGB LED's
  WiFiDrv::pinMode(25, OUTPUT); // G
  WiFiDrv::pinMode(26, OUTPUT); // R
  WiFiDrv::pinMode(27, OUTPUT); // B  

  blinkRGB(500, 2, 20, 150, 150);
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    blinkRGB(500, 2, 250, 0, 0);
    while (1);
  }
  Serial.println("card initialized.");

  Serial.println("MPRLS Simple Test");
  if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
    blinkRGB(500, 2, 0, 250, 0);
    while (1) {
      delay(10);
    }
  }
  Serial.println("Found MPRLS sensor");

  //hdc1080.begin(0x40);

//BLE Region

   if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");
    blinkRGB(500, 2, 0, 0, 250);
    while (1);
  }

   BLE.setLocalName("MKR WiFi 1010"); //Setting a name that will appear when scanning for Bluetooth® devices
  BLE.setAdvertisedService(newService);

  newService.addCharacteristic(switchChar); //add characteristics to a service
  newService.addCharacteristic(randomReading);

  BLE.addService(newService);  // adding the service

  switchChar.writeValue(0); //set initial value for characteristics
  randomReading.writeValue("initial");

  BLE.advertise(); //start advertising the service
  Serial.println(" Bluetooth® device active, waiting for connections...");
//End BLE Region

}


void loop() {

  String serialDataReceived = "";
  serialDataReceived = Serial.readString();
  if(serialDataReceived != ""){
    String seconds = splitString(serialDataReceived,',',0);
    String minutes = splitString(serialDataReceived,',',1);
    String hours = splitString(serialDataReceived,',',2);
    String days = splitString(serialDataReceived,',',3);
    String months = splitString(serialDataReceived,',',4);
    String years = splitString(serialDataReceived,',',5);

    rtc.setSeconds(seconds.toInt());
    rtc.setMinutes(minutes.toInt());
    rtc.setHours(hours.toInt());
    rtc.setDay(days.toInt());
    rtc.setMonth(months.toInt());
    rtc.setYear(years.toInt());
  }
  
    String currentTime = getTime();
    
    String dataString = "";

    float pressure_hPa = mpr.readPressure();

    delay(20);
    
    //float tmp = hdc1080.readTemperature();
    float tmp = sensor.getTemp();
    
    delay(20);
    //float hum = hdc1080.readHumidity();
    float hum = sensor.getRH();
    
    //Serial.print("Pressure (hPa): ");
    dataString += currentTime;
    dataString += ",";
    dataString += String(pressure_hPa);
    dataString += ",";
    dataString += String(tmp);
    dataString += ",";
    dataString += String(hum);
    Serial.println(dataString);

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      //Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      //Serial.println("error opening datalog.txt");
    }

 
  //Serial.print("Pressure (PSI): "); Serial.println(pressure_hPa / 68.947572932);
  //delay(1000);

//BLE Region
  BLEDevice central = BLE.central();
  if (central) {  // if a central is connected to the peripheral
    Serial.print("Connected to central: ");
    
    Serial.println(central.address()); // print the central's BT address
    
    digitalWrite(LED_BUILTIN, HIGH); // turn on the LED to indicate the connection

    // check the battery level every 200ms
    // while the central is connected:
    if (central.connected()) {
      long currentMillis = millis();
      
      if (currentMillis - previousMillis >= 200) { // if 200ms have passed, we check the battery level
        previousMillis = currentMillis;

        randomReading.writeValue(dataString);

        if (switchChar.written()) {
          if (switchChar.value()) {   // any value other than 0
            Serial.println("LED on");
            digitalWrite(ledPin, HIGH);         // will turn the LED on
          } else {                              // a 0 value
            Serial.println(F("LED off"));
            digitalWrite(ledPin, LOW);          // will turn the LED off
          }
        }

      }
    }
    
    digitalWrite(LED_BUILTIN, LOW); // when the central disconnects, turn off the LED
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
//End BLE Region
  }
}

String getTime() {

  String num = "";

  num+= print2digits(rtc.getDay());

  num+= "/";

  num+= print2digits(rtc.getMonth());

  num+= "/";

  num+= print2digits(rtc.getYear());

  num+= " ";

  // ...and time

  num+= print2digits(rtc.getHours());

  num+= ":";

  num+= print2digits(rtc.getMinutes());

  num+= ":";

  num+= print2digits(rtc.getSeconds());

  return num;
}

String print2digits(int number) {
  String num = "";
  if (number < 10) {
    num+="0";
  }

  num+= String(number);
  return num;
}

// Useful string splitting function, taken from:
// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void blinkRGB(int ms, int loops, int r, int g, int b){
  while(loops){
    WiFiDrv::analogWrite(25, r);
    WiFiDrv::analogWrite(26, g);
    WiFiDrv::analogWrite(27, b);
    delay(ms);
    WiFiDrv::analogWrite(25, 0);
    WiFiDrv::analogWrite(26, 0);
    WiFiDrv::analogWrite(27, 0);
    delay(ms);
    loops-=1;   
  }

}
