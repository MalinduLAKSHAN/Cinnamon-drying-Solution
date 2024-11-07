#define BLYNK_TEMPLATE_ID "TMPL6BPjrH0TJ"
#define BLYNK_TEMPLATE_NAME "CINNAMON DRYING"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define BLYNK_AUTH_TOKEN "SsJ4EZOcEMYqewt5XxD6yuMR-Zom_Bc_"
char auth[] = BLYNK_AUTH_TOKEN; 
char ssid[] = "Malindu’s iPhone";
char pass[] = "12345678";


#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#define RELAY_FAN 4
#define RELAY_HEATER 3


unsigned long previousMillis = 0;
unsigned long fanOnMillis = 0;
unsigned long heaterOnMillis = 0;
unsigned long fanOffMillis = 0;
unsigned long heaterOffMillis = 0;
bool fanState = LOW;
bool heaterState = LOW;
bool fanOffState = false;
bool heaterOffState = false;

const long fanOnDuration = 20000;
const long fanOnHighTempDuration = 50000;
const long heaterOnDuration = 1000;
const long fanOffDuration = 3000;
const long heaterOffDuration = 8000;


BlynkTimer timer;

void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  controlRelays(temperature, humidity);
}

void controlRelays(float temperature, float humidity) {
  unsigned long currentMillis = millis();

  if (temperature > 38) {
    digitalWrite(RELAY_HEATER, LOW);

    if (!fanState) {
      fanOnMillis = currentMillis;
      fanState = true;
    }

    if (currentMillis - fanOnMillis >= fanOnHighTempDuration) {
      digitalWrite(RELAY_FAN, LOW);
    } else {
      digitalWrite(RELAY_FAN, HIGH);
    }
    return;
  }

  if (humidity > 69) {
    digitalWrite(RELAY_FAN, LOW);

    if (heaterOffState) {
      if (currentMillis - heaterOffMillis >= heaterOffDuration) {
        heaterOffState = false;
      } else {
        digitalWrite(RELAY_HEATER, LOW);
        return;
      }
    }

    if (!heaterState) {
      heaterOnMillis = currentMillis;
      heaterState = true;
    }

    if (currentMillis - heaterOnMillis >= heaterOnDuration) {
      digitalWrite(RELAY_HEATER, LOW);
      heaterState = false;
      heaterOffMillis = currentMillis;
      heaterOffState = true;
    } else {
      digitalWrite(RELAY_HEATER, HIGH);
    }
  } else if (humidity < 60) {
    digitalWrite(RELAY_HEATER, LOW);

    if (fanOffState) {
      if (currentMillis - fanOffMillis >= fanOffDuration) {
        fanOffState = false;
      } else {
        digitalWrite(RELAY_FAN, LOW);
        return;
      }
    }

    if (!fanState) {
      fanOnMillis = currentMillis;
      fanState = true;
    }

    if (currentMillis - fanOnMillis >= fanOnDuration) {
      digitalWrite(RELAY_FAN, LOW);
      fanState = false;
      fanOffMillis = currentMillis;
      fanOffState = true;
    } else {
      digitalWrite(RELAY_FAN, HIGH);
    }
  } else {
    digitalWrite(RELAY_FAN, LOW);
    digitalWrite(RELAY_HEATER, LOW);
    fanState = false;
    heaterState = false;
    fanOffState = false;
    heaterOffState = false;
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  dht.begin();

  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_HEATER, OUTPUT);
  digitalWrite(RELAY_FAN, LOW);
  digitalWrite(RELAY_HEATER, LOW);

  timer.setInterval(1000L, sendSensorData);

  lcd.setCursor(0, 0);
  lcd.print("Power on");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("Please wait...");
  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  timer.run();
}
