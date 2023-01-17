//Deklarasi Library
#include <WiFi.h>
#include "DHTesp.h"
#include "ThingSpeak.h"
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>

//Deklrasi Pin
const int DHT_PIN = 15;
const int ledGreenPin = 4;
const int ledRedPin = 5;
const int servoPin = 16;

//Deklarasi Konst WiFi
const char* ssid = "Wokwi-GUEST";
const char* pass = "";

//Deklarasi Thingspeak
unsigned long myChannelNumber = 2;
unsigned long ChannelNumber =  1989975; 
const char* myWriteAPIKey = "0BSTTZ7JWA9OMDPU";     // Nomor Write API Key
const char* myReadAPIKey = "I89HQQBSEOOJD097";      // Nomor Read API key
const char* server = "api.thingspeak.com";
const int FieldNumber2 = 2;                         // Nomor Field 2 (beda channel)

//Deklarasi Delay Ukuran
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

//variabel Suhu
int temperatureC;
int humidity;

//Deklrasi Kontanta
Servo servo1;                      //nama servo [deprecated]
DHTesp dhtSensor;                  //nama dht22
WiFiClient client;                 //klien wifi

//Deklarasi Variabel
int valPotentiometer;              //Untuk Potensiometer

// Variabel Logika Suhu
const int maximumTemp = 45;        //Menentukan Temp Maks
const int minimumTemp = 25;        //Menentukan Temp Min
String valueStatus = "";           //Mengembalikan nilai normal dalam bentuk string kosong

void setup() {
  //Deklarasi Serial
  Serial.begin(115200);

  //Deklrasi Pin LED Green dan Red
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledRedPin, OUTPUT);

  //Deklarasi Pin DHT22
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  dhtSensor.getPin();
  delay(10);

  //Hubung Wifi
  WiFi.begin(ssid, pass);
  while(WiFi.status() !=WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("-> WiFi Connected!");
  Serial.println(WiFi.localIP());

  //Beralih Mode Wi-Fi
  WiFi.mode(WIFI_STA);

  //Inisialisasi Klien ThingSpeak
  ThingSpeak.begin(client);

  //Deklrasi Pin untuk Servo
  servo1.attach(servoPin);

  //Deklarasi Potensiometer
  pinMode(2, INPUT);
}

void loop() {

  //Pembacaan Suhu dan Kelembapan
  temperatureC = dhtSensor.getTemperature();
  Serial.print("Temperature (C) : ");
  Serial.print(temperatureC);

  //Kelempaban
  humidity = dhtSensor.getHumidity();
  Serial.print("  Humidity (%) : ");
  Serial.println(humidity);

  //Logika penentuan aktuator dan penyalaan lampu merah sebagai peringatan
  if (temperatureC >= maximumTemp || temperatureC <= minimumTemp) {
    valueStatus = "Tidak Normal";
    Serial.println("Kondisi Tidak Normal");
    digitalWrite(ledRedPin, HIGH);
  } else {
    valueStatus = "Normal";
    digitalWrite(ledRedPin, LOW);
    Serial.println("Kondisi Normal");
  }

  //Pilih Field ThingSpeak
  ThingSpeak.setField(1, temperatureC);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, valueStatus);

  //Upload ke Thingspeak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update Succcessful");
  } else {
    Serial.println("Prboblem updating channel. HTTP error code " + String(x));
  }

  //Penyalaan Lampu
  int A = ThingSpeak.readLongField(ChannelNumber, FieldNumber2, myReadAPIKey);
  if(A == 1){
    Serial.println("Lampu 1 Nyala");
    digitalWrite(ledGreenPin, HIGH);
  } else if(A == 0) {
    Serial.println("Lampu 1 Mati");
    digitalWrite(ledGreenPin, LOW);
  }

  //Pembacaan Potensiometer dan Kendali Servo
  valPotentiometer = analogRead(2);                           //Baca potensio
  valPotentiometer = map(valPotentiometer, 0, 1023, 0, 180);  //Konversi dari 10bit ke 180*
  servo1.write(valPotentiometer);                             //Jalankan servo sesuai posisi
  delay(15);                                                  //Lambatkan sedikit
}