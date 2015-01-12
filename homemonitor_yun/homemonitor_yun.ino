#include <Console.h>
#include <dht.h>
#include <Process.h>
#include <FileIO.h>

dht DHT;

#define DHT22_PIN 2
#define DHT11_PIN 3
#define DHT22_PIN_B 4


Process date;                 // process used to get the date
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

void loop()
{
    // Collect data into string
    String dataString = "";
    
    updateTimeFromServer(); // ~1.16s
    dataString = dataString + hours + ":" + minutes + ":" + seconds + "\t";
    
    // DHT 22
    int chk = DHT.read22(DHT22_PIN); // ~24ms +- 0.2ms
    if (chk == DHTLIB_OK)
    {
      dataString = dataString + DHT.humidity + "\t" + DHT.temperature + "\t";
    }
    else
    {
      dataString = dataString + "X\tX\t";
    }
    
    // DHT 11
    chk = DHT.read11(DHT11_PIN); // ~24ms +- 0.2ms
    if (chk == DHTLIB_OK)
    {
      dataString = dataString + DHT.humidity + "\t" + DHT.temperature + "\t";
    }
    else
    {
      dataString = dataString + "X\tX\t";
    }
    
    // DHT 22 B
    chk = DHT.read22(DHT22_PIN_B); // ~24ms +- 0.2ms
    if (chk == DHTLIB_OK)
    {
      dataString = dataString + DHT.humidity + "\t" + DHT.temperature + "\t";
    }
    else
    {
      dataString = dataString + "X\tX\t";
    }

    // Append data to file
    File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Console.println(dataString);
    }
    else
    {
      Serial.println("error opening datalog.txt");
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

