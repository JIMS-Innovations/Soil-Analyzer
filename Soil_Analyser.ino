#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


#define SIM_RST    12 ///< SIM808 RESET
#define SIM_RX    9 ///< SIM808 RXD
#define SIM_TX    8 ///< SIM808 TXD
#define SIM_PWR   10 ///< SIM808 PWRKEY
#define SIM_STATUS  11 ///< SIM808 STATUS


#define SIM808_BAUDRATE 9600    ///< Control the baudrate use to communicate with the SIM808 module
#define SERIAL_BAUDRATE 9600   ///< Controls the serial baudrate between the arduino and the computer

#define NL  "\n"

float lat;
float lon;
SoftwareSerial simSerial(SIM_TX, SIM_RX);

// Indoor Weather Station 1************************
char *indoor_1[] = {
  "nitrogen",
  "phosphorus",
  "potassiun",
  "soil-moisture",
  "temperature",
  "humidity",
  "uv",
  "BBFF-YXiQxsPUfhtWW1lKekGu49rgAEnkM9",
  "soil-analyser"
};

int W_led = LED_BUILTIN;

// NPK Sensor Setup----------------
#define RE_DE_1 7

// Soil pH Sensor Setup------------
#define RE_DE_2 6

// Soil Moisture Sensor Setup------
#define moisturePin A0

// UV Sensor Setup-----------------
#define uvPin A1

// Variables----------------------
String apn = "web.gprs.mtnnigeria.net";
String uvIndex;
float Sph = 0;
const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};
const byte ph[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
byte values[11];

SoftwareSerial s_pH(2, 3);
SoftwareSerial npk(4, 5);
//SoftwareSerial simSerial(8, 9);
// DHT Setup----------------------
#define DHTPIN 18      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  s_pH.begin(4800);
  npk.begin(4800);
  simSerial.begin(9600);
  dht.begin();
  pinMode(RE_DE_1, OUTPUT);
  pinMode(RE_DE_2, OUTPUT);
  pinMode(W_led, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A5, OUTPUT);
  digitalWrite(A3, HIGH);
  digitalWrite(A5, LOW);

}

void loop() {
  int temp, humidity;
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    temp = event.temperature;
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    int humidity = event.relative_humidity;
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }
  delay(1000);
  int nit, pho, pot;
  nit = nitrogen();
  delay(250);
  pho = phosphorous();
  delay(250);
  pot = potassium();
  delay(250);
  float soil_ph = soil_pH() / 10;
  float soil_moisture = moisture();
  String UV = uv();

  String token = String(indoor_1[7]);
  String device = String(indoor_1[8]);

  Serial.print("Soil Ph: ");
  Serial.println(soil_ph, 1);
  Serial.print("Nitrogen: ");
  Serial.print(nit);
  Serial.println(" mg/kg");
  Serial.print("Phosphorous: ");
  Serial.print(pho);
  Serial.println(" mg/kg");
  Serial.print("Potassium: ");
  Serial.print(pot);
  Serial.println(" mg/kg");
  delay(250);
  simSerial.listen();
  delay(1000);
  SIM_PowerOn();
  delay(500);
  if(simSerial.isListening()) {
    gprs(nit, pho, pot, soil_moisture, temp, humidity, UV, lat, lon, apn, token, device);
  }

}

//Custom Functions------
int nitrogen() {
  digitalWrite(RE_DE_1, HIGH);
  delay(10);
  npk.listen();
  delay(1000);
  if(npk.isListening()) {
    if (npk.write(nitro, sizeof(nitro)) == 8) {
      digitalWrite(RE_DE_1, LOW);
      delay(500);
      for (int i = 0; i < 11; i++)
      {
        values[i] = npk.read();
        Serial.print(values[i], HEX);
      }
    }
    Serial.println();
  }
  return int(values[4]);
}

int phosphorous() {
  digitalWrite(RE_DE_1, HIGH);
  delay(10);
  npk.listen();
  delay(500);
  if(npk.isListening()) {
    if (npk.write(phos, sizeof(phos)) == 8) {
      digitalWrite(RE_DE_1, LOW);
      delay(500);
      for (int i = 0; i < 11; i++)
      {
        values[i] = npk.read();
        Serial.print(values[i], HEX);
      }
    }
    Serial.println();
  }
  return int(values[4]);
}

int potassium() {
  digitalWrite(RE_DE_1, HIGH);
  delay(10);
  npk.listen();
  delay(500);
  if(npk.isListening()) {
    if (npk.write(pota, sizeof(pota)) == 8) {
      digitalWrite(RE_DE_1, LOW);
      delay(500);
      for (int i = 0; i < 11; i++)
      {
        values[i] = npk.read();
        Serial.print(values[i], HEX);
      }
    }

    Serial.println();
  }
  return int(values[4]);
}

int soil_pH() {
  digitalWrite(RE_DE_2, HIGH);
  delay(10);
  s_pH.listen();
  delay(1000);
  if(s_pH.isListening()) {
    if (s_pH.write(ph, sizeof(ph)) == 8)
    {
      digitalWrite(RE_DE_2, LOW);
      delay(500);
      for (int i = 0; i < 11; i++)
      {
        values[i] = s_pH.read();
        Serial.print(values[i], HEX);
      }
    }

    Serial.println();
  }

  return int(values[4]);
}

int moisture()
{
  int moist = analogRead(moisturePin);
  int moisture = map(moist, 550, 0, 0, 100);
  if (moisture < -1)
  {
    moisture = 0;
  }
  return moisture;
}

String uv()
{
  String UVIndex;
  int UVvalue = analogRead(uvPin);
  int voltage = (UVvalue * (5.0 / 1023.0)) * 1000; // Voltage in miliVolts

  if (voltage < 50)
  {
    UVIndex = "0";
  }
  else if (voltage > 50 && voltage <= 227)
  {
    UVIndex = "0";
  }
  else if (voltage > 227 && voltage <= 318)
  {
    UVIndex = "1";
  }
  else if (voltage > 318 && voltage <= 408)
  {
    UVIndex = "2";
  }
  else if (voltage > 408 && voltage <= 503)
  {
    UVIndex = "3";
  }
  else if (voltage > 503 && voltage <= 606)
  {
    UVIndex = "4";
  }
  else if (voltage > 606 && voltage <= 696)
  {
    UVIndex = "5";
  }
  else if (voltage > 696 && voltage <= 795)
  {
    UVIndex = "6";
  }
  else if (voltage > 795 && voltage <= 881)
  {
    UVIndex = "7";
  }
  else if (voltage > 881 && voltage <= 976)
  {
    UVIndex = "8";
  }
  else if (voltage > 976 && voltage <= 1079)
  {
    UVIndex = "9";
  }
  else if (voltage > 1079 && voltage <= 1170)
  {
    UVIndex = "10";
  }
  else if (voltage > 1170)
  {
    UVIndex = "11";
  }
  return UVIndex;
}

// SIM Module power ON************
void SIM_PowerOn(){
  simSerial.println("AT");
  delay(500);
  if(!simSerial.find("OK")){
    digitalWrite(SIM_PWR, HIGH);
    delay(1500);
    digitalWrite(SIM_PWR, LOW);
    delay(150);
  }
  
}

// Gprs Upload-------------------------------------------------------


void gprs(int nitrogen, int phosphorus , int potassium, int soil_moisture, int temperature, int humidity, String UV, float lat, float lon, String apn, String token, String device)
{
  if (simSerial.available())
    Serial.write(simSerial.read());

  simSerial.println("AT");
  delay(1000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(2000);

  simSerial.println("AT+CPIN?");
  delay(1000);
  
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(2000);

  simSerial.println("AT+CREG?");
  delay(1000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(2000);

  simSerial.println("AT+CGATT?");
  delay(1000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(2000);

  simSerial.println("AT+CIPSHUT");
  delay(1000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(2000);

  simSerial.println("AT+CSTT=\"" + apn + "\""); // start task and setting the APN,
  delay(1000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);

  simSerial.println("AT+CIICR"); // bring up wireless connection
  delay(3000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);

  simSerial.println("AT+CIFSR"); // get local IP adress
  delay(2000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);

  simSerial.println("AT+CIPSPRT=0");
  delay(3000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);

  simSerial.println("AT+CIPSTART=\"TCP\",\"things.ubidots.com\",80"); // start up the connection
  delay(6000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);

  simSerial.println("AT+CIPSEND"); // begin send data to remote server
  delay(4000);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);

  String value1 = "{\"" + String(indoor_1[0]) + "\": " + String(nitrogen) + ", \"" + String(indoor_1[1]) + "\": " + String(phosphorus) + ", \"" + String(indoor_1[2]) + "\": " + String(potassium);
  String value2 = ", \"" + String(indoor_1[3]) + "\": " + String(soil_moisture) + ", \"" + String(indoor_1[4]) + "\": " + String(temperature) + ", \"" + String(indoor_1[5]) + "\": " + String(humidity);
  String value3 = ", \"" + String(indoor_1[6]) + "\": " + UV + ", \"" + "location" + "\": \"context\" {\"lat\": " + String(lat) + "\"lng\": " + String(lon) + "} " + "}";
  int i = value1.length();
  int j = value2.length();
  int k = value3.length();
  // String str = "GET https://api.thingspeak.com/update?api_key=QPK8OIBKF34IDKFT&field1=" + String(temp1) + "&field2=" + String(temp2) + "&field3=" + String(hum1) + "&field4=" + String(hum2) + "&field5=" + String(co2) + "&field6=" + String(soilTemp) + "&field7=" + String(par);
  String str1 = "POST /api/v1.6/devices/" + device + "/ HTTP/1.1\r\n";
  String str2 = "Content-Type: application/json\r\nContent-Length: ";
  String str3 = String(i) + "\r\nX-Auth-Token: ";
  String str4 = token + "\r\n";
  String str5 = "Host: things.ubidots.com\r\n\r\n";
  // str+= value;
  // "User-Agent:" + USER_AGENT + "/" + VERSION + "\r\n"
  //"X-Auth-Token: " + *TOKEN + "\r\n"
  // "Connection: close\r\n"
  //"Content-Type: application/json\r\n"
  //"Content-Length: " + String(i) + "\r\n"
  // "\r\n"
  //+ value +
  //"\r\n";
  // Serial.println(str1 + str2 + str3 + str4 + value);
  // delay(1000);
  simSerial.print(" POST /api/v1.6/devices/");
  delay(1000);
  simSerial.print(device);
  delay(1000);
  simSerial.println("/HTTP/1.1");
  delay(1000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.println("Host: things.ubidots.com");
  delay(1000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.print("X-Auth-Token: ");
  delay(1000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.println(token);
  delay(1000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.println("Content-Type: application/json");
  delay(1000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.println("Content-Length: " + String(i + j + k));
  delay(1000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.println("");
  simSerial.print(value1);
  delay(2000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.print(value2);
  delay(2000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  simSerial.println(value3);
  delay(2000);
  simSerial.println("");
  delay(1000);
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(1000);

  simSerial.println((char)26); // sending
  delay(5000);               // waitting for reply, important! the time is base on the condition of internet
  simSerial.println();

//  if (simSerial.find("200"))
//  {
//    Serial.println("Upload Successful!");
//
//    digitalWrite(W_led, HIGH);
//    delay(500);
//    digitalWrite(W_led, LOW);
//    delay(500);
//  }
//  else
//  {
//    Serial.println("Upload Error!");
//
//    digitalWrite(W_led, HIGH);
//    delay(250);
//    digitalWrite(W_led, LOW);
//    delay(250);
//    digitalWrite(W_led, HIGH);
//    delay(250);
//    digitalWrite(W_led, LOW);
//    delay(250); 
//  }
  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);

  simSerial.println("AT+CIPSHUT"); // close the connection
  delay(100);

  while (simSerial.available() != 0)
    Serial.write(simSerial.read());
  delay(5000);
}

//void gprs(int nitrogen, int phosphorus , int potassium, int soil_moisture, int temperature, int humidity, String UV, float lat, float lon, String apn, String token, String device)
//{
//  if (simSerial.available())
//    Serial.write(simSerial.read());
//
//  simSerial.println("AT");
//  delay(1000);
//
//  simSerial.println("AT+CPIN?");
//  delay(1000);
//
//  simSerial.println("AT+CREG?");
//  delay(1000);
//
//  simSerial.println("AT+CGATT?");
//  delay(1000);
//
//  simSerial.println("AT+CIPSHUT");
//  delay(1000);
//
//  simSerial.println("AT+CSTT=\"" + apn + "\""); // start task and setting the APN,
//  delay(1000);
//
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//
//  simSerial.println("AT+CIICR"); // bring up wireless connection
//  delay(3000);
//
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//
//  simSerial.println("AT+CIFSR"); // get local IP adress
//  delay(2000);
//
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//
//  simSerial.println("AT+CIPSPRT=0");
//  delay(3000);
//
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//
//  simSerial.println("AT+CIPSTART=\"TCP\",\"things.ubidots.com\",80"); // start up the connection
//  delay(6000);
//
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//
//  simSerial.println("AT+CIPSEND"); // begin send data to remote server
//  delay(4000);
//
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//
//  String value1 = "{\"" + String(indoor_1[0]) + "\": " + String(nitrogen) + ", \"" + String(indoor_1[1]) + "\": " + String(phosphorus) + ", \"" + String(indoor_1[2]) + "\": " + String(potassium);
//  String value2 = ", \"" + String(indoor_1[3]) + "\": " + String(soil_moisture) + ", \"" + String(indoor_1[4]) + "\": " + String(temperature) + ", \"" + String(indoor_1[5]) + "\": " + String(humidity);
//  String value3 = ", \"" + String(indoor_1[6]) + "\": " + UV + ", \"" + "location" + "\": \"context\" {\"lat\": " + String(lat) + "\"lng\": " + String(lon) + "} " + "}";
//  int i = value1.length();
//  int j = value2.length();
//  int k = value3.length();
//  // String str = "GET https://api.thingspeak.com/update?api_key=QPK8OIBKF34IDKFT&field1=" + String(temp1) + "&field2=" + String(temp2) + "&field3=" + String(hum1) + "&field4=" + String(hum2) + "&field5=" + String(co2) + "&field6=" + String(soilTemp) + "&field7=" + String(par);
//  String str1 = "POST /api/v1.6/devices/" + device + "/ HTTP/1.1\r\n";
//  String str2 = "Content-Type: application/json\r\nContent-Length: ";
//  String str3 = String(i) + "\r\nX-Auth-Token: ";
//  String str4 = token + "\r\n";
//  String str5 = "Host: things.ubidots.com\r\n\r\n";
//  // str+= value;
//  // "User-Agent:" + USER_AGENT + "/" + VERSION + "\r\n"
//  //"X-Auth-Token: " + *TOKEN + "\r\n"
//  // "Connection: close\r\n"
//  //"Content-Type: application/json\r\n"
//  //"Content-Length: " + String(i) + "\r\n"
//  // "\r\n"
//  //+ value +
//  //"\r\n";
//  // Serial.println(str1 + str2 + str3 + str4 + value);
//  // delay(1000);
//  simSerial.println("POST /api/v1.6/devices/" + device + "/ HTTP/1.1");
//  delay(1000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.println("Host: things.ubidots.com");
//  delay(1000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.print("X-Auth-Token: ");
//  delay(1000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.println(token);
//  delay(1000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.println("Content-Type: application/json");
//  delay(1000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.println("Content-Length: " + String(i + j + k));
//  delay(1000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.println("");
//  simSerial.print(value1);
//  delay(2000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.print(value2);
//  delay(2000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  simSerial.println(value3);
//  delay(2000);
//  simSerial.println("");
//  delay(1000);
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(1000);
//
//  simSerial.println((char)26); // sending
//  delay(5000);               // waitting for reply, important! the time is base on the condition of internet
//  simSerial.println();
//
//  if (simSerial.find("200"))
//  {
//    Serial.println("Upload Successful!");
//
//    digitalWrite(W_led, HIGH);
//    delay(500);
//    digitalWrite(W_led, LOW);
//    delay(500);
//  }
//  else
//  {
//    Serial.println("Upload Error!");
//
//    digitalWrite(W_led, HIGH);
//    delay(250);
//    digitalWrite(W_led, LOW);
//    delay(250);
//    digitalWrite(W_led, HIGH);
//    delay(250);
//    digitalWrite(W_led, LOW);
//    delay(250);
//  }
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//
//  simSerial.println("AT+CIPSHUT"); // close the connection
//  delay(100);
//
//  while (simSerial.available() != 0)
//    Serial.write(simSerial.read());
//  delay(5000);
//}
