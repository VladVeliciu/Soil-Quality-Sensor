#include <Wire.h>
#include <SoftwareSerial.h>
#include <WiFi.h>           // WiFi control for ESP32
#include <ThingsBoard.h>    // ThingsBoard SDK
#include <TBPubSubClient.h>
#include <ArduinoJson.h>

#define RE 13
#define DE 12

// declaration of wi-fi credentials
const char* wifiSSID = "V9061";
const char* wifiPassword = "3*04Xe91";

// Thingsboard credentials
String tbHost = "192.168.15.56";//adresa ip raspberry pi
String tbToken = "z75yg74YU1TBB4pMC1ld";

WiFiClient client;
ThingsBoard tb(client);
PubSubClient mqtt(client);

// functions declarations
void connectWifi();
void tbReconnect();
void on_message(const char *topic, byte *payload, unsigned int length);

//inquiry format for the soil characteristics  
const byte nitro[] = {0x01, 0x03, 0x00, 0x1E, 0x00, 0x01, 0xE4, 0x0C};  
const byte phos[] = {0x01, 0x03, 0x00, 0x1F, 0x00, 0x01, 0xB5, 0xCC};  
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xC0};
const byte soil_ph[] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B};
const byte soil_moist[] = {0x01, 0x03, 0x00, 0x12, 0x00, 0x01, 0x24, 0x0F};
const byte temp[] = {0x01, 0x03, 0x00, 0x12, 0x00, 0x02, 0x64, 0x0E};
const byte ec[] = {0x01, 0x03, 0x00, 0x15, 0x00, 0x01, 0x95, 0xCE};

byte values[8];
SoftwareSerial SerialPort(16,17);  // Use SoftwareSerial for RS485 communication
  
void setup() {
  Serial.begin(9600);
  SerialPort.begin(4800); // Set the baud rate and pins for SoftwareSerial
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  connectWifi();
  mqtt.setServer(tbHost.c_str(), 1883); // set the server for MQTT communication using specific port 1883
  mqtt.setCallback(on_message);
  delay(1000);
}

void loop() {   
  //variables to store the received data from sensor
  int nitrovar, phosvar, potavar, ecvar;
  float phvar, moistvar, tempvar;

  if (!tb.connected()){
    tbReconnect();
  }

  nitrovar=nitrogen();
  phosvar=phosphorous();
  potavar=potassium();
  phvar=ph();
  moistvar=moist();
  tempvar=stemp();
  ecvar=econd();

  Serial.print("Nitrogen: ");
  Serial.print(nitrovar);
  Serial.println(" mg/kg");
 
  Serial.print("Phosphorous: ");
  Serial.print(phosvar);
  Serial.println(" mg/kg");

  Serial.print("Potassium: ");
  Serial.print(potavar);
  Serial.println(" mg/kg");

  Serial.print("Soil pH: ");
  Serial.print(phvar);
  Serial.println(" pH");

  Serial.print("Soil Moisture: ");
  Serial.print(moistvar);
  Serial.println(" %");

  Serial.print("Temperature: ");
  Serial.print(tempvar);
  Serial.println(" C");

  Serial.print("Electrical Conductivity: ");
  Serial.print(ecvar);
  Serial.println(" uS/cm");

  Serial.println();

  Serial.println("Sending data to Thingsboard");

  tb.sendTelemetryInt("Nitrogen", nitrovar);
  tb.sendTelemetryInt("Phosphorous", phosvar);
  tb.sendTelemetryInt("Potassium", potavar);
  tb.sendTelemetryFloat("PH", phvar);
  tb.sendTelemetryFloat("Moisture", moistvar);
  tb.sendTelemetryFloat("Temperature", tempvar);
  tb.sendTelemetryInt("Electrical conductivity", ecvar);
  tb.loop();
  mqtt.loop();

  delay(1000);
}
  
int nitrogen() {
  do{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (SerialPort.write(nitro, sizeof(nitro)) == 8){
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
      for (int i = 0; i < 7; i++) {
        values[i] = SerialPort.read();
        //Serial.print(values[i], HEX);
        //Serial.println();
        delay(10);
        if (values[0]!=0x01)//jump over first transmission if it is wrong
          i--;
      }
    }
  }while(values[0]!=0x01 || values[1]!=0x03);

  int inter = values[3] * 256 + values[4];
  serialFlush();
  delay(100);
  return inter;
}

int phosphorous() {
  do{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (SerialPort.write(phos, sizeof(phos)) == 8){
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
      for (int i = 0; i < 7; i++){
        values[i] = SerialPort.read();
        //Serial.print(values[i], HEX);
        //Serial.println();
        delay(10);
        if (values[0]!=0x01)//jump over first transmission if it is wrong
          i--;
      }
    }  
  }while(values[0]!=0x01 || values[1]!=0x03);

  int inter = values[3] * 256 + values[4];
  serialFlush();
  delay(100);
  return inter;
}

int potassium() {
  do{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (SerialPort.write(pota, sizeof(pota)) == 8){
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
      for (int i = 0; i < 7; i++) {
        values[i] = SerialPort.read();
        //Serial.print(values[i], HEX);
        //Serial.println();
        delay(10);
        if (values[0]!=0x01)//jump over first transmission if it is wrong
          i--;
      }
    }
  }while(values[0]!=0x01 || values[1]!=0x03);

  int inter = values[3] * 256 + values[4];
  serialFlush();
  delay(100);
  return inter;
}

double ph() {
  do{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (SerialPort.write(soil_ph,sizeof(soil_ph))==8){
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
      for (int i = 0; i < 7; i++) {
        values[i] = SerialPort.read();
        //Serial.print(values[i], HEX);
        //Serial.println();
        delay(10);
        if (values[0]!=0x01)//jump over first transmission if it is wrong
          i--;
      }
    }
  }while(values[0]!=0x01 || values[1]!=0x03);

  int inter = values[3] * 256 + values[4];
  int interfint=inter/100;
  int interffrac=inter%100;
  serialFlush();
  delay(100);
  return (double)interfint+(double)interffrac/100;
}

double moist() {
  do{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (SerialPort.write(soil_moist, sizeof(soil_moist)) == 8){
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
      for (int i = 0; i < 7; i++) {
        values[i] = SerialPort.read();
        //Serial.print(values[i], HEX);
        //Serial.println(); 
        delay(10);
        if (values[0]!=0x01)//jump over first transmission if it is wrong
          i--;
      }
    }
  }while(values[0]!=0x01 || values[1]!=0x03);
 
  int inter = values[3] * 256 + values[4];
  int interfint=inter/100;
  int interffrac=inter%100;
  serialFlush();
  delay(100);
  return (double)interfint+(double)interffrac/100;
}

double stemp() {
  do{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (SerialPort.write(temp, sizeof(temp)) == 8){
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
      for (int i = 0; i < 9; i++) {
        values[i] = SerialPort.read();
        //Serial.print(values[i], HEX);
        //Serial.println();
        delay(10);
        if (values[0]!=0x01)//jump over first transmission if it is wrong
          i--;
      }
    }
  }while(values[0]!=0x01 || values[1]!=0x03);

  int inter = values[5] * 256 + values[6];
  int interfint=inter/100;
  int interffrac=inter%100;
  serialFlush();
  delay(100);
  return (double)interfint+(double)interffrac/100;
}

int econd() {
  do{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (SerialPort.write(ec, sizeof(ec)) == 8){
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
      for (int i = 0; i < 7; i++) {
        values[i] = SerialPort.read();
        //Serial.print(values[i], HEX);
        //Serial.println();
        delay(10);
        if (values[0]!=0x01)//jump over first transmission if it is wrong
          i--;
      }
    }
  }while(values[0]!=0x01 || values[1]!=0x03);

  int inter = values[3] * 256 + values[4];
  serialFlush();
  delay(100);
  return inter;
}

void serialFlush(){//function for flushing the buffer
  while(SerialPort.available() > 0)
    char t = SerialPort.read();
}

void tbReconnect(){
  while (!tb.connected()){
    if (WiFi.status() != WL_CONNECTED)
      connectWifi();

    Serial.println("connecting to thingsboard ... ");
    if (tb.connect(tbHost.c_str(), tbToken.c_str())){
      Serial.println("Thingsboard Connected!");
      }
    else{
      Serial.println("Thingsboard connection failed");
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void connectWifi() {
  Serial.println("Connecting To Wifi");
  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Wifi Connected");
  /*Serial.println(WiFi.SSID());
  Serial.println(WiFi.RSSI());
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.dnsIP());
  */

}

void on_message(const char *topic, byte *payload, unsigned int length) {
  Serial.println("On message");
  char json[length + 1];
  strncpy(json, (char *)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);
}
