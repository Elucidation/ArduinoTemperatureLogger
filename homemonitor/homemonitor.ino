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

// #define SERIAL_OUTPUT // for debugging

// SD storage stuff
#include <SD.h> // Modified SD.cpp in SD library to allow for multiple SD.begin() calls
// Add to Line 343 of SD.cpp: 'if (root.isOpen()) {root.close();}'

// RTC Timer stuff
#include <Time.h>
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

boolean SDCardExists = false; // used for LCD display purposes

float tempA;
float tempB;

// Thermistor objects and temperature variables
ThermistorSensor thermistorA(THERMISTOR_PIN_A);
ThermistorSensor thermistorB(THERMISTOR_PIN_B);

File myFile;

int leadZero(int value)
{
  return value;
}

// Prints input integers with leading zeros up to 2
void lcdPrintLeadingZerosInt(int value)
{
  if (value >= 10)
    lcd.print(value);
  else
  {
    lcd.print("0");
    lcd.print(value);
  }
}

#ifdef SERIAL_OUTPUT
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
#endif

// Append float value to file
void logToFile(char* filename, float value)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(filename, FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    #ifdef SERIAL_OUTPUT
    // Serial info
    Serial.print("Writing ");
    Serial.print(value);
    Serial.print(" to ");
    Serial.print(filename);
    Serial.print("... ");
    #endif
    
    // Write value to file and close
    myFile.println(value);
    myFile.close();
    
    #ifdef SERIAL_OUTPUT
    Serial.println("done.");
    #endif
  } else {
    // if the file didn't open, print an error:
    #ifdef SERIAL_OUTPUT
    Serial.println("error opening for writing.");
    #endif
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
    #ifdef SERIAL_OUTPUT
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
    #endif
    
    // Write value to file and close
    myFile.print(month());
    myFile.print(" ");
    myFile.print(day());
    myFile.print(" ");
    myFile.print(hour());
    myFile.print(" ");
    myFile.println(minute());
    myFile.close();
    
    #ifdef SERIAL_OUTPUT
    Serial.println("done.");
    #endif
  } else {
    // if the file didn't open, print an error:
    #ifdef SERIAL_OUTPUT
    Serial.println("error opening for writing.");
    #endif
  }
}

boolean startSD()
{
  #ifdef SERIAL_OUTPUT
  Serial.print("Initializing SD card...");
  #endif
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(10, OUTPUT);

  if (!SD.begin(4)) {
    #ifdef SERIAL_OUTPUT
    Serial.println("initialization failed!");
    #endif
    return false;
  }
  #ifdef SERIAL_OUTPUT
  Serial.println("initialization done.");
  #endif
  return true;
}

boolean trigger_logging = false;
void buttonInterrupt()
{
  trigger_logging = true;
}

void updateTemperatures()
{
  // tempA = thermistorA.getReading();
  // tempB = thermistorB.getReading();
  tempA = thermistorA.getFilteredReading();
  tempB = thermistorB.getFilteredReading();
}

// Read and record data to log files
void recordData()
{
  delay(250);
  if (startSD())
  {
    SDCardExists = true;
    // Read thermistors
    updateTemperatures();

    // Log Time & Temperature
    logTimeToFile("time.txt");
    logToFile("T_in.txt", tempA);
    logToFile("T_out.txt", tempB);

    #ifdef SERIAL_OUTPUT
    Serial.println("time.txt: ");
    readFileToSerial("time.txt");
    Serial.println("T_in.txt: ");
    readFileToSerial("T_in.txt");
    Serial.println("T_out.txt: ");
    readFileToSerial("T_out.txt");
    #endif
  }
  else {
    SDCardExists = false;
  }
}

boolean trigger_lcd = false;

void updateLCD()
{
  // Don't use sprintf or memory issues break SD card usage :(
  // sprintf(lcd_line_buffer, "%2d/%02d %02d:%02d:%02d", 
  //   month(), day(), hour(), minute(), second());
  // lcd.clear();
  lcd.setCursor(0,0);
  lcdPrintLeadingZerosInt( month() );
  lcd.print("/");
  lcdPrintLeadingZerosInt( day() );
  lcd.print(" ");

  lcdPrintLeadingZerosInt( hour() );
  lcd.print(":");
  lcdPrintLeadingZerosInt( minute() );
  lcd.print(":");
  lcdPrintLeadingZerosInt( second() );

  // sprintf(lcd_line_buffer, "I: %2d, O: %3d", 
  //   tempA, tempB);
  lcd.setCursor(0,1);
  lcd.print("I/O ");
  lcd.print(tempA,1);
  // lcd.print((char)223);
  lcd.print(" ");
  lcd.print(tempB,1);
  lcd.print((char)223);
  lcd.print("C    ");
}


void triggerLCD()
{
  trigger_lcd = true;
}

void setup()
{
  #ifdef SERIAL_OUTPUT
  Serial.begin(9600);
  #endif

  // Initialize the lcd with backlight on
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");


  // Set RTC sync
  setSyncProvider(RTC.get); // get time from Real Time Clock
  #ifdef SERIAL_OUTPUT
  if(timeStatus()!= timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
  #endif

  // Since we're using a filter call update several times to populate
  for (int i = 0; i < 50; ++i)
  {
    updateTemperatures();
    delay(20);
  }

  // Set button to write to file
  attachInterrupt(0, buttonInterrupt, RISING);

  // Set call to update files every minute
  // Alarm.timerRepeat(60, buttonInterrupt);

  // Update LCD screenevery second
  // Alarm.timerRepeat(1, triggerLCD);

  // Done
  lcd.setCursor(0,1);
  lcd.print("Initialized.");
  delay(500);
  lcd.clear();
}

int lastMinute = -1;
int lastSecond = -1;
void loop()
{
  //////////////////////////
  // Update Triggers
  // Manual triggers instead of alarm on minute/second edges

  // Manually trigger SD logging on new minute
  if (minute() != lastMinute)
  {
    lastMinute = minute();
    trigger_logging = true;
  }

  // Manually trigger LCD refresh on new second
  if (second() != lastSecond)
  {
    lastSecond = second();
    trigger_lcd = true;
  }

  ///////////////////////////
  // Check Triggers
  // Called like this to avoid SPI bus collision between SD & LCD
  if (trigger_logging)
  {
    #ifdef SERIAL_OUTPUT
    Serial.println("Trigger Button");
    #endif
    recordData();
    delay(250);
    lcd.clear();
    if (SDCardExists)
      lcd.print("Data Logged");
    else
      lcd.print("No SD Card");
    delay(500);

    trigger_logging = false;
  }

  if (trigger_lcd)
  {
    // Serial.println("Trigger LCD");
    updateLCD();
    delay(10);

    trigger_lcd = false;
  }

  updateTemperatures(); // Updates every loop ~ 10Hz if delay below is 100
  delay(100);
}


