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

Encoder myEnc(CONTROLLINO_IN0, CONTROLLINO_IN1);
int32_t encoderValue = 0;
int32_t oldEncoderValue = -999;

LiquidCrystal_I2C lcd(0x27,20,4);

const int16_t sensorReadingIntervalSeconds = 10;
const int16_t numberOfSensorReadingsPerMeasurement = 10;

#define ONE_WIRE_BUS0   CONTROLLINO_D0
#define ONE_WIRE_BUS1   CONTROLLINO_D1
OneWire oneWire_one(ONE_WIRE_BUS0);
OneWire oneWire_two(ONE_WIRE_BUS1);
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

  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");

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
  DPRINT("Reading rotary encoder, value: ");
  DPRINTLN(encoderValue = myEnc.read());

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

}

void loop() {

  Watchdog.reset();

  encoderValue = myEnc.read();
  if (encoderValue != oldEncoderValue) {
    oldEncoderValue = encoderValue;
  }
  Serial.println(encoderValue);
  
  int16_t sensorReadings[numberOfSensorReadingsPerMeasurement];
  const unsigned long sensorReadingIntervalMilliseconds = sensorReadingIntervalSeconds * 1000;

  for (int8_t i = 0; i < numberOfSensorReadingsPerMeasurement; i++) {
    // int16_t sensorReading =
  }



}
