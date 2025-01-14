#include <EEPROM.h>
#include "GravityTDS.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ----- Pin Tanımlamaları -----
#define TDS_SENSOR_PIN A0  // TDS sensörü pini
#define PH_SENSOR_PIN A1   // pH sensörü pini

#define RELAY_PUMP_A 49    // Plantoponic A pompası
#define RELAY_PUMP_B 46    // Plantoponic B pompası
#define RELAY_PUMP_C 47    // Sabit kapalı pompa
#define RELAY_PUMP_D 48    // Sabit kapalı pompa

#define ONE_WIRE_BUS 10    // DS18B20 sıcaklık sensörü
#define DHT_PIN 11         // DHT11 sensörü
#define DHT_TYPE DHT11

#define TRIG_PIN 8         // Mesafe sensörü trig pini
#define ECHO_PIN 9         // Mesafe sensörü echo pini

#define S0 7               // Renk sensörü frekans seçimi
#define S1 6
#define S2 4
#define S3 5
#define sensorOut 3

// ----- OLED Ayarları -----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----- Kalibrasyon Değerleri -----
#define PH_OFFSET -1.00  // pH sensörü kalibrasyon değeri

// ----- Değişkenler -----
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHT_PIN, DHT_TYPE);
GravityTDS gravityTds;

float waterTemp = 25.0;  // Suyun sıcaklığı
float ecValue = 0.0;     // EC değeri
float phValue = 0.0;     // pH değeri
float roomTemp = 0.0;    // Ortam sıcaklığı
float humidity = 0.0;    // Nem
float distance = 0.0;    // Mesafe
int greenFrequency = 0;  // Renk frekansı (yeşil)

// ----- Hedef Değerler -----
const float TARGET_EC = 1000.0;  // Hedef EC değeri (µS/cm)

// ----- Zamanlayıcılar -----
unsigned long lastFeedTime = 0;
const unsigned long feedInterval = 300000;  // 5 dakika (300,000 ms)
unsigned long lastSensorUpdate = 0;
const unsigned long sensorUpdateInterval = 10000;  // 10 saniye

void setup() {
  Serial.begin(9600);
  
  // TDS sensörü başlatma
  gravityTds.setPin(TDS_SENSOR_PIN);
  gravityTds.setAref(5.0);
  gravityTds.setAdcRange(1024);
  gravityTds.begin();

  // DS18B20 sıcaklık sensörü başlatma
  sensors.begin();

  // DHT11 başlatma
  dht.begin();

  // Röle pinlerini ayarla
  pinMode(RELAY_PUMP_A, OUTPUT);
  pinMode(RELAY_PUMP_B, OUTPUT);
  pinMode(RELAY_PUMP_C, OUTPUT);
  pinMode(RELAY_PUMP_D, OUTPUT);
  digitalWrite(RELAY_PUMP_A, LOW);
  digitalWrite(RELAY_PUMP_B, LOW);
  digitalWrite(RELAY_PUMP_C, LOW);
  digitalWrite(RELAY_PUMP_D, LOW);

  // Mesafe sensörü pinleri
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Renk sensörü pinleri
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);  // %20 çıkış frekansı

  // OLED başlatma
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Sistem Hazir");
  display.display();
  delay(1500);
}

void loop() {
  unsigned long currentMillis = millis();

  // Sensör güncellemesi (her 10 saniyede bir)
  if (currentMillis - lastSensorUpdate >= sensorUpdateInterval) {
    lastSensorUpdate = currentMillis;

    // Su sıcaklığı ölçümü
    sensors.requestTemperatures();
    waterTemp = sensors.getTempCByIndex(0);

    // TDS sensörü güncelleme
    gravityTds.setTemperature(waterTemp);
    gravityTds.update();
    ecValue = gravityTds.getEcValue();

    // pH sensöründen veri al
    phValue = readPH();

    // DHT11 ölçümleri
    roomTemp = dht.readTemperature();
    humidity = dht.readHumidity();

    // Mesafe ölçümü
    distance = readDistance();

    // Renk ölçümü
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    greenFrequency = pulseIn(sensorOut, LOW);

    // Seri monitöre yazdır
    Serial.println("----- Sensör Değerleri -----");
    Serial.print("EC Değeri: "); Serial.print(ecValue); Serial.println(" µS/cm");
    Serial.print("pH Değeri: "); Serial.println(phValue, 2);
    Serial.print("Su Sıcaklığı: "); Serial.print(waterTemp); Serial.println(" C");
    Serial.print("Oda Sıcaklığı: "); Serial.print(roomTemp); Serial.println(" C");
    Serial.print("Nem: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Mesafe: "); Serial.print(distance); Serial.println(" cm");
    Serial.print("Yeşil Frekans: "); Serial.println(greenFrequency);
    Serial.println("-----------------------------");
  }

  // EC kontrolü ve pompa çalıştırma
  if (ecValue < TARGET_EC && (currentMillis - lastFeedTime >= feedInterval)) {
    Serial.println("EC değeri düşük, besin ekleniyor...");

    // Plantoponic B pompasını çalıştır
    Serial.println("Besin A ekleniyor...");
    digitalWrite(RELAY_PUMP_B, HIGH);
    delay(2250);  // 2.25 saniye
    digitalWrite(RELAY_PUMP_B, LOW);

    delay(1000);  // 1 saniye bekle

    // Plantoponic A pompasını çalıştır
    Serial.println("Besin B ekleniyor...");
    digitalWrite(RELAY_PUMP_A, HIGH);
    delay(3800);  // 3.8
    digitalWrite(RELAY_PUMP_A, LOW);

    // Besin ekleme zamanını güncelle
    lastFeedTime = currentMillis;
    Serial.println("Besin karışımı için 5 dakika bekleniyor...");
  }
}

// ----- pH Sensörü Okuma -----
float readPH() {
  int buf[10];
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(PH_SENSOR_PIN);
    delay(10);
  }
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        int temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  unsigned long avgValue = 0;
  for (int i = 2; i < 8; i++) {
    avgValue += buf[i];
  }
  float voltage = (float)avgValue * 5.0 / 1024 / 6;
  return 3.5 * voltage + PH_OFFSET;
}

// ----- Mesafe Okuma -----
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distance = (duration / 2.0) * 0.0343;
  return (distance >= 2 && distance <= 450) ? distance : -1;
}