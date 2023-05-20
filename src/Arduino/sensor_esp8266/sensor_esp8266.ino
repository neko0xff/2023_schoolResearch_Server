#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SimpleDHT.h> 
#include <SparkFun_SGP30_Arduino_Library.h>
#include <MQUnifiedsensor.h>

/*Sensor腳位定義*/
#define pinDHT11 2  //連結腳位: D4

/*MQ7函式定義*/
#define board "ESP8266"
#define Voltage_Resolution 5
#define pinMQ7 A0 //Analog input 0 of your arduino
#define ADC_Bit_Resolution 10 
#define type "MQ-7" //MQ7
MQUnifiedsensor MQ7(board,Voltage_Resolution, ADC_Bit_Resolution, pinMQ7,type);
float calcR0 = 0;

/*SGP30函式定義*/
SGP30 SenSGP30; //連結腳位: SDA&SCL
//SDA=A4
//SCL=A5

/*DHT函式定義*/
SimpleDHT11 dht11;
byte tempVar = 0;
byte humVar = 0;

/*WIFI AP Set*/
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

/*伺服器路徑*/
String serverName = "http://192.168.1.0:3095/upload/Sensor01";

/*timer*/
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

/*WiFi連結*/
void cnWiFi(){
   WiFi.begin(ssid, password);
   Serial.println("Connecting");
   while(WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.print("Connected to WiFi network with IP Address: ");
   Serial.println(WiFi.localIP());
   
   Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

/*SGP30連結*/
void cnSGP30(){
  if (SenSGP30.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  SenSGP30.initAirQuality();
}

/*MQ-7連結*/
void MQ7err(){
  if(isinf(calcR0)){
    Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); 
    while(1);
  }
  if(calcR0 == 0){
    Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); 
    while(1);
  }
}

void cnMQ7(){
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(99.042); MQ7.setB(-1.518); // Configurate the ecuation values to get CO concentration
  MQ7.init(); 
  Serial.print("Calibrating please wait.");
  
  for(int i = 1; i<=10; i ++){
    MQ7.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ7.calibrate(27.5);
    Serial.print(".");
  }

  MQ7.setR0(calcR0/10);
  Serial.println("  done!.");
  MQ7err();
  MQ7.serialDebug(true);
}

float MQ7Var(){
  MQ7.update();
  float COppm =MQ7.readSensor(); 
  return COppm;
}

/*DHT前置偵測*/
void DHTerr(){
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(pinDHT11, &tempVar, &humVar, NULL)) != SimpleDHTErrSuccess) {
       Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
       return;
  }
}

/*主程式*/
void setup() {
  Serial.begin(9600);
  Wire.begin(); 
  cnWiFi();
  cnSGP30();
}

void loop() {  
  
  DHTerr();
  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Data to send with HTTP POST
      String humUD = "hum="+String(humVar);
      String tempUD = "temp="+String(tempVar);
      String coUD = "co="+String(MQ7Var());
      String co2UD = "co2="+String(SenSGP30.CO2);
      String tvocUD = "tvoc="+String(SenSGP30.TVOC);
      String pm25 = "pm25="+String("0");
      String httpRequestData = humUD+"&"+tempUD+"&"+coUD+"&"+co2UD+"&"+tvocUD+"&"+pm25;           
      
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
  
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
       
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}