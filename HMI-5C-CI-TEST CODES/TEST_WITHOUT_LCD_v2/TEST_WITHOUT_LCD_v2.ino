#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "Wire.h"
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "FS.h"
#include "SD.h"
#include "RTClib.h"
#include <Adafruit_ADS1X15.h>


// Define the MOSI, MISO, and SCLK pins
#define MOSI 13
#define MISO 11
#define SCLK 12
#define CS 10

#define CS_PIN 10

#define TOUCH_CS 39
#define SD_CS 46

#define TIRQ_PIN  42

#define RXD 48
#define TXD 2

#define BUZER  0

#define SDA 19
#define SCL 20

// I2C address of PCA9538
#define PCA9538_ADDR 0x73

// PCA9538 register addresses
#define PCA9538_INPUT_REG 0x00
#define PCA9538_OUTPUT_REG 0x01
#define PCA9538_POLARITY_REG 0x02
#define PCA9538_CONFIG_REG 0x03

// GPIO pins
#define GPIO1 0x01
#define GPIO2 0x02
#define GPIO3 0x04
#define GPIO4 0x08
#define GPIO5 0x10
#define GPIO6 0x20
#define GPIO7 0x40
#define GPIO8 0x80

#define NUM_INPUT_PINS 4  // Change this to the number of input pins you have

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;

XPT2046_Touchscreen ts(TOUCH_CS,TIRQ_PIN);  

RTC_DS3231 rtc; 

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

unsigned int localPort = 8888;       // local port to listen for UDP packets

const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

EthernetUDP Udp;// A UDP instance to let us send and receive packets over UDP


void setPinMode(uint8_t pin, uint8_t mode) {
  uint8_t config = readRegister(PCA9538_CONFIG_REG);
  if (mode == INPUT) {
    config |= pin;
  } else {
    config &= ~pin;
  }
  writeRegister(PCA9538_CONFIG_REG, config);
}

void writeOutput(uint8_t pin, uint8_t value) {
  uint8_t output = readRegister(PCA9538_OUTPUT_REG);
  if (value == LOW) {
    output &= ~pin;
  } else {
    output |= pin;
  }
  writeRegister(PCA9538_OUTPUT_REG, output);
}

void toggleOutput(uint8_t pin) {
  uint8_t output = readRegister(PCA9538_OUTPUT_REG);
  output ^= pin;
  writeRegister(PCA9538_OUTPUT_REG, output);
}

uint8_t readInput() {
  return readRegister(PCA9538_INPUT_REG);
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(PCA9538_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(PCA9538_ADDR, 1);
  return Wire.read();
}

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(PCA9538_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void ETHERNET_CHECK() {

  //SPI.begin(SCLK, MISO, MOSI, CS_PIN);
  //SPI.begin(SCLK, MISO, MOSI);


  Ethernet.init(CS);  // ESP32 with Adafruit Featherwing Ethernet

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      Serial.println(Ethernet.hardwareStatus());
    }

    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
  }

  else {
    Serial.print("IP address: ");
    Serial.println(Ethernet.localIP());


    Udp.begin(localPort);

    sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
    delay(1000);
    if (Udp.parsePacket()) {
      // We've received a packet, read the data from it
      Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      // the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, extract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      Serial.print("Seconds since Jan 1 1900 = ");
      Serial.println(secsSince1900);

      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      // print Unix time:
      Serial.println(epoch);


      // print the hour, minute and second:
      Serial.print("The UTC time is ");
      Serial.print((epoch  % 86400L) / 3600);
      // print the hour (86400 equals secs per day)
      Serial.print(':');
      if (((epoch % 3600) / 60) < 10) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
      }

      Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
      Serial.print(':');
      if ((epoch % 60) < 10) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
      }

      Serial.println(epoch % 60); // print the second
    }
    // wait ten seconds before asking for the time again
    delay(3000);
    Ethernet.maintain();
  }
}


void sendNTPpacket(const char * address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


void i2c_scan() {
  byte error, address; //variable for error and I2C address
  int nDevices;
  Serial.println("\nI2C Scanner");
  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}



void setup() {

  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RXD, TXD);
  Wire.begin(SDA, SCL);

  //SPI.begin(SCLK, MISO, MOSI); //
  SPI.begin(SCLK, MOSI, MISO);

  ts.begin();
  ts.setRotation(2);



  i2c_scan();

  delay(1000);
  pinMode(TOUCH_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  
 
  digitalWrite(SD_CS, HIGH);
  digitalWrite(TOUCH_CS, HIGH);
  delay(200);  // delay for 200 milliseconds

  RTC_Check();
  delay(1000);

  SD_CHECK();
  delay(1000);

  ETHERNET_CHECK();

  setPinMode(GPIO1, INPUT);
  setPinMode(GPIO2, INPUT);
  setPinMode(GPIO3, INPUT);
  setPinMode(GPIO4, INPUT);

  // Set GPIO4 and GPIO5 as outputs
  setPinMode(GPIO5, OUTPUT);
  setPinMode(GPIO6, OUTPUT);
  setPinMode(GPIO7, OUTPUT);
  setPinMode(GPIO8, OUTPUT);



  if (!ads2.begin(0x49)) {
    Serial.println("Failed to initialize ADS 1 .");
    // while (1);
  }


 ads2.setGain(GAIN_ONE);


}

void loop() {


  Serial1.println("RS485 01 SUCCESS");             // Send RS485 SUCCESS serially

  while (Serial1.available()) {  // Check if data is available
    char c = Serial1.read();     // Read data from RS485
    Serial.write(c);           // Print data on serial monitor
  }
    Serial.println("------------------------------------------------------------------------");
  uint8_t input = readInput();
  bool inputValues[NUM_INPUT_PINS];
  
  for (int i = 0; i < NUM_INPUT_PINS; i++) {
      inputValues[i] = bitRead(input, i);
       Serial.print( inputValues[i]);
       
  }

  if (ts.tirqTouched()) {
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      Serial.print("Pressure = ");
      Serial.print(p.z);
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.print(p.y);
      delay(100);
      Serial.println();
    }
  }


     Serial.println("");

     
    writeOutput(GPIO5, 0);
    writeOutput(GPIO6, 0);
    writeOutput(GPIO7, 0);
    writeOutput(GPIO8, 0);
    delay(300);
  
    writeOutput(GPIO5, 1);
    writeOutput(GPIO6, 0);
    writeOutput(GPIO7, 0);
    writeOutput(GPIO8, 0);
    delay(300);
   
    writeOutput(GPIO5, 0);
    writeOutput(GPIO6, 1);
    writeOutput(GPIO7, 0);
    writeOutput(GPIO8, 0);
    delay(300);
  
    writeOutput(GPIO5, 0);
    writeOutput(GPIO6, 0);
    writeOutput(GPIO7, 1);
    writeOutput(GPIO8, 0);
    delay(300);
  
    writeOutput(GPIO5, 0);
    writeOutput(GPIO6, 0);
    writeOutput(GPIO7, 0);
    writeOutput(GPIO8, 1);
    delay(300);



    Serial.println("------------------------------------------------------------------------");

  int adc0=0,adc1=0,adc2=0,adc3=0;


 
   adc0 = ads2.readADC_SingleEnded(0);
   adc1 = ads2.readADC_SingleEnded(1);
   adc2 = ads2.readADC_SingleEnded(2);
   adc3 = ads2.readADC_SingleEnded(3);


  Serial.print("AIN1: "); Serial.print(adc0); Serial.println("  ");
  Serial.print("AIN2: "); Serial.print(adc1); Serial.println("  ");
  Serial.print("AIN3: "); Serial.print(adc2); Serial.println("  ");
  Serial.print("AIN4: "); Serial.print(adc3); Serial.println("  ");




  Serial.println("-----------------------------------------------------------");
}

void SD_CHECK(){
  uint8_t cardType = SD.cardType();
  //spi.begin(SCK, MISO, MOSI, CS);

    if(SD.begin(46))
 {
  Serial.println("Card Mount: success");
  Serial.print("Card Type: ");

    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("Unknown");
    }

  int cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Card Size: %lluMB\n", cardSize);

  }


}

void displayTime(void) {
  DateTime now = rtc.now();
     
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);

  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  delay(1000);

}


void RTC_Check(){
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
 else{
 if (rtc.lostPower()) {
  
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
  }

   
  int a=1;
  while(a<6)
  {
  displayTime();   // printing time function for oled
  a=a+1;
  }
 }
}
