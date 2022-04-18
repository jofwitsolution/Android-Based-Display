/*//Arduino pins used for the display connection
#define PIN_DMD_nOE       9    // D9 active low Output Enable, setting this low lights all the LEDs in the selected rows. Can pwm it at very high frequency for brightness control.
#define PIN_DMD_A         6    // D6
#define PIN_DMD_B         7    // D7
#define PIN_DMD_CLK       52   // D52_SCK  is SPI Clock if SPI is used
#define PIN_DMD_SCLK      8   // D8
#define PIN_DMD_R_DATA    51   // D51_MOSI is SPI Master Out if SPI is used
//Define this chip select pin that the Ethernet W5100 IC or other SPI device uses
//if it is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
#define PIN_OTHER_SPI_nCS 1 */

#include <EEPROM.h>
#include "RTClib.h" 
#include "SPI.h"
#include "DMD.h"
#include "TimerOne.h"
#include "System6x7.h"
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include "Arial_14.h"
#include "SystemFont3x5.h"
#define DISPLAYS_ACROSS 4
#define DISPLAYS_DOWN 2

RTC_DS1307 rtc; 
DMD dmd( DISPLAYS_ACROSS , DISPLAYS_DOWN );

char daysOfTheWeek [7] [4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
char monthsOfTheYear [12] [4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

// EEPROM addressing
const byte EEPROM_ID = 0x99;  // initializing EEPROM ID
int addr_0 = 0;               // ID address

//char data[200];

  //="WELCOME TO THE DEPARTMENT OF COMPUTER SCIENCE AND ENGINEERING";
// = "DR. OKE";

String textToScroll, meridiem, Hour, Mins, hodName, session, BT_input;
int mode =1;

bool readEEPROM = true;
int hr, mins, hodNameLen, sessionLen;
unsigned long currentTime = 0;

void setup() {
  
  Serial.begin(9600);
  // communication with the BT module on serial1
  Serial1.begin(9600);
  Timer1.initialize( 5000 );
  Timer1.attachInterrupt( ScanDMD );
  dmd.clearScreen( true );

  //check if the RTC module is connected
  if (!rtc.begin())
  {
      Serial.println("COULD'NT FIND RTC");
      dmd.selectFont(SystemFont3x5);
      dmd.drawString(0, 18, "COULD'NT FIND RTC", 18, GRAPHICS_NORMAL); // the stationary string
      delay(1500);
      dmd.clearScreen( true );
  }
  if (! rtc.isrunning())
  {
     Serial.println("RTC is NOT running");
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(15, 8, "RTC IS NOT RUNNING", 18, GRAPHICS_NORMAL); // the stationary string
      delay(2000);
      dmd.clearScreen( true );
 }

   dmd.selectFont(SystemFont3x5);
   dmd.drawString(5, 0, "ANDROID BASED MOVING MESSAGE", 28, GRAPHICS_NORMAL); // the stationary string
   dmd.drawString(50, 6, "DISPLAY", 7, GRAPHICS_NORMAL); // the stationary string
   dmd.drawString(60, 13, "BY:", 3, GRAPHICS_NORMAL); // the stationary string
   delay(3000);
   drawTextToScroll("FALEYE O.J 150521, YAYI N.A 143998, ARIBIDESI A.T 142491, OLADIPO J.O 142754");
   dmd.clearScreen( true );

   dmd.selectFont(SystemFont3x5);
   dmd.drawString(8, 0, "BEING A PROJECT SUBMITTED TO", 28, GRAPHICS_NORMAL); // the stationary string
   dmd.drawString(4, 6, "DEPARTMENT OF COMPUTER SCIENCE", 30, GRAPHICS_NORMAL); // the stationary string
   dmd.drawString(40, 13, "AND ENGINEERING", 15, GRAPHICS_NORMAL); // the stationary string
   delay(1500);   
   drawTextToScroll("FACULTY OF ENGINEERING AND TECHNOLOGY LAUTECH OGBOMOSO, NIGERIA");
   dmd.clearScreen( true );

   dmd.selectFont(SystemFont3x5);
   dmd.drawString(22, 0, "IN PARTIAL FULFILMENT ", 21, GRAPHICS_NORMAL); // the stationary string
   dmd.drawString(27, 6, "OF THE REQUIREMENTS", 19, GRAPHICS_NORMAL); // the stationary string
   dmd.drawString(32, 13, "FOR THE AWARD OF", 16, GRAPHICS_NORMAL); // the stationary string
   delay(1500);
   drawTextToScroll("BACHELOR OF TECHNOLOGY (B.TECH) DEGREE IN COMPUTER SCIENCE AND ENGINEERING DECEMBER 2020");
   dmd.clearScreen( true );
   
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //auto update from computer time
  //rtc.adjust(DateTime(2018, 11, 29, 18, 15, 10)); // to set the time manually
}

void loop() {
  //EEPROM.write(addr_0, 0x87);
  
  if (readEEPROM == true) {
    EEPROM_read_write(textToScroll, hodName, session); // read/write operation is performed
    readEEPROM = false;
  } 
  
  hodNameLen = hodName.length();
  sessionLen = session.length();
//  Serial.println(hodNameLen);
//  Serial.println(sessionLen);
//  Serial.println(textToScroll.length());
//  Serial.println(hodName);
//  Serial.println(session);
  //Serial.println(textToScroll);

  DateTime now = rtc.now(); //this time stores real time in DateTime now
  //Serial.println(now.hour(), DEC);
  delay(1000);
  hr = now.hour();  
  if (hr >= 12)  {// convert to 12 hour format
    hr = hr - 12;
    if (hr == 0) hr = 12;
    meridiem = "PM";
  }
  else {
    meridiem = "AM";
  }

  Hour = getTime(hr);
  Mins = getTime(now.minute());
  String Time = Hour + ":" + Mins + meridiem;
  String Day = String(daysOfTheWeek[now.dayOfTheWeek()]) + ", " + String(now.day());
  String monthYear = String(monthsOfTheYear[now.month() - 1]) + " " + String(now.year());
  String theMonth =  String(monthsOfTheYear[now.month() - 1]);
  
  if (Serial1.available()) {
      mode = Serial1.read();
      //Serial.println(mode);
   }

   while (mode == 3) {  
    // Wait for the new textToScroll
     if (Serial1.available()) {
      BT_input=Serial1.readString();
     // Serial.println(BT_input);
      if (BT_input == 1) {
        mode = 1;
      }
      else {
        //Serial.println(BT_input);
        textToScroll = BT_input;
        textToScroll.toUpperCase();
        Serial1.write("Text to scroll changed Successfully");
        writeStringToEEPROM(100, textToScroll);
        readEEPROM = true;
        mode = 1;
      }
    }
  }

  while (mode == 4) {  
    // Wait for the new hod name
    //Serial.println("SET NAME");
     if (Serial1.available()) {
      BT_input=Serial1.readString();
      if (BT_input == 1) {
        mode = 1;
      }
      else {
        //Serial.println(BT_input);
        if (BT_input.length() <= 20) { // write the new hodName
            hodName = BT_input;
            hodName.toUpperCase();
            hodNameLen = hodName.length();
            Serial1.write("HOD Name changed Successfully");
            writeStringToEEPROM(1, hodName);
            readEEPROM = true;
            mode = 1;
        } else { 
          Serial1.write("HOD Name should be maximum of 20 characters");
          mode = 1; 
          }  // exit the loop and leave the existing hodName
      }
    }
  }

   while (mode == 5) {
     // Wait for the new date and time string
     if (Serial1.available()) {
        BT_input=Serial1.readString();
        if (BT_input == "1") {
        mode = 1;
      }
      else {
        //dateTime = BT_input;
        setDateTime(BT_input);
        Serial1.write("Time and Date changed Successfully");
        mode = 1;
      }
     } 
   }

   while (mode == 6) {  
    // Wait for the new hod name
     if (Serial1.available()) {
      BT_input=Serial1.readString();
      if (BT_input == 1) {
        mode = 1;
      }
      else {
        //Serial.println(BT_input);
        if (BT_input.length() <= 30) { // write the new hodName
            session = BT_input;
            session.toUpperCase();
            sessionLen = session.length();
            Serial1.write("Semester changed Successfully");
            writeStringToEEPROM(50, session);
            readEEPROM = true;
            mode = 1;
        } else { 
          Serial1.write("Semester should be maximum of 30 characters");
          mode = 1; 
          }  // exit the loop and leave the existing session
      }
    }
  }

  //First set of text to display
  dmd.selectFont(SystemFont3x5);
  dmd.drawString(0, 0, "COMPUTER ENGINEERING", 20, GRAPHICS_NORMAL); // the stationary string
  dmd.drawString(20, 6, "DEPARTMENT", 10, GRAPHICS_NORMAL); // the stationary string

  dmd.selectFont(SystemFont3x5);
  dmd.drawString( 2,13, "HOD:", 4, GRAPHICS_NORMAL);
  dmd.drawString(19, 13, hodName.c_str(), hodNameLen, GRAPHICS_NORMAL); // the stationary string

  dmd.selectFont( SystemFont5x7 );
  dmd.drawString( 84,2, Time.c_str(), 7, GRAPHICS_NORMAL);
  
  dmd.selectFont( SystemFont3x5 );
  dmd.drawString( 84,12, Day.c_str(), 7, GRAPHICS_NORMAL);
  dmd.drawString( 113, 12, theMonth.c_str(), 3, GRAPHICS_NORMAL);

  drawTextToScroll(textToScroll);
  dmd.clearScreen( true );

  
  //Second set of text to display
  dmd.selectFont (SystemFont5x7);
  dmd.drawString(0,-4,"vvvvv",5,GRAPHICS_NORMAL);
  dmd.drawString(103,-4,"vvvvvv",5,GRAPHICS_NORMAL);
  currentTime = millis();
  while (currentTime + 10000 >= millis()) {
    DateTime now = rtc.now(); //this time stores real time in DateTime now
    dmd.selectFont ( Arial_Black_16 );
    dmd.drawString( 32, 1, Hour.c_str(), 2, GRAPHICS_NORMAL);
    if (now.second()% 2 != 0) {
      dmd.drawBox(52, 5, 54, 6, GRAPHICS_NORMAL ); // dmd.drawBox(x1, y1, x2, y2, GRAPHICS_NORMAL );
      dmd.drawBox(52, 8, 54, 9, GRAPHICS_NORMAL ); // dmd.drawBox(x1, y1, x2, y2, GRAPHICS_NORMAL );
    }
    else {
      dmd.drawBox(52, 5, 54, 6, GRAPHICS_NOR ); // dmd.drawBox(x1, y1, x2, y2, GRAPHICS_NORMAL );
      dmd.drawBox(52, 8, 54, 9, GRAPHICS_NOR );
    }
    dmd.drawString( 58,1, Mins.c_str(), 2, GRAPHICS_NORMAL);
    dmd.drawString( 77,1, meridiem.c_str(), 2, GRAPHICS_NORMAL);
  
    //dmd.selectFont ( Arial_Black_16 );
    if (now.hour() < 12) {     
      dmd.drawString( 1,16, "GOOD MORNING", 12, GRAPHICS_NORMAL);
    }
    if (now.hour() >= 12 && now.hour() < 16) {
      dmd.selectFont( Arial_14 );
      dmd.drawString( 2,16, "GOOD AFTERNOON", 14, GRAPHICS_NORMAL);
    }
    if (now.hour() >= 16 && now.hour() < 24) {
      //dmd.selectFont ( Arial_Black_16 );
      dmd.drawString( 1,16, "GOOD EVENING", 12, GRAPHICS_NORMAL);
    }
  }
      dmd.clearScreen( true );
    
    //Third set of text to display
    dmd.selectFont ( System6x7 );
    dmd.drawString( 1,3, Hour.c_str(), 2, GRAPHICS_NORMAL);
    dmd.drawBox(15, 5, 16, 6, GRAPHICS_NORMAL ); // dmd.drawBox(x1, y1, x2, y2, GRAPHICS_NORMAL );
    dmd.drawBox(15, 8, 16, 9, GRAPHICS_NORMAL ); // dmd.drawBox(x1, y1, x2, y2, GRAPHICS_NORMAL );
    dmd.drawString( 18,3, Mins.c_str(), 2, GRAPHICS_NORMAL);
    dmd.drawString( 32,3, meridiem.c_str(), 2, GRAPHICS_NORMAL);

    dmd.selectFont( SystemFont3x5 );
    dmd.drawString( 52,2, session.c_str(), sessionLen, GRAPHICS_NORMAL);
    dmd.drawString( 52,12, Day.c_str(), 7, GRAPHICS_NORMAL);
    dmd.drawString( 82, 12, monthYear.c_str(), 8, GRAPHICS_NORMAL);
    drawTextToScroll(textToScroll);
    dmd.clearScreen( true );

} // end of void loop()

void ScanDMD() {
  dmd.scanDisplayBySPI();
}

void drawTextToScroll (String dispString) {
  unsigned long time;
  int n;
  boolean ret = false;
  int stringLength = dispString.length();

  dmd.selectFont( Arial_Black_16 );
  //dmd.selectFont( Arial_14 );
  
  dmd.drawMarquee(dispString.c_str(), stringLength, ( 32*DISPLAYS_ACROSS )-1, 19); // set up the marquee
  time = millis();
  
    while (!ret) {
      if ((time+30) < millis()) {
        ret = dmd.stepSplitMarquee(19, 31); // parameters are the top & bottom rows to be scrolled
        time = millis();
      }
    }
    ret = false;
   
}

// this function prefix time with "0" if time is less than 10
String getTime(int Time) {
  char prefix = '0';
  if (Time < 10) {
    String result = prefix + String(Time);
    //Time = result.toInt();
    return result;
  }

  return String(Time);
} // end of getTime()


// This function split string. It takes two arg the String to split and the char to be used to split.
String * split(String dateTime, String Char) {
  
  int i = 0;
  int charPosition = 0; 
  static String result[3];  // static pointer string array
 
  do  {
    charPosition = dateTime.indexOf(Char);
    if(charPosition != -1) {
      //Serial.println( dateTime.substring(0, charPosition));
      result[i] = dateTime.substring(0, charPosition);  //return text starting from index 0 and less than charPosition(index)
      dateTime = dateTime.substring(charPosition+1, dateTime.length()); // overwrite dateTime with text after charPosition 
      i++;  // increment the array
    }
    else  { // here after the last char is found
      if(dateTime.length() > 0) {
      // if there is text after the last char, store it
      result[i] = dateTime;
        }
      }
    }
  while(charPosition >=0);

  return result;
} // end of split()


// this function update the date and time 
void setDateTime(String dateTime) {

  String *dateAndTime;
  String *splittedDate;
  String *splittedTime;
  
  int BT_year, BT_month, BT_day, BT_hour, BT_min, BT_second = 0;

  dateTime.trim();
  dateAndTime = split(dateTime, "/");

  String date = *(dateAndTime + 0);
  String Time = *(dateAndTime + 1);

  splittedDate = split(date, "-");
  BT_year = String(*(splittedDate + 0)).toInt();
  BT_month = String(*(splittedDate + 1)).toInt();
  BT_day = String(*(splittedDate + 2)).toInt();

  splittedTime = split(Time, "-");
  BT_hour = String(*(splittedTime + 0)).toInt();
  BT_min = String(*(splittedTime + 1)).toInt();
  BT_second = String(*(splittedTime + 2)).toInt();
  
  rtc.adjust(DateTime(BT_year, BT_month, BT_day, BT_hour, BT_min, BT_second));
  
  } // end of setDateTime()


void writeStringToEEPROM(int addrOffset, const String &strToWrite) { 
  Serial.println("writing..");
  int count = 1;
  int len = strToWrite.length(); 
  if (len > 800) return len;  // return if the remaining character length is too much for the EEPROM
  //Serial.println(len);
  if (len > 254) {  // if string length is greater than a byte(255)
    EEPROM.write(addrOffset, 254);  // write 254 to EEPROM
    int remainder = len - 254;      //  get the remainder
    while (remainder > 254) {       // while the remainder is still greater than 254
       EEPROM.write(addrOffset + count, 254); // write 254 to the next address
       remainder = remainder - 254;
       count++;   //  increment address
       if (count == 3) break; // break bcus we dont want length of charact to exceed EEPROM limit
    }
    
    byte remByte = remainder;
    //Serial.println(remByte);
    EEPROM.write(addrOffset + count, remByte);  // store the remaining length in EEPROM
    count++;  // move to the next address
  }
  else { // otherwise
    EEPROM.write(addrOffset, len);  
    Serial.println("writing..");
  }
   
  for (int i = 0; i < len; i++) { 
    EEPROM.write(addrOffset + count + i, strToWrite[i]);  // write characters to EEPROM
    } 
    
    //return len; // return length of characters
 } // end of writeStringToEEPROM()

String readStringFromEEPROM(int addrOffset) { 
  Serial.println("reading..");
    int count = 1;
    int totalLen = 0;
    int newStrLen = EEPROM.read(addrOffset);
    totalLen = newStrLen;
    
    while (newStrLen == 254) {
      newStrLen = EEPROM.read(addrOffset + count);
      totalLen += newStrLen;
      //Serial.println(totalLen);
      count++;
    }

    //Serial.println(count);
    //Serial.println(totalLen);
    
     char data[totalLen + 1]; 
    for (int i = 0; i < totalLen; i++) { 
     int offset = addrOffset + count + i;
      data[i] = EEPROM.read(offset); 
     } 

     data[totalLen] = '\0';  

        String result = "";
    for (int i=0; i<sizeof(data); i++) {
      result+=data[i];
    }
    
    //delay(1000);
     
     //Serial.println(result);
     
    //Serial.println(String(data));
     
    return result;  
} 

void EEPROM_read_write(String &textToScroll, String &hodName, String &session) {
  byte id = EEPROM.read(addr_0); // read the first byte from the EEPROM
  if( id == EEPROM_ID) {
    Serial.println("read....");
     textToScroll = readStringFromEEPROM(100);
     hodName = readStringFromEEPROM(1); 
     session = readStringFromEEPROM(50);  
  }
  else {// this code is executed once after the program is uploaded on the controller
    Serial.println("write....");
    EEPROM.write(addr_0, EEPROM_ID);
    writeStringToEEPROM(100, textToScroll);
    writeStringToEEPROM(1, hodName);
    writeStringToEEPROM(50, session); 
  }
}
