/*
Temperature Monitoring system

The circuit:
* SD card attached to SPI bus as follows:
** MOSI - pin 11
** MISO - pin 12
** CLK - pin 13
** CS - pin 4

filename has to be short :\ *NOTE*

*/

// SD storage stuff
#include <SD.h> // Modified SD.cpp in SD library to allow for multiple SD.begin() calls
// Add to Line 343 of SD.cpp: 'if (root.isOpen()) {root.close();}'

// RTC Timer stuff
#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>
#include <DS1307RTC.h>

// Thermistor stuff
#include <ThermistorSensor.h>

// LCD stuff
#include <LiquidCrystal_I2C.h>

// Thermistor Setup
// which analog pins to connect
#define THERMISTOR_PIN_A A0 // Indoor
#define THERMISTOR_PIN_B A1 // Outdoor

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
char lcd_line_buffer[17]; // Single line buffer for writing to LCD


int tempA;
int tempB;

// Thermistor objects and temperature variables
ThermistorSensor thermistorA(THERMISTOR_PIN_A);
ThermistorSensor thermistorB(THERMISTOR_PIN_B);

File myFile;

void readFileToSerial(char* filename)
{
  // re-open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.print("Reading ");
    Serial.print(filename);
    Serial.println(":");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening for reading");
  }
}

// Append int value to file
void logToFile(char* filename, int value)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(filename, FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    // Serial info
    Serial.print("Writing ");
    Serial.print(value);
    Serial.print(" to ");
    Serial.print(filename);
    Serial.print("... ");
    
    // Write value to file and close
    myFile.println(value);
    myFile.close();
    
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening for writing.");
  }
}

// Log time to file
void logTimeToFile(char* filename)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(filename, FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    // Serial info
    Serial.print("Writing '");
    Serial.print(month());
    Serial.print(" ");
    Serial.print(day());
    Serial.print(" ");
    Serial.print(hour());
    Serial.print(" ");
    Serial.print(minute());
    Serial.print("' to ");
    Serial.print(filename);
    Serial.print("... ");
    
    // Write value to file and close
    myFile.print(month());
    myFile.print(" ");
    myFile.print(day());
    myFile.print(" ");
    myFile.print(hour());
    myFile.print(" ");
    myFile.println(minute());
    myFile.close();
    
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening for writing.");
  }
}

boolean startSD()
{
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(10, OUTPUT);

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return false;
  }
  Serial.println("initialization done.");
  return true;
}

boolean trigger_button = false;
void buttonInterrupt()
{
  trigger_button = true;
}

void updateTemperatures()
{
  tempA = thermistorA.getReading();
  tempB = thermistorB.getReading();
}

// Read and record data to log files
void recordData()
{
  delay(250);
  Serial.println("Woo");
  if (startSD())
  {
    // Read thermistors
    updateTemperatures();

    // Log Time & Temperature
    logTimeToFile("time.txt");
    logToFile("T_in.txt", tempA);
    logToFile("T_out.txt", tempB);

    Serial.println("time.txt: ");
    readFileToSerial("time.txt");
    Serial.println("T_in.txt: ");
    readFileToSerial("T_in.txt");
    Serial.println("T_out.txt: ");
    readFileToSerial("T_out.txt");
  }
  trigger_button = false;

}

boolean trigger_lcd = false;

// void updateLCD()
// {
//   sprintf(lcd_line_buffer, "%2d/%02d %02d:%02d:%02d", 
//     month(), day(), hour(), minute(), second());
//   // lcd.clear();
//   lcd.setCursor(0,0);
//   lcd.print(lcd_line_buffer);

//   sprintf(lcd_line_buffer, "IN: %2d, OUT: %3d", 
//     tempA, tempB);
//   lcd.setCursor(0,1);
//   lcd.print(lcd_line_buffer);

//   // LCD refreshed, clear update bit
//   trigger_lcd = false;
// }


void triggerLCD()
{
  trigger_lcd = true;
}

void setup()
{
  Serial.begin(9600);

  // Initialize the lcd with backlight on
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");


  // Set RTC sync
  setSyncProvider(RTC.get); // get time from Real Time Clock
  if(timeStatus()!= timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  updateTemperatures();

  // Set button to write to file
  attachInterrupt(0, buttonInterrupt, RISING);

  // Set call to update files every minute
  Alarm.timerRepeat(60, buttonInterrupt);

  // Update LCD screenevery second (avoid SPI collision)
  // Alarm.timerRepeat(1, triggerLCD);

  // Done
  lcd.setCursor(0,1);
  lcd.print("Initialized.");
  delay(500);
  lcd.clear();
}

void loop()
{
  // Called like this to avoid SPI bus collision between SD & LCD
  if (trigger_button)
  {
    Serial.println("Trigger Button");
    recordData();
    delay(1000);
    lcd.clear();
    lcd.print("Data Logged");
    delay(1000);
  }

  // if (trigger_lcd)
  // {
  //   Serial.println("Trigger LCD");
  //   updateTemperatures();
  //   updateLCD();
  //   delay(10);
  // }

  Alarm.delay(500); // Check every second
}


