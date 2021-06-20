# **ArduinoSensors**


## **This sketch reads the sensors:**

* Designed specifically to work with the Adafruit CCS811 breakout
  * http://www.adafruit.com/products/3566
* Designed specifically to work with the Adafruit BMP280 Breakout
  * http://www.adafruit.com/products/2651
* Designed specifically to work with the Adafruit WIRED DHT22 temperature/humidity sensor
  * https://www.adafruit.com/product/393
* Designed specifically to work with the Plantower PM2.5 PMS5003 particle sensor
  * http://www.plantower.com/en/content/?108.html
		
		
## **Sensors communication protocols:**

* CCS811  - I2C (address: 0x5A)
* BMP280  - SPI
* DHT22   - 1WIRE
* PMS5003 - UART (baudrate: 9600)
					

## **SD card section:**

* MKR SD PROTO SHIELD chip select pin - 4
* Data string from sensors used for http request is stored on SD card (file: datalog.txt)
* Old data stored on SD card can be deleted uncommenting lines 95-100
* Stored data format: &CO2=409&TVOC=1&temp_bmp280=28.39&pressure=99687.62&altitude_bmp280=137.22&humidity=61.50&temperature=26.60&pm1.0=6&pm2.5=9&pm10.0=10