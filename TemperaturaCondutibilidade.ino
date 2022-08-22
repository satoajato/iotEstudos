#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 0
#define TdsSensorPin A0
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

const int pinosSelecaoCanal[] = {D1,D2};
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation

float valor_calibracao = 21.34;   // Fator de calibração
int contagem = 0;           // Variável de contagem
float soma_tensao = 0;      // Variável para soma de tensão
float media = 0;            // Variável que calcula a media
float entradaPinoPH_A0;           // Variável de leitura do pino A0
float tensao;   
float pH;
float Value=0;
unsigned long tempo; 


// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Replace with your network credentials
const char* ssid = "Wifi Unespar Campus Apucarana";
const char* password = "";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDSTemperatureC() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC); 
  }
  return String(tempC);
}

String readDSTemperatureF() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempF = sensors.getTempFByIndex(0);

  if(int(tempF) == -196){
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Temperature Fahrenheit: ");
    Serial.println(tempF);
  }
  return String(tempF);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP DS18B20 Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Celsius</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Fahrenheit</span>
    <span id="temperaturef">%TEMPERATUREF%</span>
    <sup class="units">&deg;F</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Condutibilidade</span>
    <span id="condu">%CONDU%</span>
    <sup class="units">ppm</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Turbidez</span>
    <span id="turbidez">%TURBIDEZ%</span>
    <sup class="units">NTU</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">pH</span>
    <span id="turbidez">%PH%</span>
    <sup class="units">pH</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, 10000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturef").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturef", true);
  xhttp.send();
}, 10000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("condu").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/condu", true);
  xhttp.send();
}, 10000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("turbidez").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/turbidez", true);
  xhttp.send();
}, 10000) ;
</script>
</html>)rawliteral";


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

void initPinosSelecaoCanal() {
  for(int i = 0; i < 2; i++) {
    pinMode(pinosSelecaoCanal[i], OUTPUT);
  }  
}

String tds() {
  ativarPortaAnalogica(1);
pinMode(TdsSensorPin,INPUT);
        int sensortop = 0;
    sensortop = analogRead(TdsSensorPin);    //read the analog value and store into the buffer


float averagevoltage =  sensortop * (float)VREF / 4096.0;    
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      //averageVoltage = getMedianNum(sensortop) * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averagevoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      Serial.print("TDS Value:");
      Serial.print(tdsValue);
      Serial.println("ppm");
      return String(tdsValue);
    
  
}

String turbidez(){
  ativarPortaAnalogica(3);
  int valor = analogRead(TdsSensorPin);
  Serial.print("Turbidez: ");
  Serial.println(valor);
  return String(valor);
}

String ph() {
  ativarPortaAnalogica(2);
  Value= analogRead(A0);
  Serial.print(Value);
  Serial.print(" | ");
  float voltage=Value*(3.3/4095.0);
  pH=(3.3*voltage);
  Serial.print("pH: ");
  Serial.println(pH);
  
  return String(pH);
}

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATUREC"){
    return readDSTemperatureC();
  }
  else if(var == "TEMPERATUREF"){
    return readDSTemperatureF();
  }
  else if(var == "CONDU"){
    return tds();
  }else if(var == "TURBIDEZ"){
    return turbidez();
  }
  else if(var == "PH"){
    return ph();
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  initPinosSelecaoCanal();
  // Start up the DS18B20 library
  sensors.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDSTemperatureC().c_str());
  });
  server.on("/temperaturef", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDSTemperatureF().c_str());
  });
  server.on("/condu", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", tds().c_str());
  });
  server.on("/turbidez", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", turbidez().c_str());
  });
  server.on("/ph", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ph().c_str());
  });
  // Start server
  server.begin();
}



void loop(){
  
}
