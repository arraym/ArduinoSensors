/***************************************************************************
  Example testing sketch for various sensors:
  - CCS811 air sensor;
  - BMP280 humidity, temperature & pressure sensor
  - DHT22 humidity/temperature sensor
  - PMS5003 particle concentration sensor
  
  This sketch reads the sensors

  Designed specifically to work with the Adafruit CCS811 breakout
  ----> http://www.adafruit.com/products/3566
  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651
  Designed specifically to work with the Adafruit WIRED DHT22 temperature/humidity sensor
  ----> https://www.adafruit.com/product/393
  Designed specifically to work with the Plantower PM2.5 PMS5003 particle sensor
  ----> http://www.plantower.com/en/content/?108.html

  Sensors communication protocols:
  CCS811  - I2C (address: 0x5A)
  BMP280  - SPI
  DHT22   - 1WIRE
  PMS5003 - UART (baudrate: 9600)
 ***************************************************************************/

#include "Adafruit_CCS811.h"  // https://github.com/adafruit/Adafruit_CCS811
#include <Adafruit_BMP280.h>  // https://github.com/adafruit/Adafruit_BMP280_Library
#include "DHT.h"              // https://github.com/adafruit/DHT-sensor-library
#include "PMS.h"              // https://github.com/fu-hsi/pms
  
#include <SD.h>               // https://www.arduino.cc/en/Reference/SD
#include <RTCZero.h>          // https://www.arduino.cc/en/Reference/RTC

#define BMP_CS      7         // nCS (chip select) for BMP sensor
#define DHTPIN      2         // Digital pin connected to the DHT sensor
#define DHTTYPE     DHT22     // DHT 22  (AM2302)

#define DEBUG_MODE  3         // Digital debug pin => connected to GND deactivates standby mode
#define INTERVAL    10        // Time interval between readings

Adafruit_CCS811 ccs;
Adafruit_BMP280 bmp(BMP_CS);  // hardware SPI
DHT dht(DHTPIN, DHTTYPE);
PMS pms(Serial1);
PMS::DATA data;

// set up variables using the SD utility library functions:
const int chipSelect = 4;     // MKR SD PROTO SHIELD chip select pin
File myFile;                  // SD card file 
bool SDInitFlag = false;      // shows if SD card was successfuly initialized

String sensorsData = "";      // sensors data string to write on card and for http request

// RTC section
RTCZero rtc;
/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 30;
const byte hours = 15;
/* Change these values to set the current initial date */
const byte day = 20;
const byte month = 6;
const byte year = 21;

void setup() {
  pinMode(DEBUG_MODE, INPUT);     // configure pin as imput
  digitalWrite(DEBUG_MODE, HIGH); // activate pull-up resistor 

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
  {
    Serial.begin(9600);
    while(!Serial);             // wait for serial to connect
    Serial.println("CCS811, BMP280, DHT22, PMS5003 test");
  }
    
  Serial1.begin(9600);
  
  if(!ccs.begin()){
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
      Serial.println("Failed to start CCS811! Please check your wiring.");
    while(1) delay(10);
  }
  // Wait for the CCS811 sensor to be ready
  while(!ccs.available())
  {
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
      Serial.print(".");
    delay(1000);
  }
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    Serial.println();
  
  if (!bmp.begin()) {
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
      Serial.println("Could not find a valid BMP280 sensor! Please check your wiring.");
    while(1) delay(10);
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  dht.begin();

  // SD card initialization section
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
  {
    Serial.println();
    Serial.println("Initializing SD card...");
  }
  if(!SD.begin(chipSelect)) 
  {
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
      Serial.println("Initialization failed!");
  }
  else
  {
    SDInitFlag = true;
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    {
      Serial.println("Initialization done.");
      Serial.println();
      // delete the old data file if you want a fresh start
      if(SD.exists("datalog.txt"))
      {
        Serial.println("Removing datalog.txt...");
        SD.remove("datalog.txt");
        Serial.println();
      }
    }
  }

  // RTC section
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  setNewAlarm();
  rtc.attachInterrupt(alarmMatch);
}

void loop() {
  // CCS811 section
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    Serial.println("CCS811:");
  if(ccs.available()){
    if(!ccs.readData()){
      uint16_t co2 = ccs.geteCO2();
      uint16_t tvoc = ccs.getTVOC();
      // add ccs811 data to string
      sensorsData = "&CO2=" + String(co2, DEC);
      sensorsData += "&TVOC=" + String(tvoc, DEC);
      if(!digitalRead(DEBUG_MODE))    // debug mode enabled
      {
        Serial.print("CO2 = ");
        Serial.print(co2);
        Serial.print("ppm, TVOC = ");
        Serial.println(tvoc);
      }
    }
    else
    {
      if(!digitalRead(DEBUG_MODE))    // debug mode enabled
        Serial.println("Failed to read from CCS811 sensor!");
    }
  }
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    Serial.println();
  
  // BMP280 section
  float temp_bmp280 = bmp.readTemperature();
  float pressure_bmp280 = bmp.readPressure();
  float altitude_bmp280 = bmp.readAltitude(1013.25);
  // add bmp280 data to string
  sensorsData += "&temp_bmp280=" + String(temp_bmp280, 2);
  sensorsData += "&pressure=" + String(pressure_bmp280, 2);
  sensorsData += "&altitude_bmp280=" + String(altitude_bmp280, 2);
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
  {
    Serial.println("BMP280:");
    Serial.print("Temperature = ");
    Serial.print(temp_bmp280);
    Serial.println(" °C");
  
    Serial.print("Pressure = ");
    Serial.print(pressure_bmp280);
    Serial.println(" Pa");
  
    Serial.print("Approx altitude = ");
    Serial.print(altitude_bmp280); /* Adjusted to local forecast! */
    Serial.println(" m");
    Serial.println();
  }

  // DHT22 section
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity_dht22 = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temp_dht22 = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity_dht22) || isnan(temp_dht22)) {
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
      Serial.println("Failed to read from DHT sensor!");
  }
  else
  {
    // add dht22 data to string
    sensorsData += "&humidity=" + String(humidity_dht22, 2);
    sensorsData += "&temperature=" + String(temp_dht22, 2);
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    {
      Serial.println("DHT22:");
      Serial.print("Humidity = ");
      Serial.print(humidity_dht22);
      Serial.print("%  Temperature = ");
      Serial.print(temp_dht22);
      Serial.println("°C");
    }
  }
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
  {
    Serial.println();
  
    // PMS5003 section
    Serial.println("PMS5003:");
  }
  if (pms.readUntil(data, 2000))
  {
    uint16_t pm1_0 = data.PM_AE_UG_1_0;
    uint16_t pm2_5 = data.PM_AE_UG_2_5;
    uint16_t pm10_0 = data.PM_AE_UG_10_0;
    // add pms5003 data to string
    sensorsData += "&pm1.0=" + String(pm1_0, DEC);
    sensorsData += "&pm2.5=" + String(pm2_5, DEC);
    sensorsData += "&pm10.0=" + String(pm10_0, DEC);
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    {
      Serial.print("PM 1.0 = ");
      Serial.print(pm1_0);
      Serial.println("ug/m3");
  
      Serial.print("PM 2.5 = ");
      Serial.print(pm2_5);
      Serial.println("ug/m3");
  
      Serial.print("PM 10.0 = ");
      Serial.print(pm10_0);
      Serial.println("ug/m3");
    }
  }
  else
  {
    if(!digitalRead(DEBUG_MODE))    // debug mode enabled
      Serial.println("Failed to read from PMS5003 sensor!");
  }
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    Serial.println();
  
  // write sensors data string to datalog.txt file on SDcard
  if(SDInitFlag)
  {
    myFile = SD.open("datalog.txt", FILE_WRITE);
    if(myFile)
    {
      if(!digitalRead(DEBUG_MODE))    // debug mode enabled
        Serial.println("Writing data to datalog.txt...");
      myFile.println(sensorsData);
      myFile.close();
    }
    else
    {
      if(!digitalRead(DEBUG_MODE))    // debug mode enabled
        Serial.println("Error opening datalog.txt");
    }
  }
  if(!digitalRead(DEBUG_MODE))    // debug mode enabled
    Serial.println();

  if(digitalRead(DEBUG_MODE))    // if not in debug mode
  {
    digitalWrite(LED_BUILTIN, LOW);
    rtc.standbyMode();            // go to standby mode
  }
  else
  {
    delay(2000);
  }
}

/*
 * Attach interrupt function
 */
void alarmMatch()
{
  digitalWrite(LED_BUILTIN, HIGH);
  setNewAlarm();
}

/*
 * Adds a time interval to corrent time and sets an alarm 
 */
void setNewAlarm()
{
  uint32_t alarmTimestamp = rtc.getEpoch() + INTERVAL;
  rtc.setAlarmEpoch(alarmTimestamp);
  rtc.enableAlarm(rtc.MATCH_YYMMDDHHMMSS);
}
