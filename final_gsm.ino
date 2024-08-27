#include <String.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <pitches.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include "DHT.h"

SoftwareSerial gprsSerial(3,1);   //R T pins dht sensor


LiquidCrystal_I2C lcd(0x27, 20, 4); //lcd display

const int trigPin = 27;     //ultrasonic sensor
const int echoPin = 26;

#define DHTPIN 16 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE); // Initializing the DHT11 sensor.

unsigned long myChannelNumber = 1;

unsigned long lastTime = 0;
unsigned long timerDelay = 15000;

#define ini_h 23.13
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
WiFiClient client;

void setup() {
  Serial.begin(9600);//Initialize serial
  gprsSerial.begin(9600);               // the GPRS baud rate   

  delay(10);
  ThingSpeak.begin(client);   //non wifi
  dht.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); 
  lcd.begin();
  lcd.backlight(); 
  WiFi.mode(WIFI_STA); 
 
  delay(500);

}

void loop() {
      //DHT sensor       
      float h = dht.readHumidity();
      float t = dht.readTemperature(); 
      delay(100);   

      Serial.print("Temperature = ");
      Serial.print(t);
      Serial.println(" °C");
      Serial.print("Humidity = ");
      Serial.print(h);
      Serial.println(" %");    

      //ULTRASONIC sensor
      // Get a new distance reading
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);    // Sets the trigPin on HIGH state for 10 micro seconds
      digitalWrite(trigPin, LOW);
        
      duration = pulseIn(echoPin, HIGH);    // Reads the echoPin, returns the sound wave travel time in microseconds
      distanceCm = 0.9659*((duration*(0.0001) * 20.05*sqrt(273.16+t))/2);
      float height=(ini_h - distanceCm);
        distanceInch = (height) * CM_TO_INCH;   // Convert to inches

      Serial.print("Distance (cm): ");
      Serial.println(height);
      Serial.print("Distance (inch): ");
      Serial.println(distanceInch);
      lcd.setCursor(0,0);
      lcd.print(height);
      lcd.print(" cm  ");
      lcd.setCursor(0,1);
      lcd.print(distanceInch);
      lcd.print(" inch  ");
      delay(500);
  if (gprsSerial.available())
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
  
  String str="GET https://api.thingspeak.com/update?api_key=DDGA4GHHRZ832KPH""&field1=" + String(t) +"&field2="+String(h)+"&field3="+String(height)+"&field4="+String(distanceInch);
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

void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
  
}
