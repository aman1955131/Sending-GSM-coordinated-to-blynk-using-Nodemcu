#define BLYNK_TEMPLATE_ID "TMPLQrPgX_tt"
#define BLYNK_DEVICE_NAME "Vehicle tracking"
#define BLYNK_AUTH_TOKEN "T07nq4Kjg2sFiyBpv13fsFA-k5qoKKlO"
#define BLYNK_PRINT Serial

#include <TinyGPS++.h>
#include<SoftwareSerial.h>
#include<BlynkSimpleEsp8266.h>
#include<ESP8266WiFi.h>

char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;
TinyGPSPlus gps;
WiFiClient client;

//WIFI Creds
const char* ssid     = "CHARGEKART";
const char* password = "CHARGEKARTD7";
int count = 0;               // count = 0
char input[12];              // character array of size 12
boolean flag = 0;            // flag =0
const int relay = D6;
const int relay2 = D7;
char id[13] = "180041C56BF7";  

//GPS constant
float latitude , longitude;
String  lat_str , lng_str;

//voltae sensor constants
float vout = 0.0;
float vin = 0.0;
float r1 = 30000 + 39000;
float r2 = 7500;
int vsensor = A0;
int val;
float correctionfactor = 9.29;

//gps pins and baud rate
static const uint32_t GPSBaud = 9600;
static const int RXPin = D4, TXPin = D5;

SoftwareSerial ss(RXPin, TXPin);

BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
  Serial.flush();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ss.begin(GPSBaud);

  Blynk.begin(auth, ssid, password);

  WiFi.mode(WIFI_STA);
  Serial.print("connected to ssid");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.flush();
  }

  pinMode(vsensor, A0);
  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);

  digitalWrite(relay, HIGH);
  digitalWrite(relay2, HIGH);
  Serial.flush();

}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  Serial.flush();
  upload2();
  Serial.flush();
  upload();
  Serial.flush();
  rfid();
  Serial.flush();
}

void upload() {
  while (ss.available() > 0) {
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
//        Serial.print("Latitude = ");
//        Serial.println(lat_str);
//        Serial.print("Longitude = ");
//        Serial.println(lng_str);
        Blynk.virtualWrite(V0, lat_str);
        Blynk.virtualWrite(V1, lng_str);
      }
      else {
        Serial.println("FAiled");
      }
      delay(1000);
//      Serial.println();
    }
  }
}

void upload2() {
  val = analogRead(vsensor);
  vout = (val * 5.0) / 1023.0;
  vin = vout / (r2 / (r1 + r2));
  vin = vin - correctionfactor;
//  Serial.println(vin);
  Blynk.virtualWrite(V5, vin);
}

void rfid(){
  while (Serial.available()){
    Serial.readString();
  }
  if (Serial.available())
  {
    count = 0;
    while (Serial.available() && count < 12)         // Read 12 characters and store them in input array
    {
      input[count] = Serial.read();
      count++;
      delay(5);
    }
    Serial.print(input);                             // Print RFID tag number


    if ((input[0] ^ input[2] ^ input[4] ^ input[6] ^ input[8] == input[10]) &&
        (input[1] ^ input[3] ^ input[5] ^ input[7] ^ input[9] == input[11])) {
      if (compare()) {
        Serial.println("No Error");
        digitalWrite(relay, !digitalRead(relay));
        delay(3000);
        serialflush();
      }
      else {
        digitalWrite(relay2, !digitalRead(relay2));
        delay(3000);
        serialflush();
      }
    }
    else {
      Serial.println("Error");
    }
  }
}

void serialflush() {
  while (Serial.available() > 0) {
    char t = Serial.read();
  }
}

bool compare() {
  for (int i = 0; i < 12; i++) {
    if (input[i] != id[i]) {
      return false;
    }
  }
  return true;
}
