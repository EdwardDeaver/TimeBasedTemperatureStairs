const byte ledPin = LED_BUILTIN;
const byte interruptPin = 2;
const byte interruptPin3 = 3;

volatile byte state = LOW;
  unsigned long currentMillis;
unsigned long timegoal = 0;
unsigned long delayValue = 20000;

////////////////////////////
/// RTC ///////////////////
///////////////////////////
// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

///////////////////////////////
// Ethrnet
//////////////////////////////

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned int localPort = 8888;       // local port to listen for UDP packets

const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

////////////////////////////////////////////////////////////////
//  TIMEZONE
////////////////////////////////////////////////////////////////

#include <Timezone.h>           // http://github.com/JChristensen/Timezone

TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
Timezone usEastern(usEDT, usEST);
time_t utc = now();  //current time from the Time Library
time_t eastern = usEastern.toLocal(utc);



void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
    Serial.begin(9600);        // initialize serial

  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin3), blink, RISING);


////////////////////////////////////////////////////
/// ETHERNET
///////////////////////////////////////////////////

// start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  Udp.begin(localPort);


  /////////////////////
  // RTC
  ////////////////////

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
 
 uint32_t eastern = getEpoch(false);

 Serial.println("epoch eastern");
 Serial.println(eastern);
  rtc.adjust(DateTime(eastern));
  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    syncRTCWithNTP(false);
  }
    rtc.start();

}
bool SYNCED = true;
void loop() {
 currentMillis = millis();
  if(abs(millis()) < timegoal){
   // Serial.print("In IF");
      // get Time of day from RTC
      // Lookup the values of colors in array
      // Write to LEDS
      // Sync NTP to RTC
      digitalWrite(LED_BUILTIN, HIGH);
       getTime();
       SYNCED = false;

  }
  else{
          
          digitalWrite(ledPin, LOW);
          if(SYNCED == false){
              syncRTCWithNTP(false);
          Serial.println("SYNCED");

          }
          SYNCED = true;


  }
  //Serial.println("Doing other thing");

}

void blink() {
  Serial.println(millis());
  Serial.println(currentMillis);
  
  timegoal = currentMillis + delayValue;
  Serial.println(timegoal);
    Serial.println(state);

  state = !state;
}


void getTime(){
  DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

    // calculate a date which is 7 days, 12 hours and 30 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.println();
}


uint32_t getEpoch(bool GMT){
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
    uint32_t epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
    if(GMT){
          return epoch;
    }
    else{
      uint32_t eastern = usEastern.toLocal(epoch);
      return eastern;
    }
  }
  // wait ten seconds before asking for the time again
} 

void syncRTCWithNTP(bool TimeZoneValue){
  uint32_t eastern = getEpoch(TimeZoneValue);
  rtc.adjust(DateTime(eastern));
}

// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
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
