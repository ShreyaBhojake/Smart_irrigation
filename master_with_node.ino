#include<SPI.h>
#include <Wire.h>
#include <pitches.h>
//#include <LiquidCrystal_I2C.h>
#include<SoftwareSerial.h>
SoftwareSerial gprsSerial(3,1);
#include "DHT.h"
#include <nRF24L01.h>
#include <RF24.h>

//LiquidCrystal_I2C lcd(0x27, 20, 4); 

const int trigPin = 27;
const int echoPin = 26;
const int relayPin = 25;

RF24 radio(4, 5, 18, 19, 23); // CE, CSN,SCK,MISO,MOSI 
const uint64_t address = 0xF0F0F0F0E1LL;


 
#define CM_TO_INCH 0.393701  //#define SOUND_SPEED

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

//----DHT declarations
#define DHTPIN 16 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);   // Initializing the DHT11 sensor.
#define ini_h 27.77    // Variable to hold distance readings
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

//WiFiClient client;
void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); 
  pinMode(relayPin,OUTPUT);
  Serial.begin(9600);//Initialize serial
  delay(10);
  gprsSerial.begin(9600);
  delay(10);
  radio.begin();
  radio.setChannel(5);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  //radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();
  delay(10);
  dht.begin();
  
  // lcd.init();
  // lcd.backlight(); 
  
  
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
    // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // Read temperature as Celsius (the default)
  delay(250);
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  return;
  }
  Serial.print("Temperature (ºC): ");
  Serial.print(t);
  Serial.println("ºC");
  Serial.print("Humidity");
  Serial.println(h);
  
  // Get a new distance reading
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);  // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);  // Reads the echoPin, returns the sound wave travel time in microseconds
  distanceCm = 0.9659*((duration*(0.0001) * 20.05*sqrt(273.16+t))/2);
  float height=(ini_h - distanceCm);
  // if(height < 5)
  // digitalWrite(relayPin,HIGH);
  // else 
  // digitalWrite(relayPin,HIGH);
  distanceInch = (height) * CM_TO_INCH;  // Convert to inches
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(height);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  // lcd.setCursor(0,0);
  // lcd.print(height);
  // lcd.print(" cm  ");
  // lcd.setCursor(0,1);
  // lcd.print(distanceInch);
  // lcd.print(" inch  ");
  delay(500);
  if(gprsSerial.available())
    Serial.write(gprsSerial.read());
  gprsSerial.println("AT");
  delay(1000);
  gprsSerial.println("AT+CPIN?");
  delay(1000); 
  gprsSerial.println("AT+CREG?");
  delay(1000); 
  gprsSerial.println("AT+CGATT?");
  delay(1000); 
  gprsSerial.println("AT+CIPSHUT");
  delay(2000); 
  gprsSerial.println("AT+CIPSTATUS");
  delay(2000); 
  gprsSerial.println("AT+CIPMUX=0");
  delay(2000); 
  ShowSerialData(); 
  gprsSerial.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
  delay(1000); 
  ShowSerialData(); 
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(3000); 
  ShowSerialData(); 
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000); 
  ShowSerialData(); 
  gprsSerial.println("AT+CIPSPRT=0");
  delay(3000); 
  ShowSerialData();  
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(5000); 
  ShowSerialData();
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();  
  String str="GET https://api.thingspeak.com/update?api_key=CM5KD50GI0ICPUEC""&field1=" + String(t) +"&field2="+String(h)+"&field3="+String(height)+"&field4="+String(distanceInch)+"&field5=" + String(data.t1) +"&field6="+String(data.h1)+"&field7="+String(data.height1)+"&field8="+String(data.distanceInch1);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server
  
  delay(4000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
  }
}
void ShowSerialData()
{
  while(gprsSerial.available()!=0){
    Serial.write(gprsSerial.read());
    delay(5000);}
}