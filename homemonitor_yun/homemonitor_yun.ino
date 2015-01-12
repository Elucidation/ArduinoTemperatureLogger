#include <Console.h>
#include <dht.h>
#include <Process.h>
#include <FileIO.h>

dht DHT;

#define DHT22_PIN 2
#define DHT11_PIN 3
#define DHT22_PIN_B 4


Process date;                 // process used to get the date
String dateString;
int hours, minutes, seconds;  // for the results

void setup()
{
  Bridge.begin();
    Console.begin();
    FileSystem.begin();
    while (!Console){
      ; // wait for Console port to connect.
    }
    Console.println("Temp/Humidity Logging using DHT sensors");
    Console.print("LIBRARY VERSION: ");
    Console.println(DHT_LIB_VERSION);
    Console.println();
    Console.println("Date\tA-Humidity(%),\tA-Temp(C)\tB-Humidity(%),\tB-Temp(C)\tC-Humidity(%),\tC-Temp(C)");
}


double humidity[3];
double temp[3];

void loop()
{
  static char msg[100];

  // Collect data into string  
  updateTimeFromServer(); // ~1.16s
  
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

  // Append data to file
  File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);
  if (dataFile) {    
    dataFile.print(dateString);
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

    Console.print(dateString);
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
    Console.println("error opening datalog.txt");
  } 

  delay(2000);
}


void updateTimeFromServer()
{
  // restart the date process:
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%T");
    date.run();
  }
  
  //if there's a result from the date process, parse it:
  while (date.available()>0) {
    // get the result of the date process (should be hh:mm:ss):
    String timeString = date.readString();
    dateString = timeString; // TODO - Replace with Date+Time
    dateString.trim(); // Remove newline

    // find the colons:
    int firstColon = timeString.indexOf(":");
    int secondColon= timeString.lastIndexOf(":");

    // get the substrings for hour, minute second:
    String hourString = timeString.substring(0, firstColon); 
    String minString = timeString.substring(firstColon+1, secondColon);
    String secString = timeString.substring(secondColon+1);

    // convert to ints,saving the previous second:
    hours = hourString.toInt();
    minutes = minString.toInt();
    seconds = secString.toInt();
  } 
}

