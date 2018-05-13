/*

  SeventSegment P transistor 12 pic led display
  Mounted upside-down
*/


#include <RTC.h>
#include "SevSeg_upsidedown.h"

int8_t TimeDisp[] = {0x00, 0x00, 0x00, 0x00};
unsigned char ClockPoint = 1;

const int buttonPin = A5;
int buttonState = 0;

RTC  time;
SevSeg sevseg;

boolean init_time = false;
boolean debug = true;

void setup() {
  delay(300);

  if (debug)   Serial.begin (9600);

  // SevenSegment init
  // =========================
  byte numDigits = 4;
  byte digitPins[] = {9, 10, 11, 12};
  byte segmentPins[] = {13, 8, 7, 6, 5, 4, 3, 2};
  sevseg.begin(P_TRANSISTORS, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(0);

  pinMode(buttonPin, INPUT);

  // RealTime clock init
  // ==========================
  time.begin(RTC_DS1302, A0, A1, A2); // на базе чипа DS1302, вывод RST, вывод CLK, вывод DAT
  //  запись даты и времени, если год == 0

  if (init_time) {
    //           сек, мин, час, число, месяц, год, день недели
    time.settime( 0,   33,  22,   12,    5,    18,    7);
  }
}

void loop() {


  time.gettime("s");
  TimeDisp[0] = time.Hours / 10;
  TimeDisp[1] = time.Hours % 10;
  TimeDisp[2] = time.minutes / 10;
  TimeDisp[3] = time.minutes  % 10;

  buttonState = digitalRead(buttonPin);
  if (debug)   Serial.println (buttonState);

  display_number (TimeDisp);
  sevseg.refreshDisplay(); // Must run repeatedly
}

/// END ///


void display_number (int8_t num[])
{
  String str_num = num2str (TimeDisp[3]) + num2str(TimeDisp[2]) + num2str(TimeDisp[1]) + num2str(TimeDisp[0]);

  int flip_num = str_num.toInt();
  sevseg.setNumber(flip_num, 4);
}


String num2str (int num)
{
  String out_str = String (num);
  if (num < 1) out_str = "0";
  return out_str;
}
