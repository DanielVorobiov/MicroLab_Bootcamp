#include <WiFi.h>
#include <ThingsBoard.h>
#include <Servo_ESP32.h>

#define TOKEN "osiM8RxQBhIDc4W2EdVO"
#define sensorPin 32
static const int servoPin = 33; 

Servo_ESP32 servo1;

int status = WL_IDLE_STATUS;
unsigned long lastSend;

const char*_ssid       = "";
const char*_password   = "";
char thingsboardServer[] = "demo.thingsboard.io";

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

//declararea variabilelor necesare pentru executarea programului
int timer;
int steps;
int value;
int luminozitate;
int stare;

int angle =0;
int angleStep = 5;
int angleMin =0;
int angleMax = 180;


 

//Connectarea la WiFi
void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  WiFi.begin(_ssid, _password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

//Reconectarea la WiFi
void reconnect() {
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(_ssid, _password);
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
      delay( 5000 );
    }
  }
}

 
void setup() {
  Serial.begin(9600);
    InitWiFi(); 
 servo1.attach(servoPin);
}
 
void loop() {
if (!tb.connected()) { //conectarea la ThingsBoard
   reconnect();
}

value = analogRead(sensorPin);//citirea valorii senzorului LDR
luminozitate = map(value, 0 , 4095, 0, 100); /Transformarea valorii în procente
Serial.println(luminozitate);
tb.sendTelemetryFloat("luminozitate:", luminozitate); //transmiterea datelor despre luminozitatea către ThingsBoard
tb.sendTelemetryFloat("Jaluzele", stare); //transmiterea datelor despre starea jaluzeleleor (deschise, închise) către ThingsBoard


if (luminozitate > 40 && timer==7200){ //dacă luminozitatea depășeste 40% la verificarea după 2 ore, atunci jaluzelele se închid și se modifică și starea acestora
servo1.write(0);
stare = 1;
  delay(50);
 
}
if (luminozitate <=40 && timer ==10){ //dacă luminozitatea depășeste 40% la verificarea după 2 ore, atunci jaluzelele se deschidși se modifică și starea acestora
  servo1.write(180);
  stare= 2;
  delay(50);
}

timer++; //calcularea timpului până la 2 ore
if (timer>=7200){
  timer =0;
}


Serial.println(timer);
delay(1000);
tb.loop();
}
