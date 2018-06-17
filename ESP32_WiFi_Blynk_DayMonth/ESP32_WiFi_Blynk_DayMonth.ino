/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP32 chip.

  Note: This requires ESP32 support package:
    https://github.com/espressif/arduino-esp32

  Please be sure to select the right ESP32 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

#define RELAY_PIN            17    // R1 output pin
#define RELAY_PIN2           16    // R2 output pin
//#include "DHT.h"
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon). 
char auth[] = "yourToken"; // pi2 local

//********************* blobal variable ************************
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ssid";
char pass[] = "pass";

BlynkTimer timer;
WidgetRTC rtc;
WidgetTerminal terminal(V1);
WidgetLED led1(V8);
WidgetLED led2(V7);

//********************* End blobal variable ************************

//********************* helper function ****************************

class TimeInterval{
  public:
     int Start = 0;
     int End = 0;
     int RelayID = 1;
     int sDay = 1; // start day element for chek time interval( 1 - 31)
     int sMonth = 1; // start month element for chek time interval( 1 - 12)
     int eDay = 1; // end day element for chek time interval( 1 - 31)
     int eMonth = 1; // end month element for chek time interval( 1 - 12)
};

TimeInterval t1;
TimeInterval t2;
TimeInterval t3;
TimeInterval t4;

TimeInterval checkTimes[] = {t1,t2,t3,t4};

void requestTime() {
  Blynk.sendInternal("rtc", "sync");
}



bool curState = false;
bool curState2 = false;

int selectedMode = 1; // manual
bool after1stCycle = false;


bool IsONTIME(int id, int cTime, int cday, int cmonth)
{
   TimeInterval t = checkTimes[id];

   bool checkMonth = (t.sMonth <= cmonth)&&(cmonth <= t.eMonth) ;
   bool checkDay = (t.sDay <= cday)&&(cday <= t.eDay) ;
   bool ret = (t.Start <= cTime)&&(cTime <= t.End) ;
   return ret && checkMonth && checkDay;
}

void ShowTimeFromRTC()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + "/" + month() + "/" + year();
  String v = currentDate + " " + currentTime;
  Blynk.virtualWrite(V3, v);
}

void validateTimer()
{
  checkRelayState();
  after1stCycle = true;
  ShowTimeFromRTC();
  if(selectedMode == 2){return;} // not allow to change state in manual mode
   
   int hnow = (hour() * 60) + minute();
   int cMonth = month();
   int cDay = day();
   bool nState = false;
   
   Serial.print("Time =: ");
   Serial.print(hnow);

   int relay = 0;
   for (int i = 0;  i < 4; i++)
   {
     
     if(IsONTIME(i, hnow,cDay, cMonth ))
     {
      // present time is inside checkTimes[i] interval
        nState = true;
        Serial.print(" Valid to timer ");
        Serial.print(i);
        relay = checkTimes[i].RelayID;
        break;
     }
     else
     {
       // present time is not inside checkTimes[i] interval
     }
   }

   if(!nState)
   {
    // no timer active at this time then reset all output relay
      Serial.print(" not Valid to any timer ");
      Serial.println(" ->OFF time");
      digitalWrite(RELAY_PIN, LOW);   //send active low to OFF relay
      //Blynk.virtualWrite(V8, 0);
      led1.off();
      digitalWrite(RELAY_PIN2, LOW);   //send active low to OFF relay
      //Blynk.virtualWrite(V7, 0);
      led2.off();
   }
   else
   {

    //Some timer active at this time
     Serial.print(" ->ON time");
     Serial.print(" ->Relay ");
     Serial.print(relay);
   }

    setRelayState(relay, (relay == 1)? RELAY_PIN  : RELAY_PIN2, nState);
/*
    if(relay == 1)
    {
       Serial.print(" -> current state [");
       Serial.print(curState);
       Serial.print("] ");
       if(nState != curState)
       {
         Serial.println(" state Changed.");
         curState = nState;
    
         if(nState)
         {
          digitalWrite(RELAY_PIN, HIGH);  //send active high to ON relay
          //Blynk.virtualWrite(V1, 1);
          Blynk.virtualWrite(V8, 1);
          led1.on();
         }
         else
         {
          digitalWrite(RELAY_PIN, LOW);   //send active low to OFF relay
          //Blynk.virtualWrite(V1, 0);
          Blynk.virtualWrite(V8, 0);
          led1.off();
         }
       }
       else
       {
          Serial.println(" state not Change.");
       }
    }
    else if(relay == 2)
    {
       Serial.print(" -> current state [");
       Serial.print(curState2);
       Serial.print("] ");
       if(nState != curState2)
       {
         Serial.println(" state Changed.");
         curState2 = nState;
    
         if(nState)
         {
          digitalWrite(RELAY_PIN2, HIGH);  //send active high to ON relay
          //Blynk.virtualWrite(V2, 1);
          Blynk.virtualWrite(V7, 1);
          led2.on();
         }
         else
         {
          digitalWrite(RELAY_PIN2, LOW);   //send active low to OFF relay
          //Blynk.virtualWrite(V2, 0);
          Blynk.virtualWrite(V7, 0);
          led2.off();
         }
       }
       else
       {
          Serial.println(" state not Change.");
       }
    }
    */

   
}

void setRelayState(int relayNo, int Pin, bool state)
{
       int vPin = (relayNo == 1)? V8:V7;
       bool cState = (relayNo == 1)? curState : curState2;
       WidgetLED led = (relayNo == 1) ? led1 :led2;
       Serial.print(" -> current state [");
       Serial.print(cState);
       Serial.print("] ");
      
       if(state != cState)
       {
         Serial.println(" state Changed.");
         if(relayNo == 1)
         {
          curState = state;
         }
         else
         {
          curState2 = state;
         }
         
    
         if(state)
         {
          digitalWrite(Pin, HIGH);  //send active high to ON relay
          Blynk.virtualWrite(vPin, 1);
          led.on();
         }
         else
         {
          digitalWrite(Pin, LOW);   //send active low to OFF relay
          Blynk.virtualWrite(vPin, 0);
          led.off();
         }
       }
       else
       {
          Serial.println(" state not Change.");
       }
}

void checkRelayState()
{
  boolean pressed = (digitalRead(RELAY_PIN) == HIGH);
  if (pressed) {
      led1.on();
    } else {
      led1.off();
    }
  curState = pressed;

  boolean pressed2 = (digitalRead(RELAY_PIN2) == HIGH);
  if (pressed2) {
      led2.on();
    } else {
      led2.off();
    }
  curState2 = pressed2;
}

// Initial timer at first start
void SetTimeIntervals()
{
  //int sDay = 1; // start day element for chek time interval( 1 - 31)
    // int sMonth = 1; // start month element for chek time interval( 1 - 12)
    // int eDay = 1; // end day element for chek time interval( 1 - 31)
    // int eMonth = 1; // end month element for chek time interval( 1 - 12)
     
  checkTimes[0].Start = 1 *60;
  checkTimes[0].End = 4 * 60;
  checkTimes[0].sDay = 1;
  checkTimes[0].sMonth = 6;
  checkTimes[0].eDay = 30;
  checkTimes[0].eMonth = 1;
  
  checkTimes[1].Start = 7 *60;
  checkTimes[1].End = 10 * 60;
  checkTimes[1].sDay = 1;
  checkTimes[1].sMonth = 6;
  checkTimes[1].eDay = 30;
  checkTimes[1].eMonth = 1;
  
  checkTimes[2].Start = 13 *60;
  checkTimes[2].End = 16 * 60;
  checkTimes[2].RelayID = 2;
  checkTimes[20].sDay = 1;
  checkTimes[2].sMonth = 6;
  checkTimes[2].eDay = 30;
  checkTimes[2].eMonth = 1;
  
  
  checkTimes[3].Start = 19 *60;
  checkTimes[3].End = 22 * 60;
  checkTimes[3].RelayID = 2;
  checkTimes[3].sDay = 1;
  checkTimes[3].sMonth = 6;
  checkTimes[3].eDay = 30;
  checkTimes[3].eMonth = 1;
  
}


//*********************************************************************

//*********************** Blynk Handler********************************
BLYNK_CONNECTED() {
  
  Blynk.syncAll();
  // Synchronize time on connection
  rtc.begin();
}


int tStart = 0;
int tEnd = 0;
int timerID = 0;
int relayID = 1;

int tempStartDay = 1;
int tempEndDay = 1;
int tempStartMonth = 1;
int tempEndMonth = 1;

// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V0)
{

  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("Marco") == param.asStr()) 
  {
    terminal.println("You said: 'Marco'") ;
    terminal.println("I said: 'Polo'") ;
  } 
  else 
  {
    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
  }

  // Ensure everything is sent
   //terminal.flush();
}

// Handle Button state change from blynk
BLYNK_WRITE(V1) 
{
  if(selectedMode == 1){return;} // not allow to change state in auto mode
  int s = param.asInt();

  curState = s > 0;

   if(curState)
   {
    digitalWrite(RELAY_PIN, HIGH);  //send active high to ON relay
    led1.on();
    terminal.print("Relay 1 On by user in manual mode");
   }
   else
   {
    digitalWrite(RELAY_PIN, LOW);   //send active low to OFF relay
    led1.off();
    terminal.print("Relay 1 Off by user in manual mode");
   }
}

// Handle Button state change from blynk
BLYNK_WRITE(V2) 
{
  if(selectedMode == 1){return;} // not allow to change state in auto mode
  int s = param.asInt();

  curState2 = s > 0;

   if(curState2)
   {
    digitalWrite(RELAY_PIN2, HIGH);  //send active high to ON relay
    led2.on();
    terminal.print("Relay 2 On by user in manual mode");
   }
   else
   {
    digitalWrite(RELAY_PIN2, LOW);   //send active low to OFF relay
    led2.off();
    terminal.print("Relay 2 Off by user in manual mode");
   }
}

// Handle mode change from blynk
BLYNK_WRITE(V9) 
{
  //if(!after1stCycle){return;}
  switch (param.asInt())
  {
    case 1: // Item 1
      selectedMode = 1;
      Serial.println("Mode = Auto");
      terminal.println("Mode = Auto") ;
      break;
    case 2: // Item 2
      selectedMode = 2;
      Serial.println("Mode = Manual");
      terminal.println("Mode = Manual") ;
      break;
    default:
      selectedMode = -1;
      Serial.println("Mode = N/A");
      terminal.println("Mode = N/A") ;
  }
}

String tz = "";
// Notify time setting change from blynk
BLYNK_WRITE(V10) 
{
  if(!after1stCycle){return;}
  // handle start end time
  TimeInputParam t(param);
  tStart = (t.getStartHour() * 60) + t.getStartMinute();
  tEnd = (t.getStopHour() * 60) + t.getStopMinute();

  tz = t.getTZ();
  Serial.println(tz);
  Serial.println(tStart);
  Serial.println(tEnd);
}

// Update Relay No from blynk
BLYNK_WRITE(V11) 
{
  if(!after1stCycle){return;}
  switch (param.asInt())
  {
    case 1: // Item 1
      relayID = 1;
      Serial.println("Relay select = R1");
      terminal.println("Relay select = R1");
      break;
    case 2: // Item 2
      relayID = 2;
      Serial.println("Relay select = R2");
      terminal.println("Relay select = R2");
      break;
    default:
      selectedMode = -1;
      Serial.println("Relay select = None");
      terminal.println("Relay select = None") ;
  }
}

// notify selected Timer from blynk
BLYNK_WRITE(V12) 
{

  if(!after1stCycle){return;}
  switch (param.asInt())
  {
    case 1: // Item 1
      timerID = 0;
      Serial.println("Timer 0 selected");
      terminal.println("Timer 0 selected") ;
      Blynk.setProperty(V13, "label", "Set Timer0");
      Blynk.virtualWrite(V10, checkTimes[0].Start * 60, checkTimes[0].Start * 60, tz);
      Blynk.virtualWrite(V11, checkTimes[0].RelayID);
      break;
    case 2: // Item 2
      timerID = 1;
      Serial.println("Timer 1 selected");
      terminal.println("Timer 1 selected") ;
      Blynk.setProperty(V13, "label", "Set Timer1");
      Blynk.virtualWrite(V10, checkTimes[1].Start * 60, checkTimes[1].Start * 60, tz);
      Blynk.virtualWrite(V11, checkTimes[1].RelayID);
      break;
    case 3: // Item 3
      timerID = 2;
      Serial.println("Timer 2 selected");
      terminal.println("Timer 2 selected") ;
      Blynk.setProperty(V13, "label", "Set Timer2");
      Blynk.virtualWrite(V10, checkTimes[2].Start * 60, checkTimes[2].Start * 60, tz);
      Blynk.virtualWrite(V11, checkTimes[2].RelayID);
      break;
    case 4: // Item 3
      timerID =3;
      Serial.println("Timer 3 selected");
      terminal.println("Timer 3 selected") ;
      Blynk.setProperty(V13, "label", "Set Timer3");
      Blynk.virtualWrite(V10, checkTimes[3].Start * 60, checkTimes[3].Start * 60, tz);
      Blynk.virtualWrite(V11, checkTimes[3].RelayID);
      break;
    default:
      timerID = -1;
      Serial.println("No Timer selected");
      terminal.println("No Timer selected") ;
      Blynk.setProperty(V13, "label", "Set Timer");
  }
}

// Set Timer Trigger from blynk
BLYNK_WRITE(V13) 
{
  if(!after1stCycle){return;} // not update timer when blynk sync at begin
  if(timerID < 0){return;} // if timer id < 0,  not update
  int s = param.asInt();

  if(s == 1)
  {

     checkTimes[timerID].Start = tStart;
     checkTimes[timerID].End = tEnd; 
     checkTimes[timerID].RelayID = relayID;
     checkTimes[timerID].sDay = tempStartDay;
     checkTimes[timerID].eDay = tempEndDay;
     checkTimes[timerID].sMonth = tempStartMonth;
     checkTimes[timerID].eMonth = tempEndMonth;
     String s = "Timer " + String(timerID) + ".Start = " + String(checkTimes[timerID].Start) + " .End = " + String(checkTimes[timerID].End) + " .Relay = " + String(checkTimes[timerID].RelayID);
     s = s + " Start M:D " + String(tempStartMonth) + " : " + String(tempStartDay);
     s = s + " End M:D " + String(tempEndMonth) + " : " + String(tempEndDay);
     Serial.println(s); 
  }
}

/*
int tempStartDay = 1;
int tempEndDay = 1;
int tempStartMonth = 1;
int tempEndMonth = 1;
*/
// Handle Start Day change
BLYNK_WRITE(V14) 
{
  if(!after1stCycle){return;} // not update timer when blynk sync at begin
  tempStartDay = param.asInt();
  String s = "Start Day change to " + String(tempStartDay);
     Serial.println(s); 
  
}

// Handle End Day change
BLYNK_WRITE(V15) 
{
  if(!after1stCycle){return;} // not update timer when blynk sync at begin
  tempEndDay = param.asInt();
  String s = "End Day change to " + String(tempEndDay);
  Serial.println(s); 
  
}

// Handle Start Month change
BLYNK_WRITE(V16) 
{
  if(!after1stCycle){return;} // not update timer when blynk sync at begin
  tempStartMonth = param.asInt();
  String s = "Start Month change to " + String(tempStartMonth);
  Serial.println(s); 
  
}

// Handle End Month change
BLYNK_WRITE(V17) 
{
  if(!after1stCycle){return;} // not update timer when blynk sync at begin
  tempEndMonth = param.asInt();
  String s = "End Month change to " + String(tempEndMonth);
  Serial.println(s); 
  
}

//*********************************************************************


void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass,IPAddress(192,168,1,41), 8080);

  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)

  SetTimeIntervals();
  
  // check hour segment every 10 seconds
  timer.setInterval(1000L, validateTimer);

  pinMode(RELAY_PIN, OUTPUT); // set pin as output to relay R1
  pinMode(RELAY_PIN2, OUTPUT); // set pin as output to relay R2

  // Display current setting time interval in combination of ( hour * 60 + minute)
  for(int i = 0; i < 4; i++)
  {
    String s = "Timer " + String(i) + ".Start = " + String(checkTimes[i].Start) + " .End = " + String(checkTimes[i].End)+ " .Relay = " + String(checkTimes[timerID].RelayID);
    Serial.println(s);
  }
  checkRelayState();
  //ShowTimeFromRTC();
}

void loop()
{
  Blynk.run();
  timer.run();
}




