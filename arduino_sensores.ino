#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// -----LEITURA DA ÁGUA-----
const int pinosSelecaoCanal[] = {D1,D2};
#define Multiplexador A0

// --TEMPERATURA--
// GPIO where the DS18B20 is connected to
const int oneWireBus = 0;    
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);


// CONDUTIBILIDADE

// TURBIDEZ

// --PH-- 
float ph;
float Value=0;

void initPinosSelecaoCanal() {
  for(int i = 0; i < 2; i++) {
    pinMode(pinosSelecaoCanal[i], OUTPUT);
  }  
}

void ativarPortaAnalogica(int porta) {
  /*
   * De acordo com a tabela de portas do LS151,
   * Porta 1 (TDS) H/L        fio azul
     Porta 2 (PH) L/H         fio roxo
     Porta 3 (Turbidez) H/H   fio verde
   */

  switch(porta){
    case 1: digitalWrite(pinosSelecaoCanal[0], HIGH);
            digitalWrite(pinosSelecaoCanal[1], LOW);  
             
    case 2: digitalWrite(pinosSelecaoCanal[0], LOW);
            digitalWrite(pinosSelecaoCanal[1], HIGH); 

    case 3: digitalWrite(pinosSelecaoCanal[0], HIGH);
            digitalWrite(pinosSelecaoCanal[1], HIGH);   
  }

}

void medirPH() {
    Value= analogRead(Multiplexador);
    float voltage=Value*(3.3/4095.0);
    ph=(3.3*voltage);
    Serial.print("pH: ");
    Serial.print(ph);
}

void medirTemperatura(){
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print("Temperatura Celsius: ");
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print("Temperatura Farenheit: ");
  Serial.print(temperatureF);
  Serial.println("ºF");
}

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  // Start the DS18B20 sensor
  sensors.begin();
}

void loop() {
  medirTemperatura();
  delay(5000);
}
