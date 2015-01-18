#include <Console.h>
#include <dht.h>
#include <Process.h>
#include <FileIO.h>
#include <Time.h>

dht DHT;

//#define DO_LOGGING
#define FILE_PATH ("/mnt/sd/datalog.txt")


#define DHT22_PIN 2
#define DHT11_PIN 3
#define DHT22_PIN_B 4


Process date;                 // process used to get the date
String dateTimeString;
int last_minute = -1;

// Sensor results arrays
// [DHT22 Outdoor, DHT11 Indoor, DHT22 Indoor]
double humidity[3]; // Humidity in %
double temp[3]; // Temperatures in celsius

void setup()
{
  Bridge.begin();
    Console.begin();
    FileSystem.begin();
    while (!Console){
      ; // wait for Console port to connect.
    }
    Console.println("Arduino Yun Home Monitor");
    Console.println();
    Console.println("Temp/Humidity Logging using DHT sensors");
    Console.print("DHT LIBRARY VERSION: ");
    Console.println(DHT_LIB_VERSION);
#ifdef DO_LOGGING
    Console.println("Logging Enabled");
    Console.print("Logging to");
    Console.println(FILE_PATH);
#else
    Console.println("Logging Disabled");
#endif

    // Update time  from server, and there-after every new day 
    updateTimeFromServer(); // ~1.16s
    Console.print("Current time Set to: ");
    displayCurrentTime();

    
    Console.println();
    Console.println("Date\tA-Humidity(%),\tA-Temp(C)\tB-Humidity(%),\tB-Temp(C)\tC-Humidity(%),\tC-Temp(C)");

}



void logToFile()
{
  File dataFile = FileSystem.open(FILE_PATH, FILE_APPEND);
    if (dataFile) {    
      dataFile.print(dateTimeString);
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

      Console.print("Logged: ");
      Console.print(dateTimeString);
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
      Console.println("error opening logfile");
    }
}


void loop()
{
  displayCurrentTime();

  if (second() % 10 == 0) // Every minute
  {
    last_minute = minute();

    readData();    

#ifdef DO_LOGGING
    // Append data to file
    logToFile();
#endif

    // Display updated temperatures
    displayCurrentTime();
    displayCurrentData();

  }

  delay(1000); // Check every second
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