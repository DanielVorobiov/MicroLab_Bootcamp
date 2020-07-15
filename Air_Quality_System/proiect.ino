#include "DHT.h"
#define DHT_Pin 10
#define DHT_Type DHT22

DHT dht (DHT_Pin, DHT_Type);
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
float humidity = dht.readHumidity();
float temperature= dht.readTemperature();


Serial.print(humidity);

Serial.print(":");
Serial.println(temperature);
delay(1000);
}
