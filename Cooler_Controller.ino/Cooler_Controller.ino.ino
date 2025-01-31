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

int16_t eepromAddr = 0;
int16_t setTempC;
byte setTempByte;
float temperature1;
float temperature2;

int8_t encoderSwitchPin = CONTROLLINO_D2;
int8_t encoderSwitchState = HIGH;
uint32_t encoderSwitchLowMs;
uint32_t showEepromStoreSign;

Encoder myEnc(CONTROLLINO_IN0, CONTROLLINO_IN1);
int32_t encoderValue;
//int32_t oldEncoderValue = -999;

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int16_t sensorReadingIntervalSeconds = 10;
const int16_t numberOfSensorReadingsPerMeasurement = 10;

#define ONE_WIRE_BUS0 CONTROLLINO_D0
#define ONE_WIRE_BUS1 CONTROLLINO_D1
#define COOLANT_FLOW_VALVE CONTROLLINO_R0
#define FAN_RELAY CONTROLLINO_R1

OneWire oneWire_one(ONE_WIRE_BUS0);
OneWire oneWire_two(ONE_WIRE_BUS1);
DallasTemperature sensor_one(&oneWire_one);
DallasTemperature sensor_two(&oneWire_two);

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
  lcd.setCursor(3, 0);
  lcd.print("Hello, world!");

  int16_t countdownMS = Watchdog.enable(4000);
  Watchdog.reset();

  setTempByte = EEPROM.read(eepromAddr);
  setTempC = setTempByte - 127;

  lcd.setCursor(3, 0);
  lcd.print("             ");
  lcd.setCursor(3, 0);
  lcd.print("Set Temp: ");
  lcd.print(setTempC);
  lcd.setCursor(3, 2);
  lcd.print("Dial: ");

  DPRINTLN();
  DPRINTLN("------------------------------------------------------------");
  DPRINTLN("              Cooler Controller starting!");
  DPRINTLN();
  DPRINT("     Enabled the watchdog with max countdown of ");
  DPRINT(countdownMS, DEC);
  DPRINTLN(" ms");
  DPRINTLN();
  DPRINT("Set Temp: ");
  DPRINTLN(setTempC);
  DPRINTLN("------------------------------------------------------------");
  DPRINTLN();

  sensor_one.begin();
  sensor_two.begin();
  Watchdog.reset();

  sensor_one.setResolution(resolution);
  sensor_one.requestTemperatures();
  lcd.setCursor(0, 1);
  lcd.print("T1: ");
  temperature1 = sensor_one.getTempCByIndex(0);
  lcd.print(temperature1);
  DPRINT("Initializing sensor_one. Temperature: ");
  DPRINTLN(temperature1);

  sensor_two.setResolution(resolution);
  sensor_two.requestTemperatures();
  lcd.print(" T2: ");
  temperature2 = sensor_two.getTempCByIndex(0);
  lcd.print(temperature2);
  DPRINT("Initializing sensor_two. Temperature: ");
  DPRINTLN(temperature2);

}

void loop() {

  Watchdog.reset();

  encoderValue = myEnc.read();
  Serial.println(encoderValue);

  lcd.setCursor(9, 2);
  lcd.print("       ");
  lcd.setCursor(9, 2);
  lcd.print(encoderValue);

  sensor_one.requestTemperatures();
  lcd.setCursor(4, 1);
  temperature1 = sensor_one.getTempCByIndex(0);
  lcd.print(temperature1);
  sensor_two.requestTemperatures();
  lcd.setCursor(14, 1);
  temperature2 = sensor_two.getTempCByIndex(0);
  lcd.print(temperature2);


  if (temperature1 > setTempC) {
    digitalWrite(COOLANT_FLOW_VALVE, HIGH);
    digitalWrite(FAN_RELAY, HIGH);
  } else {
    digitalWrite(COOLANT_FLOW_VALVE, LOW);
    digitalWrite(FAN_RELAY, LOW);
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
      lcd.setCursor(0, 3);
      lcd.print("Set Temp: ");
      lcd.print(setTempC);
      showEepromStoreSign = millis();
    }
  }

  if (millis() - showEepromStoreSign > 3000) {
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    lcd.setCursor(0, 0);
    lcd.print("                    ");
    lcd.setCursor(3, 0);
    lcd.print("Set Temp: ");
    lcd.print(setTempC);
  }


  //  int16_t sensorReadings[numberOfSensorReadingsPerMeasurement];
  //  const unsigned long sensorReadingIntervalMilliseconds = sensorReadingIntervalSeconds * 1000;
  //
  //  for (int8_t i = 0; i < numberOfSensorReadingsPerMeasurement; i++) {
  //
  //  }



}

// NOTES

//  TODO:
//    interval for turning fan off
//    interval for switching coolant valve
//
//  sensorReadinInterval (bool)
//  coolantValve_state (bool)
//  fan_state (bool)
//  dial_state (int16_t) ?
//  

// VARIABLES
//
//  uint32_t sensorReadingInterval;
//  int16_t eepromAddr = 0;
//  int16_t setTemp = 0;
//  float temperature1;
//  float temperature2;
//  int8_t encoderSwitchPin = CONTROLLINO_D2;
//  int8_t encoderSwitchState = HIGH;
//  uint32_t encoderSwitchLowMs;
//  uint32_t showEepromStoreSign;
//  int32_t encoderValue;
//  uint32_t fanOffDelayMs = 20*60*1000;
//   

// as soon as possible, read encoderValue, if changed, (multiply by 2?), rewrite dial number on display
// as soon as possible, check encoder switch, debounce, set setTemp, write to eeprom
// every one second, rewrite temperatures on display (set temp, T1, T2)
// every 5 minutes, change coolant valve state, unless set temp - T1 > 1C
// when coolant valve turns to long cycle (cooling), turn on fan
// after turning valve to short cycle (not cooling), keep fan going on 20 minutes?
// 
