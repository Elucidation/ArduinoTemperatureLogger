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

File myFile;
void readFileToSerial()
{
  // re-open the file for reading:
  myFile = SD.open("mtest.txt");
  if (myFile) {
    Serial.println("mtest.txt:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening mtest.txt");
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
    
    // Update counter
    counter++;
    
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening for writing.");
  }
}

boolean startSD()
{
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return false;
  }
  Serial.println("initialization done.");
  return true;
}

boolean buttonPressed = false;
void doButtonPressed()
{
  buttonPressed = true;
}

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(10, OUTPUT);
  
  if (!startSD())
  {
    return;
  }
  
  logToFile("mtest.txt", counter);
  
  readFileToSerial();
  
  // Trigger backlight on push button on pin 2 (interrupt 0)
  attachInterrupt(0, doButtonPressed, RISING);
}

void loop()
{
  if (buttonPressed)
  {
    delay(250);
    Serial.println("Woo");
    if (startSD())
    {
      logToFile("mtest.txt", counter);
      readFileToSerial();
    }
    buttonPressed = false;
  }
}


