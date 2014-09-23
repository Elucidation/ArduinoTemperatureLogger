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
#include <SD.h> // Modified SD.cpp in SD library to allow for multiple SD.begin() calls
// Add to Line 343 of SD.cpp: 'if (root.isOpen()) {root.close();}'

#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>
#include <DS1307RTC.h>

#include <ThermistorSensor.h>

// Thermistor Setup
// which analog pins to connect
#define THERMISTOR_PIN_A A0
#define THERMISTOR_PIN_B A1

// Thermistor objects and temperature variables
ThermistorSensor thermistorA(THERMISTOR_PIN_A);
ThermistorSensor thermistorB(THERMISTOR_PIN_B);

#define FILENAME "mtest.txt"
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

int counter = 0;

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
    myFile.print(minute());
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

boolean buttonPressed = false;
void buttonInterrupt()
{
  buttonPressed = true;
}

// Read and record data to log files
void recordData()
{
  delay(250);
  Serial.println("Woo");
  if (startSD())
  {
    // Read thermistors
    int tempA = thermistorA.getReading();
    int tempB = thermistorB.getReading();

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
  buttonPressed = false;

}

void setup()
{
  Serial.begin(9600);
  setSyncProvider(RTC.get); // get time from Real Time Clock
  if(timeStatus()!= timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  // Set button to write to file
  attachInterrupt(0, buttonInterrupt, RISING);

  // Set call to update files every minute
  Alarm.timerRepeat(10, recordData);
}

void loop()
{
  if (buttonPressed)
  {
    recordData();
  }

  Alarm.delay(1000); // Check every second
}


