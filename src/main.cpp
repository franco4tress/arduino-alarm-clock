#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <string.h>
#include <TM1637.h>

#define CLK 11
#define DIO 10

TM1637 tm(CLK, DIO);

void updateScreen();
// void checkAlarms();
String formatTimePart(int part);
void setMemory(RtcDS1302<ThreeWire>* rtc, const char* data);
char* getMemory();
// void buzz();

const int upPin = 6;
const int downPin = 7;
const int setPin = 8;
const int modePin = 9;
bool upPrev = false;
bool downPrev = false;
bool setPrev = false;
bool modePrev = false;
int counter = 0;
int mode = 0;
int alarms = 27;


bool initial = true;

const int BUZZER_PIN = 2;

const int RST = 3;
const int IO = 4;
const int SCLK = 5;

int hours = 0;
int minutes = 0;

const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const char data[] = "Jan 01 1970";

bool setTimeMode = false;
bool second = true;
bool comingBackToClock = true;

RtcDateTime current;

ThreeWire myWire(IO, SCLK, RST);
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(setPin, INPUT);
  pinMode(modePin, INPUT);

  tm.begin();
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(9600);

  Rtc.Begin();

  char* buff = getMemory();

  RtcDateTime dt = RtcDateTime(buff, "23:59:55");
  current = dt;

  Rtc.SetDateTime(dt);
  /*
  if (!Rtc.IsDateTimeValid()) {
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    Rtc.SetDateTime(dt);
  }
  */

  if (Rtc.GetIsWriteProtected()) {
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  updateScreen();
}

void setDateOnMemory() {
    RtcDateTime now = Rtc.GetDateTime();

    // Prepare new date to be set into the mem
    char date[12];
    int dd = sprintf(date, "%s %02d %04d", monthNames[now.Month() - 1], now.Day(), now.Year());
    uint8_t count = sizeof(date);

    uint8_t written = Rtc.SetMemory((const uint8_t*)date, count); // this includes a null terminator for the string
    if (written != count) 
    {
        Serial.print("something didn't match, count = ");
        Serial.print(count, DEC);
        Serial.print(", written = ");
        Serial.print(written, DEC);
        Serial.println();
    }
}

char* getMemory() {
  uint8_t buff[20];
  const uint8_t count = sizeof(buff);
  uint8_t gotten = Rtc.GetMemory(buff, count);

  if (gotten != count) {
      return __DATE__;
  } else {
    char* memDate = (char*)&buff;
    return memDate;
  }
}

void loop() {
  if (digitalRead(upPin) == HIGH) {
    if (upPrev == false) {
      upPrev = true;
      counter += 1;
      if (mode == 1) {
        if (counter + 1 > alarms) {
          counter = 0;
        }
        updateScreen();
      } else if (mode == 2) {
        if (hours == 23) {
          hours = 0;
        } else {
          hours = hours + 1;
        }
      }
      Serial.println(counter);
    }
  } else {
    upPrev = false;
  }
  
  if (digitalRead(downPin) == HIGH) {
    if (downPrev == false) {
      downPrev = true;
      counter -= 1;
      if (mode == 2) {
        if (hours == 0) {
          hours = 23;
        } else {
          hours = hours - 1;
        }
      }
      Serial.println(counter);
    }
  } else {
    downPrev = false;
  }

  if (digitalRead(setPin) == HIGH) {
    if (setPrev == false) {
      setPrev = true;
      Serial.println("SET");
    }
  } else {
    setPrev = false;
  }

  if (digitalRead(modePin) == HIGH) {
    if (modePrev == false) {
      modePrev = true;
      if (mode < 2) {
        mode += 1;
        counter = 0;
      } else {
        initial = true;
        mode = 0;
      }
      if (mode == 2) {
        RtcDateTime now = Rtc.GetDateTime();
        hours = now.Hour();
        minutes = now.Minute();
      } else {
        hours = 0;
        minutes = 0;
      }
      Serial.print("mode=");
      Serial.println(mode);
      updateScreen();
    }
  } else {
    modePrev = false;
  }

  if (mode == 0 || mode == 2) {
    updateScreen();
  }

  // checkAlarms();
}

void updateScreen() {
  RtcDateTime now = Rtc.GetDateTime();

  if (mode == 0) {
    if (now.Minute() != current.Minute() || comingBackToClock) {
      char time[4];
      int c2 = sprintf(time, "%02d%02d", now.Hour(), now.Minute());
      uint8_t timeCnt = sizeof(time);
      String fmtTime = time;
      tm.display(fmtTime);
      comingBackToClock = true;
    }

    if (now.Second() != current.Second()) {
      second = !second;
      if (second) {
        tm.colonOn();
      } else {
        tm.colonOff();
      }
    }
    
    if (current.Day() != now.Day()) {
        setDateOnMemory();
    }

    current = now;

  } else if (mode == 1) {
    if (alarms == 0) {
      tm.display("NO");
    } else {
      int index = 0;
      // There CANNOT be more than 99 alarms.
      if (counter + 2 <= alarms) {
        for (int i = counter + 1; i <= counter + 2; ++i) {
          char alarm[4] = "   ";
          int c = sprintf(alarm, "AL-%02d", 1);
          String formated = alarm;
          tm.display(formated);
          index += 1;
        }
      } else {
        char alarm[4] = "   ";
        int c = sprintf(alarm, "AL-%02d", 1);
        String formated = alarm;
        tm.display(formated);
      }
    }
  } else if (mode == 2) {
    char time[4];
    int c2 = sprintf(time, "%02d%02d", hours, minutes);
    uint8_t timeCnt = sizeof(time);
    String fmtTime = time;
    tm.display(fmtTime);
  }
}
//}
