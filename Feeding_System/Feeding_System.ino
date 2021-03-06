 #include <Stepper.h>
#include "Arduino.h"
#include <WiFi.h>
#include "time.h"
#include <ThingsBoard.h>
#include<Wire.h>
#include <math.h>

/****************************************************************/
/*******************Init of the stepper**************************/
/****************************************************************/

const int MPU=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
double pitch,roll;

/*======================================*/

//convert the accel data to pitch/roll
void getAngle(int Vx,int Vy,int Vz) {
double x = Vx;
double y = Vy;
double z = Vz;

pitch = atan(x/sqrt((y*y) + (z*z)));
roll = atan(y/sqrt((x*x) + (z*z)));
//convert radians into degrees
pitch = pitch * (180.0/3.14);
roll = roll * (180.0/3.14) ;
}

/****************************************************************/
/*******************Init of the stepper**************************/
/****************************************************************/
 
// Define number of steps per rotation:
const int stepsPerRevolution = 2048;
 
// Wiring Pin 5 to IN1; Pin 17 to IN2;
// Wiring Pin 16 to IN3; Pin 4 to IN4;
// Create stepper object called 'myStepper', note the pin order:
Stepper myStepper = Stepper(stepsPerRevolution, 5, 16, 17, 4);
 
/****************************************************************/
/***********************Init of the Ultrasonic**************************/
/****************************************************************/
 
// Defines pins numbers
#define trig_pin 23
#define echo_pin 19
 
// Ultrasonic setup of the trig and echo pins
void UltrasonicSetup() {
  pinMode(trig_pin, OUTPUT); // Sets the TRIG_PIN as an Output
  pinMode(echo_pin, INPUT); // Sets the ECHO_PIN as an Input
}
 
// The funtion for getting the real distance in cm
float get_distance() {
 
// Defines variables
  long duration;
  int distance;
 
// Clears the TRIG_PIN
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
 
// Sets the TRIG_PIN on HIGH state for 10 micro seconds
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
 
// Reads the ECHO_PIN, returns the sound wave travel time in microseconds
  duration = pulseIn(echo_pin, HIGH);
 
// Calculating the distance
  distance = duration*0.0343/2;
  return distance;
}
 
/****************************************************************/
/****************Getting the time from wifi**********************/
/****************************************************************/
 
int hours = 0;
int minutes = 0;
int seconds = 0;
 
int local_time[3];
 
// Setup of the connection with your wifi
const char* ssid       = "MTC_010";
const char* password   = "ALH84001";
 
// Setup of the local time (UTC+3)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 108000;
const int   daylightOffset_sec = 3600;
 
// Get the local time
void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
}
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
 
//
void get_Local_time(int time_array[]){
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  time_array[0] = timeinfo.tm_hour;
  time_array[1] = timeinfo.tm_min;
  time_array[2] = timeinfo.tm_sec;  
}
 
 
//Connect to WiFi
void Connect_to_WiFi()
{
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");}
  Serial.println(" CONNECTED");
}
 
//Init and get the time
void Init_and_get_the_time() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
 
//Disconnect WiFi as it's no longer needed
void Disconnect_WiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

/**********************************************************/
/****************Conecting with Thingsoard*****************/
/**********************************************************/

#define TOKEN "2HW1gjUczfg0TqlSkEr3"

char thingsboardServer[] = "demo.thingsboard.io";

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend1;
unsigned long lastSend2;


void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.println("Connecting to ThingsBoard node ...");
    if ( tb.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

/**********************************************************/

void setup() {
 
// Begin Serial communication at a baud rate of 115200:
  Serial.begin(115200);
  UltrasonicSetup();
 
// Set the speed to 10 rpm:
  myStepper.setSpeed(10);
 
//Connect to WiFi
  Connect_to_WiFi();
 
//Init and get the time
  Init_and_get_the_time();
 
//Disconnect WiFi as it's no longer needed
//  Disconnect_WiFi();

Wire.begin();
Wire.beginTransmission(MPU);
Wire.write(0x6B);
Wire.write(0);
Wire.endTransmission(true);
}
 
/**************************************************************/

void loop() {
 
 
// Gen the value of distance and keep it in a float variable
  float distance = get_distance();
  float last_hour = -1;

  if (local_time[0] == 7 || local_time[0] == 11 || local_time[0] == 19)
  {
    if (local_time[0] != last_hour)
    {
        last_hour = local_time[0];
        myStepper.step(stepsPerRevolution);
    }
  }

  if (!tb.connected())
  {
    reconnect();
  }

  if (distance > 0 & distance < 20) {
    
    Serial.println(distance);
    Serial.println("In recipient este mancare");
    
    if ( millis() - lastSend1 > 1000 )
    { 
      // Update and send only after 1 seconds
      Serial.println("Collecting ultrasonic  data.");

      float test_distance = map(distance, 1, 20, 1, 100);
      Serial.println(test_distance);

      //operatiile de obtinere a datelor de la sensor 
  
      tb.sendTelemetryFloat("Food_quantity", test_distance);
      lastSend1 = millis();
    }
  }
  else
  {
    float test = 0;
    tb.sendTelemetryFloat("Food_quantity", test);
  }
    

  
  tb.loop();

/**********************************************************************/

Wire.beginTransmission(MPU);
Wire.write(0x3B);
Wire.endTransmission(false);
Wire.requestFrom(MPU,14,true);

int AcXoff,AcYoff,AcZoff,GyXoff,GyYoff,GyZoff;
int temp,toff;
double t,tx,tf;

//Acceleration data correction
AcXoff = -950;
AcYoff = -300;
AcZoff = 0;

//Temperature correction
toff = -1600;

//Gyro correction
GyXoff = 480;
GyYoff = 170;
GyZoff = 210;

//read accel data
AcX=(Wire.read()<<8|Wire.read()) + AcXoff;
AcY=(Wire.read()<<8|Wire.read()) + AcYoff;
AcZ=(Wire.read()<<8|Wire.read()) + AcYoff;


//read gyro data
GyX=(Wire.read()<<8|Wire.read()) + GyXoff;
GyY=(Wire.read()<<8|Wire.read()) + GyYoff;
GyZ=(Wire.read()<<8|Wire.read()) + GyZoff;

//get pitch/roll
getAngle(AcX,AcY,AcZ);

//send the data out the serial port
Serial.print("Angle: ");
Serial.print("Pitch = "); Serial.print(pitch);
Serial.print(" | Roll = "); Serial.println(roll);


      if ( millis() - lastSend2 > 1000 )
      { 
        // Update and send only after 1 seconds
        Serial.println("Collecting gyroscope data.");
  
        //operatiile de obtinere a datelor de la gyroscope
    
        tb.sendTelemetryFloat("Pitch", pitch );
        lastSend2 = millis();
      }
     myStepper.step(stepsPerRevolution);
      delay(5000);
}
