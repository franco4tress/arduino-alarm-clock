#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <string.h>

void updateScreen();
// void checkAlarms();
String formatTimePart(int part);
void setMemory(RtcDS1302<ThreeWire>* rtc, const char* data);
char* getMemory();
// void buzz();

LiquidCrystal_I2C lcd(0x27,16,2);


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

const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const char data[] = "Jan 01 1970";

RtcDateTime current;
// RtcDateTime alarms[] = {RtcDateTime("Sep 07 2024", "00:00:00")};

ThreeWire myWire(IO, SCLK, RST);
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(setPin, INPUT);
  pinMode(modePin, INPUT);

  lcd.init();
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(9600);

  Rtc.Begin();

  char* buff = getMemory();

  Serial.print("Getting: ");
  Serial.println(buff);  

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

void setMemory() {
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
  if (mode == 0) {
    updateScreen();
  }
  if (digitalRead(upPin) == HIGH) {
    if (upPrev == false) {
      upPrev = true;
      counter += 1;
      if (mode == 1) {
        if (counter + 1 > alarms) {
          counter = 0;
        }
        updateScreen();
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
      updateScreen();
    }
  } else {
    modePrev = false;
  }
  // checkAlarms();
}

String formatTimePart(int part) {
  if (part < 10) {
    return "0" + String(part);
  }
  return String(part);
}

void updateScreen() {
  if (mode == 0) {
    RtcDateTime now = Rtc.GetDateTime();
    if (initial) {
      lcd.clear();
      lcd.backlight();

      lcd.setCursor(2,0);
      lcd.print("/");
      lcd.setCursor(5,0);
      lcd.print("/");

      lcd.setCursor(2,1);
      lcd.print(":");
      lcd.setCursor(5,1);
      lcd.print(":");

      initial = false;
    }

    // Prepare new date to be shown
    char date[12];
    int c1 = sprintf(date, "%s %02d %04d", monthNames[now.Month() - 1], now.Day(), now.Year());
    uint8_t dateCnt = sizeof(date);

    // Prepare new time to be shown
    char time[8];
    int c2 = sprintf(time, "%02d:%02d:%02d", now.Hour(), now.Minute(), now.Second());
    uint8_t timeCnt = sizeof(time);

    lcd.setCursor(0,0);
    lcd.print(date);

    lcd.setCursor(0,1);
    lcd.print(time);

    if (current.Day() != now.Day()) {
        setMemory();
    }

    current = now;
  } else if (mode == 1) {
    if (alarms == 0) {
      lcd.setCursor(0,0);
      lcd.print("You have no     ");
      lcd.setCursor(0,1);
      lcd.print("Alarms set.     ");
    } else {
      int index = 0;
      // There CANNOT be more than 99 alarms.
      if (counter + 2 <= alarms) {
        for (int i = counter + 1; i <= counter + 2; ++i) {
          lcd.setCursor(0,index);
          lcd.print("Alarm ");
          lcd.setCursor(6,index);
          lcd.print(i);
          lcd.setCursor(6+String(i).length(),index);
          lcd.print("        ");
          index += 1;
        }
      } else {
        lcd.setCursor(0,0);
        lcd.print("Alarm ");
        lcd.setCursor(6,0);
        lcd.print(counter + 1);
        lcd.setCursor(6+String(counter + 1).length(),0);
        lcd.print("        ");
        lcd.setCursor(0,1);
        lcd.print("                ");
      }
    }
  } else if (mode == 2) {
    Serial.println("Case 2");
  }
}
//}
