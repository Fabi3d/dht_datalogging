#include <Adafruit_Sensor.h>        //including librarys
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


RTC_DS1307 rtc;             //initialize the real time clock


File dataFile;


#define DHTTYPE    DHT22
#define DHTPIN 3     // Digital pin connected to the DHT sensor 
DHT dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
//-------------------------------------------------------------------------------------------------------------------

void setup() {
  // put your setup code here, to run once:
//SD card reader
  Serial.begin(9600);            //set the baudrate to 9600
   lcd.init();                      // initialize the lcd 
         lcd.backlight();
         lcd.clear();
         lcd.setCursor(0,0);
lcd.print("starting...");
  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  dht.begin();             //Intitialize the DHT22 sensor
  
//rtc
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));                     //adjusting the time and the date
    
  }
  
  //display
 

  //SD card reader printing first line
    dataFile = SD.open("DHT22Log.csv", FILE_WRITE);       //open the file "DHT22Log.csv" on the SD card
    if(dataFile)

    {
      dataFile.print("Uhrzeit;;");
      dataFile.print("Temperatur;;");
      dataFile.println("Luftfeuchtigkeit;");
      dataFile.println();
      dataFile.close();
    }

    delay(3000);
    lcd.clear();

}

uint16_t line = 1;


//--------------------------------------------------------------------------------------------------------------------------
void loop() {
  // put your main code here, to run repeatedly:

  lcd.setCursor(1 ,0);

  DateTime time = rtc.now();
  delay(1000);

  float luftfeuchtigkeit = dht.readHumidity();      //reading the sensor values form the DHT sensor
  float temperatur = dht.readTemperature();

   Serial.println(time.timestamp(DateTime::TIMESTAMP_TIME));              //print the time in the serial monitor
 
  if (isnan(temperatur)) {                 //if the sensor has any error
    Serial.println(F("Bitte Service-Hotline kontaktieren!!"));
  }

  else {                                        //printing the temperatur in the serial monitor
    Serial.print(F("Temperature: "));
    Serial.print(temperatur);
    Serial.println(F("°C"));
  }
  
  if (isnan(luftfeuchtigkeit)) {       //if the sensor has any error
    Serial.println(F("Bitte Service-Hotline kontaktieren!!"));
    lcd.clear();
    while(isnan(luftfeuchtigkeit))
    {

    lcd.setCursor(1,0);
    lcd.print("Bitte Service ");
    lcd.setCursor(1,1);
    lcd.print("kontaktieren und ");
    lcd.setCursor(1,2);
    lcd.print("resetten!");
    
    }
  }
  else {
    Serial.print(F("Humidity: "));            //printing the humidity in the serial monitor
    Serial.print(luftfeuchtigkeit);
    Serial.println(F("%"));
  }

  dataFile = SD.open("DHT22Log.csv", FILE_WRITE);       //open the file "DHT22Log.csv" on the SD card

  if (dataFile)
  {
         //write the data of the DHT event in the "DHT22LOG.csv" file
    dataFile.print(time.timestamp(DateTime::TIMESTAMP_TIME)+";"+";");
   
    dataFile.print(temperatur);
    dataFile.print((char)176);
    dataFile.print("C ;;");
    dataFile.print(luftfeuchtigkeit);
    dataFile.println("%");
    dataFile.close();
  }

  lcd.print("Uhrzeit: ");
  lcd.print(time.timestamp(DateTime::TIMESTAMP_TIME));
  lcd.setCursor(1,  1);
  lcd.print("Temperatur: ");
  lcd.print(temperatur);
  lcd.print("\337C");
  lcd.setCursor(1, 2);
  lcd.print("Hum.: ");
  lcd.print(luftfeuchtigkeit);
  lcd.print("\045");
  

}
