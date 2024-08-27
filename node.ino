#include <SPI.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <pitches.h>
//#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include "DHT.h"
#include <nRF24L01.h>
#include <RF24.h>



RF24 radio(4, 5); // CE, CSN on Blue Pill 
const uint64_t address = 0xF0F0F0F0E1LL;
int counter = 0;
 



//LiquidCrystal_I2C lcd(0x27, 20, 4); 

const int trigPin = 27;
const int echoPin = 26;
const int relayPin = 25;
/*const char * myWriteAPIKey = "FQEAGPYTDV5BS3CO";  // replace with your API key
const char *ssid =  "ECED";     // replace with your wifi ssid and password
const char *password =  "eced@tiet1";
const char* server = "api.thingspeak.com";
unsigned long myChannelNumber = 1;*/

 
#define CM_TO_INCH 0.393701  //#define SOUND_SPEED

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 15000;

//----DHT declarations
#define DHTPIN 16 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);   // Initializing the DHT11 sensor.
#define ini_h 27.77   // Variable to hold distance readings

struct MyData 
{
  int counter;
  float t1;//temperature
  float h1;//humidity
  long duration1;
  //float distanceCm;
  float distanceInch1;
  float height1;
};
MyData data;


WiFiClient client;
void setup() {
  Serial.begin(115200);//Initialize serial
  delay(10);

  radio.begin();                  //Starting the Wireless communication
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();          //This sets the module as transmitter
 
  dht.begin();


  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); 
  pinMode(relayPin,OUTPUT);

  // lcd.init();
  // lcd.backlight(); 
  //WiFi.mode(WIFI_STA); 
  //ThingSpeak.begin(client);
}
void loop() {
  if ((millis() - lastTime) > timerDelay) {

    
    // Connect or reconnect to WiFi
    /*if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }*/
    // Reading temperature or humidity takes about 250 milliseconds!
    data.counter = counter;
    data.h1 = dht.readHumidity();
    data.t1 = dht.readTemperature();  // Read temperature as Celsius (the default)
    if (isnan(data.h1) || isnan(data.t1) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  


 

  // Get a new distance reading
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);  // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  data.duration1 = pulseIn(echoPin, HIGH);  // Reads the echoPin, returns the sound wave travel time in microseconds
  float distanceCm;
  distanceCm = 0.9659*((data.duration1*(0.0001) * 20.05*sqrt(273.16+data.t1))/2);
  data.height1=(ini_h - distanceCm);
  
  data.distanceInch1 = (data.height1) * CM_TO_INCH;  // Convert to inches
  
  Serial.print("Temperature (ºC): ");
  Serial.print(data.t1);
  Serial.println("ºC");
  Serial.print("Humidity");
  Serial.println(data.h1);
  Serial.print("Distance (cm): ");
  Serial.println(data.height1);// Prints the distance in the Serial Monitor
  Serial.print("Distance (inch): ");
  Serial.println(data.distanceInch1);
  // lcd.setCursor(0,0);
  // lcd.print(height);
  // lcd.print(" cm  ");
  // lcd.setCursor(0,1);
  // lcd.print(distanceInch);
  // lcd.print(" inch  ");

      // if(height < 5)
      // digitalWrite(relayPin,LOW);
      // else 
      // digitalWrite(relayPin,HIGH);
      // delay(500);
  // int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  // if(x == 200){
  //   Serial.println("Channel update successful.");
  //   }
  // else{
  //   Serial.println("Problem updating channel. HTTP error code " + String(x));
  // }
   // ThingSpeak.setField(1,data.t);
  // ThingSpeak.setField(2,data.h);
  // ThingSpeak.setField(3,data.height);
  // ThingSpeak.setField(4,data.distanceInch);


  radio.write(&data, sizeof(MyData));
  
  Serial.println("Data Packet Sent");
  Serial.println("");
  counter++;
  delay(2000);

  lastTime =millis();
}
}