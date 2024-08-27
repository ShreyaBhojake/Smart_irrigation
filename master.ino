#include<SPI.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
//#include <pitches.h>
//#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include "DHT.h"
#include <nRF24L01.h>
#include <RF24.h>


//LiquidCrystal_I2C lcd(0x27, 20, 4); 

const int trigPin = 27;
const int echoPin = 26;
const int relayPin = 25;
const char*  myWriteAPIKey = "FQEAGPYTDV5BS3CO";  // replace with your API key
String apikey = "FQEAGPYTDV5BS3CO";
const char *ssid =  "EACCESS";     // replace with your wifi ssid and password
const char *password =  "hostelnet";
const char* server = "api.thingspeak.com";
RF24 radio(4, 5); // CE, CSN on Blue Pill 
const uint64_t address = 0xF0F0F0F0E1LL;

 
#define CM_TO_INCH 0.393701  //#define SOUND_SPEED

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 15000;

//----DHT declarations
#define DHTPIN 16 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);   // Initializing the DHT11 sensor.
#define ini_h 23.13 
unsigned long myChannelNumber = 1;   // Variable to hold distance readings
long duration;
float distanceCm;
float distanceInch;
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
  radio.begin();
  Serial.println("Receiver Started....");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();
  delay(10);
  dht.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); 
  pinMode(relayPin,OUTPUT);
  //lcd.init();
  //lcd.backlight(); 
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);
}
int recvData()
{
  if ( radio.available() ) 
  {
    radio.read(&data, sizeof(MyData));
    return 1;
    }
    return 0;
}
void loop() {
  if ((millis() - lastTime) > timerDelay) {

    
    // Connect or reconnect to WiFi
   /* if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }*/
    // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // Read temperature as Celsius (the default)
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  return;
  }
  Serial.println("data of Master");
  Serial.println("Temperature (ºC): ");
  Serial.print(t);
  Serial.println("ºC");
  Serial.print("Humidity");
  Serial.println(h);
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  // Get a new distance reading
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);  // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);  // Reads the echoPin, returns the sound wave travel time in microseconds
  distanceCm = 0.9659*((duration*(0.0001) * 20.05*sqrt(273.16+t))/2);
  float height=(ini_h - distanceCm);
  /*if(height < 5)
  digitalWrite(relayPin,LOW);
  else 
  digitalWrite(relayPin,HIGH);*/
  distanceInch = (height) * CM_TO_INCH;  // Convert to inches
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(height);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  //lcd.setCursor(0,0);
  //lcd.print(height);
  //lcd.print(" cm  ");
  //lcd.setCursor(0,1);
  //lcd.print(distanceInch);
  //lcd.print(" inch  ");
  delay(500);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
    }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  ThingSpeak.setField(3,height);
  ThingSpeak.setField(4,distanceInch);


  
  if(recvData())
  {
  Serial.println("data of node");
  Serial.println("Data Received:");
  Serial.print("Packet No. = ");
  Serial.println(data.counter);
  
  Serial.print("Temperature = ");
  Serial.print(data.t1);
  Serial.println("*C");
 

  Serial.print("Humidity = ");
  Serial.print(data.h1);
  Serial.println("%");

  Serial.print("Distance(cm)");
  Serial.println(data.height1);

  Serial.print("Distance(inch)");
  Serial.print(data.distanceInch1);
  Serial.println();
  lastTime =millis();

 
  
  
  }
    
  ThingSpeak.setField(5,data.t1);
  ThingSpeak.setField(6,data.h1);
  ThingSpeak.setField(7,data.height1);
  ThingSpeak.setField(8,data.distanceInch1);
  }

}