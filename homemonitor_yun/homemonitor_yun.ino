#include <Console.h>
#include <dht.h>
#include <Process.h>
#include <FileIO.h>
#include <Time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

dht DHT;

// Uncomment DO_LOGGING to enable logging data to SD card
#define DO_LOGGING 


#define FILE_DIRECTORY "/mnt/sd/"

// Pins 2/3 are hardwired for I2C on the Arduino Yun
#define DHT22_PIN 4
#define DHT11_PIN 5
#define DHT22_PIN_B 6


Process date; // process used to get the date
String dateTimeString; // string holding process returned date and time
String datestring; // Used by file writer/LCD for formatting

#define MAX_FILEPATH_LENGTH 50
static char filePath[MAX_FILEPATH_LENGTH];
String filename;

// Used for main loop timing purposes
int last_second = -1;
int last_minute = -1;
int last_hour = -1;
int last_day = -1;
int last_sensor_read_time = -1;

// Sensor results arrays
// [DHT22 Outdoor, DHT11 Indoor, DHT22 Indoor]
double humidity[3]; // Humidity in %
double temp[3]; // Temperatures in celsius

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  Bridge.begin();
  Console.begin();
#ifdef DO_LOGGING
  FileSystem.begin();
  filename.reserve(MAX_FILEPATH_LENGTH);
  datestring.reserve(20);
#endif
  lcd.init();
  lcd.backlight();

  // lcd.clear();
  // lcd.print("Waiting for");
  // lcd.setCursor(0, 1);
  // lcd.print("console...");

  // while (!Console){
  //   ; // wait for Console port to connect.
  // }

  lcd.clear();
  lcd.print("  Arduino Yun   ");
  lcd.setCursor(0,1);
  lcd.print("  Home Monitor  ");
  delay(1000);

  
  Console.println("Arduino Yun Home Monitor");
  Console.println();

  

  Console.println("Temp/Humidity Logging using DHT sensors");
  Console.print("DHT LIBRARY VERSION: ");
  Console.println(DHT_LIB_VERSION);

  // Update time  from server, and there-after every X 
  updateTimeFromServer(); // ~1.16s
  Console.print("Current time Set to: ");
  displayCurrentTime();

#ifdef DO_LOGGING
  Console.println("Logging Enabled");
  Console.print("Logging to");
  Console.println(FILE_DIRECTORY);

  // Set filename to record to
  getFilename();
  last_minute = minute(); // So we don't immediately write data till next minute
  Console.print("Filename set to: ");
  Console.println(filePath);
  lcd.print("Filename set to:");
  lcd.setCursor(0, 1);
  lcd.print(filename.substring(8)); // Assuming 8 chars is length of FILE_DIRECTORY
  delay(2000);

#else
  Console.println("Logging Disabled");
#endif

  Console.println("------");
}

// Run loop at 10Hz
#define LOOP_RATE 100 // ms delay (1/LOOP_RATE = Frequency in Hz)

void loop()
{
  // Things in loop can take longer than a second, so keep track of start second
  int second_now = second();

  //// EVERY SECOND
  // Display time every second
  if (second_now != last_second)
  {
    last_second = second_now;

    displayCurrentTime();

    updateLCD();
  }

  //// EVERY DAY (Update Filename here before writing etc.)
  if (day() != last_day)
  {
    last_day = day();
#ifdef DO_LOGGING
    getFilename();
    Console.print("Filename set to: ");
    Console.println(filePath);

    lcd.clear();
    lcd.print("Filename set to:");
    lcd.setCursor(0, 1);
    lcd.print(filename.substring(8)); // Assuming 8 chars is length of FILE_DIRECTORY
    delay(2000);
#endif
  }


  //// EVERY 10 SECONDS
  // Read sensors every 10 seconds
  if (second_now % 10 == 0 && second_now != last_sensor_read_time)
  {
    last_sensor_read_time = second_now; // To avoid multiple hits in same second
    readData();
    displayCurrentData();
  }

  //// EVERY MINUTE
  // Log data every minute
  if (minute() != last_minute)
  {
    last_minute = minute();

#ifdef DO_LOGGING
    // Append data to file
    logToFile();

    lcd.setCursor(0,0);
    lcd.print("LOGGED"); // This will get overwritten by next second
#endif
  }

  //// EVERY HOUR
  // Resync with linux clock every hour
  if (hour() != last_hour)
  {
    last_hour = hour();
    updateTimeFromServer(); // ~1.16s
    Console.print("Current time Set to: ");
    displayCurrentTime();

  }

  delay(LOOP_RATE); // Rate-limit main loop
}


// Reads most recent sensor humidity and temperature readings
void readData()
{
    // DHT 22
    int chk = DHT.read22(DHT22_PIN); // ~24ms +- 0.2ms
    if (chk == DHTLIB_OK)
    {
      humidity[0] = DHT.humidity;
      temp[0] = DHT.temperature;
    }
    else
    {
      humidity[0] = -1E100;
      temp[0] = -1E100;
    }
    
    // DHT 11
    chk = DHT.read11(DHT11_PIN); // ~24ms +- 0.2ms
    if (chk == DHTLIB_OK)
    {
      humidity[1] = DHT.humidity;
      temp[1] = DHT.temperature;
    }
    else
    {
      humidity[1] = -1E100;
      temp[1] = -1E100;
    }
    
    // DHT 22 B
    chk = DHT.read22(DHT22_PIN_B); // ~24ms +- 0.2ms
    if (chk == DHTLIB_OK)
    {
      humidity[2] = DHT.humidity;
      temp[2] = DHT.temperature;
    }
    else
    {
      humidity[2] = -1E100;
      temp[2] = -1E100;
    }
}

#ifdef DO_LOGGING

// Restriction on filename length is 8 chars not including extension for FAT?
// Readings_018.txt is what seems to get generated, so maybe 12 chars
void getFilename()
{
  filename = FILE_DIRECTORY;
  filename += 'R';
  
  filename += year();
  filename += '_';
  
  if (month() < 10) {filename += '0';}
  filename += month();
  filename += '_';
  
  if (day() < 10) {filename += '0';}
  filename += day();

  filename += ".txt";

  // Push to filePath char array
  filename.toCharArray(filePath, MAX_FILEPATH_LENGTH);
}

// Store data on file : TODO Save to separate file each day
void logToFile()
{
  File dataFile = FileSystem.open(filePath, FILE_APPEND);
  datestring = "";
  datestring += year();
  datestring += '/';
  if (month() < 10) {datestring += '0';}
  datestring += month();
  datestring += '/';
  
  if (day() < 10) {datestring += '0';}
  datestring += day();
  datestring += ' ';

  if (hour() < 10) {datestring += '0';}
  datestring += hour();
  datestring += ':';

  if (minute() < 10) {datestring += '0';}
  datestring += minute();

  if (dataFile) {    
    dataFile.print(datestring);
    dataFile.print("\t");
    dataFile.print(humidity[0],1);
    dataFile.print("\t");
    dataFile.print(temp[0],1);
    dataFile.print("\t");
    dataFile.print(humidity[1],1);
    dataFile.print("\t");
    dataFile.print(temp[1],1);
    dataFile.print("\t");
    dataFile.print(humidity[2],1);
    dataFile.print("\t");
    dataFile.print(temp[2],1);
    dataFile.println();

    dataFile.close();

    Console.print("Logged to ");
    Console.print(filePath);
    Console.print(": ");

    Console.print(datestring);
    Console.print("\t");
    Console.print(humidity[0],1);
    Console.print("\t");
    Console.print(temp[0],1);
    Console.print("\t");
    Console.print(humidity[1],1);
    Console.print("\t");
    Console.print(temp[1],1);
    Console.print("\t");
    Console.print(humidity[2],1);
    Console.print("\t");
    Console.print(temp[2],1);
    Console.println();
  }
  else
  {
    Console.print("Error opening ");
    Console.println(filePath);

    lcd.clear();
    lcd.print("SD Write Error");
    lcd.setCursor(0,1);
    lcd.print(filename.substring(8)); // Assuming 8 chars is length of FILE_DIRECTORY
    
    delay(2000);
  }
}
#endif


void updateLCD()
{
  lcd.setCursor(0,0);
  lcdPrintLeadingZeroInt(month());
  lcd.print("/");
  lcdPrintLeadingZeroInt(day());
  lcd.print("   ");
  lcdPrintLeadingZeroInt(hour());
  lcd.print(":");
  lcdPrintLeadingZeroInt(minute());
  lcd.print(":");
  lcdPrintLeadingZeroInt(second());

  if ((second()/5) % 2 == 0) // switch every 5 seconds
  {
    lcd.setCursor(0, 1);
    lcd.print("OUT "); // Out
    lcd.print(temp[0],1);
    lcd.print((char)223);
    lcd.print("C ");
    lcd.print(humidity[0],1);
    lcd.print("%");
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("IN  "); // In
    lcd.print(temp[2],1);
    lcd.print((char)223);
    lcd.print("C ");
    lcd.print(humidity[2],1);
    lcd.print("%");
  }
}

// Prints input integer with a leading zero if < 10
void lcdPrintLeadingZeroInt(int value)
{
  if (value >= 10)
    lcd.print(value);
  else
  {
    lcd.print("0");
    lcd.print(value);
  }
}

// Displays sensor values on Console
void displayCurrentData()
{
  Console.print("Outdoor: ");
  Console.print(temp[0],2);
  Console.print("*C, ");
  Console.print(humidity[0],2);
  Console.println("%%");

  Console.print("Indoor (DHT11): ");
  Console.print(temp[1],2);
  Console.print("*C, ");
  Console.print(humidity[1],2);
  Console.println("%%");

  Console.print("Indoor: ");
  Console.print(temp[2],2);
  Console.print("*C, ");
  Console.print(humidity[2],2);
  Console.println("%%");
}

///////////////////////
/// TIME FUNCTIONS

// Updates arduino time from linux `date` call
void updateTimeFromServer()
{
  // restart the date process:
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%F\ %T"); // Date Time 'YYYY-MM-DD hh:mm:ss'
    date.run();
  }
  
  //if there's a result from the date process, parse it:
  while (date.available()>0) {
    // Get the result of the date process (should be 'YYYY-MM-DD hh:mm:ss'):
    dateTimeString = date.readString();
    dateTimeString.trim(); // Remove newline

    // Get the substrings
    String yearString = dateTimeString.substring(0, 4);
    String monthString = dateTimeString.substring(5, 7);
    String dayString = dateTimeString.substring(8, 10);

    String hourString = dateTimeString.substring(11, 13);
    String minString = dateTimeString.substring(14, 16);
    String secString = dateTimeString.substring(17);

    // Update Arduino Clock with integer values
    setTime(hourString.toInt(), minString.toInt(), secString.toInt(),
            dayString.toInt(), monthString.toInt(), yearString.toInt());
  } 
}

void displayCurrentTime()
{
  Console.print(year()); 
  Console.print("/");
  Console.print(month());
  Console.print("/");
  Console.print(day());

  Console.print(" ");
  
  Console.print(hour());
  Console.print(":");
  printDigits(minute());
  Console.print(":");
  printDigits(second());
  
  Console.println(); 
}

// Preceding Zeros
void printDigits(int digits){
  if(digits < 10)
    Console.print('0');
  Console.print(digits);
}