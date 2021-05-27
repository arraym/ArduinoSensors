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

#define BMP_CS    7           // nCS (chip select) for BMP sensor
#define DHTPIN    2           // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22         // DHT 22  (AM2302)

Adafruit_CCS811 ccs;
Adafruit_BMP280 bmp(BMP_CS);  // hardware SPI
DHT dht(DHTPIN, DHTTYPE);
PMS pms(Serial1);
PMS::DATA data;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  
  while(!Serial);             // wait for serial to connect
  Serial.println("CCS811, BMP280, DHT22, PMS5003 test");

  if(!ccs.begin()){
    Serial.println("Failed to start CCS811! Please check your wiring.");
    while(1) delay(10);
  }
  // Wait for the CCS811 sensor to be ready
  while(!ccs.available())
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  
  if (!bmp.begin()) {
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
}

void loop() {
  // CCS811 section
  Serial.println("CCS811:");
  if(ccs.available()){
    if(!ccs.readData()){
      Serial.print("CO2 = ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm, TVOC = ");
      Serial.println(ccs.getTVOC());
    }
    else{
      Serial.println("Failed to read from CCS811 sensor!");
    }
  }
  Serial.println();
  
  // BMP280 section
  Serial.println("BMP280:");
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  Serial.print("Approx altitude = ");
  Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
  Serial.println(" m");
  Serial.println();

  // DHT22 section
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else
  {
    Serial.println("DHT22:");
    Serial.print("Humidity = ");
    Serial.print(h);
    Serial.print("%  Temperature = ");
    Serial.print(t);
    Serial.println("°C");
  }
  Serial.println();

  // PMS5003 section
  Serial.println("PMS5003:");
  if (pms.readUntil(data, 2000))
  {
    Serial.print("PM 1.0 = ");
    Serial.print(data.PM_AE_UG_1_0);
    Serial.println("ug/m3");

    Serial.print("PM 2.5 = ");
    Serial.print(data.PM_AE_UG_2_5);
    Serial.println("ug/m3");

    Serial.print("PM 10.0 = ");
    Serial.print(data.PM_AE_UG_10_0);
    Serial.println("ug/m3");
  }
  else
  {
    Serial.println("Failed to read from PMS5003 sensor!");
  }
  Serial.println();
  
  delay(2000);
}
