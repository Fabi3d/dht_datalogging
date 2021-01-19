#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

//Display---------------------------------------
LiquidCrystal_I2C lcd(0x27, 20, 4);           //the i2c adress of the Display is 0X27 and the Display has 20 characters in one line and 4 lines

//Bluetooth module------------------------------
SoftwareSerial BTserial(4, 5);       //RX | TX of the Bluetooth module

//RTC--------------------------------------------
RTC_DS1307 rtc;           //initialize the rtc modul (it is a DS1307)

//SD Card----------------------------------------
bool sdcard = true;       //If no SD card is inserted, this bool is going to be false later
File dataFile;            //create a File

//DHT22------------------------------------------
#define DHTTYPE DHT22         //if you are using a DHT11 you have to change it here
#define DHTPIN 3              //the DHT22 is connected to Digital Pin 3
DHT dht(DHTPIN, DHTTYPE);     //initialize the DHT sensor with the type and the pin

//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  //starting the serial connections-------------------------------------------
  Serial.begin(9600);                   //start a serial communtication with 9600 baud
  BTserial.begin(9600);                 //start a serial communtication over the BT module with 9600 baud

  //lcd panel-------------------------------------------------------------------
  lcd.init();                     //initialize the lcd panel
  lcd.backlight();                //turn on the backlight of the lcd panel
  lcd.clear();                    //clear the lcd
  lcd.setCursor(0, 0);            //set the cursor to the first character of the lcd
  lcd.print("starting......");    //printing the string "starting" on the lcd

  //SD Card----------------------------------------------------------------------
  Serial.print("Initializing the SD Card");
  if (!SD.begin())                //starting the SD card reader
  {
    Serial.print("initializion failed!");           //printing an error message on the serial monitor if the SD Card can't be inizialized
    lcd.clear();                                    //also clear the LCD and print an error message
    lcd.print("unable to start the SD card");
    delay(1500);
    sdcard = false;                                 //set this bool to false if no SD card is inserted, that the data log does not get confused
  }

  else
  {
    Serial.print("initializion done.");               //if a SD card is inseted, print this messageon the Serial monitor
  }

  //DHT22------------------------------------------------------------------------
  dht.begin();            //start the DHT22

  //rtc--------------------------------------------------------------------------
  if (! rtc.begin())               //if the RTC couldn't be initialized print error messages on the lcd and the serial monitor
  {
    Serial.println("Couldn't finde the RTC");
    lcd.clear();
    lcd.print("Couldn't find the RTC");
    delay(1000);
    Serial.flush();
    abort();  
  }
 // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  

  if (!rtc.isrunning())                 //if the rtc isn't running, set the time correct
  {
    Serial.print("RTC is NOT running, let's set the time!");
    lcd.print("rtc problem");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  
  }

  //SD card----------------------------------------------------------------------
  if (sdcard == true)
  {
    dataFile = SD.open("DHT22_weatherstation_log.csv", FILE_WRITE);     //create a file on the SD card called "DHT22_weatherstation_log" and save as csv so you can it easily open in Excel
    if (dataFile)
    {
      dataFile.print("Uhrzeit;;");                                    //printing a header in the csv tabel. The ';' stands for the tabulator
      dataFile.print("Temperatur;;");
      dataFile.print("Luftfeuchtigkeit;");
      dataFile.println();                                             //go into the next line
      dataFile.close();                                               //close the file
    }

  }

  delay(3000);              //end the setup
  lcd.clear();

}


uint16_t line = 1;
void loop() {


  //rtc----------------------------------------------------
  DateTime now = rtc.now();   //getting the time
  DateTime time = rtc.now();
  delay(1000);
  //DHT---------------------------------------------------
  float luftfeuchtigkeit = dht.readHumidity();            //read the humidity from the dht sensor and save it in the float luftfeuchtigkeit
  float temperatur = dht.readTemperature();               //read the temperatur from the dht sensor and save it in the float temperatur


  /*if (isnan(temperatur))                //if there is an sensor error, print a error message on the serial monitor and the LCD screen
  {
    Serial.println("Bitte Service kontaktrieren!!");
    lcd.clear();

    while (isnan(temperatur))
    {
      lcd.setCursor(1, 0);
      lcd.print("Bitte Service ");
      lcd.setCursor(1, 1);
      lcd.print("kontaktrieren und ");
      lcd.setCursor(1, 2);
      lcd.print("resetten!");
    }
  }

  else
  {*/
    Serial.print("Temperatur: ");             //if the Arduino is getting correct data, print this on the serial monitor
    Serial.print(temperatur);
    Serial.println("°C");
    Serial.print("Luftfeuchtigkeit: ");
    Serial.print(luftfeuchtigkeit);
    Serial.println("%");

  //}

  //LCD---------------------------------------------------
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Uhrzeit: ");                   //printing the string "Uhrzeit" on the LCD
  lcd.print(time.timestamp(DateTime::TIMESTAMP_TIME));      //this is the full time from the RTC
  lcd.setCursor(1, 2);                      //set the cursor in the next line
  lcd.print("Temperatur: ");                //printing the temperature on the lcd
  lcd.print(temperatur);
  lcd.print("\337C");                       // "\337" is  the ascii code for the degree mark  --> °C
  lcd.setCursor(1, 3);                      //go into the next line
  lcd.print("Hum.: ");                      //printing the humidity on the LCD
  lcd.print(luftfeuchtigkeit);
  lcd.print("\045");                        // "\045" is the ascii code for the %

  //SD card-----------------------------------------------
  if (now.second() == 0)                    //if the seconds of the rtc equals 0, get data to the SD card
  {
    if (sdcard == true)                     //only open the SD card if one is inserted
    {
      dataFile = SD.open("DHT22_weatherstation_log.csv", FILE_WRITE);       //open the file "DHT22_weatherstation_log.csv"
      if (dataFile)
      {
        dataFile.print(time.timestamp(DateTime::TIMESTAMP_TIME) + ";" + ";");      //first print the time
        dataFile.print(temperatur);                                           //print the temperatur in the log
        dataFile.print((char)176);                                            //((char)176) is the degree mark
        dataFile.print("C ;;");                                               //print the 'C' and go to the next column
        dataFile.print(luftfeuchtigkeit);                                     //printing the humidity
        dataFile.print("%");
        dataFile.close();                                                     //close the file
      }
    }
  }

  //BT----------------------------------------------------
  /*
    For this part you have to have the ArduTooth App to communicate with the BT module
    For this App you have to seperate values by a ',' and end with a ';'
  */
  BTserial.print(time.timestamp(DateTime::TIMESTAMP_TIME));           //print the timestamp in the first box of the App
  BTserial.print (" Uhr");
  BTserial.print(",");
  BTserial.print(temperatur);                                         //print the temperatur in the second box
  BTserial.print((char)176);                                          //((char)176) is the degree mark
  BTserial.print("C,");
  BTserial.print(luftfeuchtigkeit);                                   //print the humidity in the third box
  BTserial.print("\045");                                             //("\045") is the ASCII code for the %
  BTserial.print(",");

  if (sdcard == true)                                                 //if a SD card is inserted, print this in the last box
  {
    BTserial.print("gestartet");
  }
  else
  {
    BTserial.print("angehalten");
  }
  BTserial.print(";");

}
