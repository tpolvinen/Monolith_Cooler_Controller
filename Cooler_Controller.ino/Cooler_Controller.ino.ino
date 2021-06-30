#include <Adafruit_SleepyDog.h>
#include <SPI.h>
#include <Controllino.h>
//#define DEBUG
#include <DebugMacros.h>
#include <Wire.h>
#include <OneWire.h> // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <avr/wdt.h>
#include <LiquidCrystal.h>

const int8_t potPin = CONTROLLINO_A6;
const int8_t rs = CONTROLLINO_D13,
             en = CONTROLLINO_D12,
             d4 = CONTROLLINO_D11,
             d5 = CONTROLLINO_D10,
             d6 = CONTROLLINO_D9,
             d7 = CONTROLLINO_D8;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int16_t sensorReadingIntervalSeconds = 10;
const int16_t numberOfSensorReadingsPerMeasurement = 10;

#define ONE_WIRE_BUS6   CONTROLLINO_D6
#define ONE_WIRE_BUS7   CONTROLLINO_D7
OneWire oneWire_one(ONE_WIRE_BUS6);
OneWire oneWire_two(ONE_WIRE_BUS7);
DallasTemperature sensor_one(&oneWire_one);
DallasTemperature sensor_two(&oneWire_two);

// set global sensor resolution to 9, 10, 11, or 12 bits
const int8_t resolution = 12;

void error(char *str)
{
  DPRINT("error: ");
  DPRINTLN(str);
  while (1);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  lcd.begin(20, 4);
  lcd.print("hello, world!");

  int countdownMS = Watchdog.enable(4000);
  Watchdog.reset();

  DPRINTLN();
  DPRINTLN("------------------------------------------------------------");
  DPRINTLN("              Cooler Controller starting!");
  DPRINTLN();
  DPRINT("     Enabled the watchdog with max countdown of ");
  DPRINT(countdownMS, DEC);
  DPRINTLN(" ms");
  DPRINTLN("------------------------------------------------------------");
  DPRINTLN();

  int8_t setupPotValue = analogRead(potPin);
  DPRINT("Checking pot value: ");
  DPRINTLN(setupPotValue);

  float temperature;
  sensor_one.begin();
  sensor_two.begin();
  Watchdog.reset();

  sensor_one.setResolution(resolution);
  sensor_one.requestTemperatures();
  DPRINT("Initializing sensor_one... temperature: ");
  DPRINT(sensor_one.getTempCByIndex(0));
  DPRINTLN(" ...Done.");

  sensor_two.setResolution(resolution);
  sensor_two.requestTemperatures();
  DPRINT("Initializing sensor_two... temperature: ");
  DPRINT(sensor_two.getTempCByIndex(0));
  DPRINTLN(" ...Done.");



  //int16_t setupSensorReadings[numberOfSensorReadingsPerMeasurement];


}

void loop() {

  Watchdog.reset();
  int16_t potValue = analogRead(potPin);
  Serial.println(potValue);
  
  int16_t sensorReadings[numberOfSensorReadingsPerMeasurement];
  const unsigned long sensorReadingIntervalMilliseconds = sensorReadingIntervalSeconds * 1000;

  for (int8_t i = 0; i < numberOfSensorReadingsPerMeasurement; i++) {
    // int16_t sensorReading =
  }



}
