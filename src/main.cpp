#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

void updateTime(bool);
void checkAlarms();
void buzz();

LiquidCrystal_I2C lcd(0x27,16,2);

const int BUZZER_PIN = 2;

const int RST = 3;
const int IO = 4;
const int SCLK = 5;

RtcDateTime current;
RtcDateTime alarms[] = {RtcDateTime("Sep 07 2024", "00:42:59")};

ThreeWire myWire(IO, SCLK, RST);
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  lcd.init();
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(9600);

  Rtc.Begin();
  RtcDateTime dt = RtcDateTime(__DATE__, __TIME__);
  
  current = dt;

  Rtc.SetDateTime(dt);

  /*
  if (!Rtc.IsDateTimeValid()) {
    Rtc.SetDateTime(dt);
  }
  */  

  if (Rtc.GetIsWriteProtected()) {
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  updateTime(true);
}

void loop() {
  delay(150);
  updateTime(false);
  checkAlarms();
}

String formatTimePart(int part) {
  if (part < 10) {
    return "0" + String(part);
  }
  return String(part);
}

void updateTime(bool init) {
  RtcDateTime now = Rtc.GetDateTime();

  if (init) {
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
  }
  if (init | current.Month() != now.Month()) {
    lcd.setCursor(0,0);
    lcd.print(formatTimePart(now.Month()));
  }
  if (init | current.Day() != now.Day()) {
    lcd.setCursor(3,0);
    lcd.print(formatTimePart(now.Day()));
  }
  if (init | current.Year() != now.Year()) {
    lcd.setCursor(6,0);
    lcd.print(now.Year());
  }
  if (init | current.Second() != now.Second()) {
    lcd.setCursor(6,1);
    lcd.print(formatTimePart(now.Second()));
  }
  if (init | current.Minute() != now.Minute()) {
    lcd.setCursor(3,1);
    lcd.print(formatTimePart(now.Minute()));
  }
  if (init | current.Hour() != now.Hour()) {
    lcd.setCursor(0,1);
    lcd.print(formatTimePart(now.Hour()));
  }

  current = now;
}


void checkAlarms() {
  for (RtcDateTime alarm : alarms) {
    if (alarm == current) {
      buzz();
    }
  }
}


void buzz() {
  // lcd.print("Alarm");
}