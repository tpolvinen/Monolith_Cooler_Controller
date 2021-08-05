// New Cooler Controller!
// TODO: re-arrange lcd layout to make some sense

#include <EEPROM.h>
#include <Adafruit_SleepyDog.h>
#include <SPI.h>
#include <Controllino.h>
//#define DEBUG
#include <DebugMacros.h>
#include <Wire.h>
#include <Encoder.h>
#include <OneWire.h> // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <avr/wdt.h>
#include <LiquidCrystal_I2C.h>

uint32_t sensorReadingInterval = 1000;
uint32_t nextSensorReadingMs;
uint32_t controlChangeInterval = 2000; //5*60*1000; // =five minutes
uint32_t nextControlChangeMs;
uint32_t fanShutOffMs;
uint32_t fanShutOffDelay = 5000; //20*60*1000; // =20 minutes
int16_t eepromAddr = 0;
int16_t setTempC;
byte setTempByte;
float temperature1;
float temperature2;
float previousTemperature1;
float previousTemperature2;
int8_t encoderSwitchPin = CONTROLLINO_D2;
int8_t encoderSwitchState = HIGH;
uint32_t encoderSwitchLowMs;
int32_t encoderValue;
int32_t previousEncoderValue;

Encoder myEnc(CONTROLLINO_IN0, CONTROLLINO_IN1);
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define ONE_WIRE_BUS0 CONTROLLINO_D0
#define ONE_WIRE_BUS1 CONTROLLINO_D1

OneWire oneWire_one(ONE_WIRE_BUS0);
OneWire oneWire_two(ONE_WIRE_BUS1);

DallasTemperature sensor_one(&oneWire_one);
DallasTemperature sensor_two(&oneWire_two);

#define COOLANT_FLOW_VALVE CONTROLLINO_R0
#define FAN_RELAY CONTROLLINO_R1

// set global sensor resolution to 9, 10, 11, or 12 bits
const int8_t resolution = 12;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  pinMode(encoderSwitchPin, INPUT_PULLUP);
  pinMode(COOLANT_FLOW_VALVE, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);

  Wire.begin();

  lcd.init();
  lcd.backlight();

  int16_t countdownMS = Watchdog.enable(4000);
  Watchdog.reset();

  setTempByte = EEPROM.read(eepromAddr);
  setTempC = setTempByte - 127;

  sensor_one.begin();
  sensor_two.begin();

  Watchdog.reset();

  lcd.setCursor(0, 0);
  lcd.print("T1:       T2:");
  lcd.setCursor(0, 1);
  lcd.print("T1 AIM:");
  lcd.setCursor(0, 2);
  lcd.print("VALVE:");
  lcd.setCursor(0, 3);
  lcd.print("FAN:OFF  T NEW:");

  encoderValue = myEnc.read() / 2;
  lcd.setCursor(15, 3);
  lcd.print(encoderValue);
  previousEncoderValue = encoderValue;

  sensor_one.setResolution(resolution);
  sensor_two.setResolution(resolution);

  sensor_one.requestTemperatures();
  temperature1 = sensor_one.getTempCByIndex(0);
  lcd.setCursor(3, 0);
  lcd.print(temperature1);
  previousTemperature1 = temperature1;

  sensor_two.requestTemperatures();
  temperature2 = sensor_two.getTempCByIndex(0);
  lcd.setCursor(13, 0);
  lcd.print(temperature2);
  previousTemperature2 = temperature2;

  nextSensorReadingMs = millis() + sensorReadingInterval;

  lcd.setCursor(7, 1);
  lcd.print(setTempC);

  if (temperature1 > setTempC) {
    digitalWrite(COOLANT_FLOW_VALVE, HIGH);
    lcd.setCursor(6, 2);
    lcd.print("ON ");
    digitalWrite(FAN_RELAY, HIGH);
    lcd.setCursor(4, 3);
    lcd.print("ON ");
  } else {
    digitalWrite(COOLANT_FLOW_VALVE, LOW);
    lcd.setCursor(6, 2);
    lcd.print("OFF");
    digitalWrite(FAN_RELAY, LOW);
    lcd.setCursor(4, 3);
    lcd.print("OFF");
  }

}

void loop() {
  Watchdog.reset();
  encoderValue = myEnc.read() / 2;
  if (encoderValue != previousEncoderValue) {
    lcd.setCursor(15, 3);
    lcd.print("     ");
    lcd.setCursor(15, 3);
    lcd.print(encoderValue);
    previousEncoderValue = encoderValue;
  }

  int8_t prevEncoderSwitchState = encoderSwitchState;
  encoderSwitchState = digitalRead(encoderSwitchPin);
  if (prevEncoderSwitchState == HIGH && encoderSwitchState == LOW) {
    encoderSwitchLowMs = millis();
  } else if (prevEncoderSwitchState == LOW && encoderSwitchState == HIGH) {
    if (millis() - encoderSwitchLowMs < 50) {
      // ignoring debounce
    }
    else {
      if (encoderValue < -127) {
        setTempC = -127;
      } else if (encoderValue > 128) {
        setTempC = 128;
      } else {
        setTempC = encoderValue;
      }
      byte setTempByte = setTempC + 127;
      EEPROM.write(eepromAddr, setTempByte);
      lcd.setCursor(7, 1);
      lcd.print("     ");
      lcd.setCursor(7, 1);
      lcd.print(setTempC);
    }
  }

  if (millis() >= nextSensorReadingMs) {
    Watchdog.reset();
    sensor_one.requestTemperatures();
    temperature1 = sensor_one.getTempCByIndex(0);
    if (temperature1 != previousTemperature1) {
      lcd.setCursor(3, 0);
      lcd.print("      ");
      lcd.setCursor(3, 0);
      lcd.print(temperature1);
      previousTemperature1 = temperature1;
    }
    sensor_two.requestTemperatures();
    temperature2 = sensor_two.getTempCByIndex(0);
    if (temperature2 != previousTemperature2) {
      lcd.setCursor(13, 0);
      lcd.print("      ");
      lcd.setCursor(13, 0);
      lcd.print(temperature2);
      previousTemperature2 = temperature2;
    }
    nextSensorReadingMs = millis() + sensorReadingInterval;
  }

  if (millis() >= nextControlChangeMs) {
    if (temperature1 > setTempC) {
      digitalWrite(COOLANT_FLOW_VALVE, HIGH);
      lcd.setCursor(6, 2);
      lcd.print("ON ");
      digitalWrite(FAN_RELAY, HIGH);
      lcd.setCursor(4, 3);
      lcd.print("ON ");
      fanShutOffMs = millis() + fanShutOffDelay;
    } else {
      digitalWrite(COOLANT_FLOW_VALVE, LOW);
      lcd.setCursor(6, 2);
      lcd.print("OFF");
      if (millis() >= fanShutOffMs) {
        digitalWrite(FAN_RELAY, LOW);
        lcd.setCursor(4, 3);
        lcd.print("OFF");
      }
    }
    nextControlChangeMs = millis() + controlChangeInterval;
  }
}
