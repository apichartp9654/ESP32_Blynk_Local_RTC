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
char auth[] = "yourtoken"; // pi2 local

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

// Check current time with timer array value
bool IsONTIME(int id, int cTime)
{
   TimeInterval t = checkTimes[id];

   bool ret = (t.Start <= cTime)&&(cTime <= t.End);
   return ret;
}

//Display rtc time to label widget
void ShowTimeFromRTC()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + "/" + month() + "/" + year();
  String v = currentDate + " " + currentTime;
  Blynk.virtualWrite(V3, v);
}

// validate current time with timers
void validateTimer()
{
  checkRelayState();
  after1stCycle = true;
  ShowTimeFromRTC();
  if(selectedMode == 2){return;} // not allow to change state in manual mode
   
   int hnow = (hour() * 60) + minute();
   bool nState = false;
   
   Serial.print("Time =: ");
   Serial.print(hnow);

   int relay = 0;
   for (int i = 0;  i < 4; i++)
   {
     
     if(IsONTIME(i, hnow))
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

   
}

// Check output pins state and update to LED widget
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

// initial timers interval
void SetTimeIntervals()
{
  checkTimes[0].Start = 1 *60;
  checkTimes[0].End = 4 * 60;
  
  checkTimes[1].Start = 7 *60;
  checkTimes[1].End = 10 * 60;
  checkTimes[2].Start = 13 *60;
  checkTimes[2].End = 16 * 60;
  checkTimes[2].RelayID = 2;
  checkTimes[3].Start = 19 *60;
  checkTimes[3].End = 22 * 60;
  checkTimes[3].RelayID = 2;
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
// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget

// Terminal widget
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

//Button Widget for Relay1
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

//Button Widget for relay2
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

// Menu Widget  for mode select
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

//Time Input widget 
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

// Menu Widget  for relay selection
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

// Menu Widget  for timer selection
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

// Button widget for Set Timer value
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
     String s = "Timer " + String(timerID) + ".Start = " + String(checkTimes[timerID].Start) + " .End = " + String(checkTimes[timerID].End) + " .Relay = " + String(checkTimes[timerID].RelayID);
     Serial.println(s); 
  }
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




