#include <AceButton.h>
#include <AdjustableButtonConfig.h>
#include <ButtonConfig.h>

using namespace ace_button;
/*

  SeventSegment P transistor 12 pic led display
  Mounted upside-down
*/


#include <RTC.h>
#include "SevSeg_upsidedown.h"

boolean init_time = false;
boolean debug = false;


int8_t TimeDisp[] = {0x00, 0x00, 0x00, 0x00};
unsigned char ClockPoint = 1;

const int buttonPin = A5;

int setup_phase = 0;
const int max_setup_phase = 1;

const int flash_delay = 500;
unsigned long flash_start;
unsigned long flash_time;
boolean flash_flag = false;

int setup_hour_start;
int setup_min_start;


AceButton button(buttonPin);

RTC  time;
SevSeg sevseg;

boolean setup_mode = false;


void handleEvent(AceButton*, uint8_t, uint8_t);

void setup() {
  delay(300);

  if (debug)   Serial.begin (9600);

  // SevenSegment init
  // =========================
  byte numDigits = 4;
  byte digitPins[] = {9, 10, 11, 12};
  byte segmentPins[] = {13, 8, 7, 6, 5, 4, 3, 2};
  sevseg.begin(P_TRANSISTORS, numDigits, digitPins, segmentPins);
  sevseg.setBrightness (1);

  pinMode(buttonPin, INPUT);
  button.init(buttonPin, LOW);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);

  // Check if the button was pressed while booting
  if (button.isPressedRaw()) {
    if (debug)   Serial.println(F("setup(): button was pressed while booting"));
  }

  // RealTime clock init
  // ==========================
  time.begin(RTC_DS1302, A0, A1, A2); // на базе чипа DS1302, вывод RST, вывод CLK, вывод DAT
  //  запись даты и времени, если год == 0

  if (init_time) {
    //           сек, мин, час, число, месяц, год, день недели
    time.settime( 0,   10,  14,   31,    3,    19,    8);
  }
}

void loop() {


  if (setup_mode)  {
    TimeDisp[0] = setup_hour_start / 10;
    TimeDisp[1] = setup_hour_start % 10;
    TimeDisp[2] = setup_min_start / 10;
    TimeDisp[3] = setup_min_start  % 10;
  }  else  {
    time.gettime("s");
    TimeDisp[0] = time.Hours / 10;
    TimeDisp[1] = time.Hours % 10;
    TimeDisp[2] = time.minutes / 10;
    TimeDisp[3] = time.minutes  % 10;
  }

  // display
  display_number (TimeDisp);

  // Check button state
  button.check();

  // We now in setup mode
  if (setup_mode)  handle_setup_mode ();

  sevseg.refreshDisplay(); // Must run repeatedly

}

/// END ///

void handle_setup_mode () {
  flash_time = millis() - flash_start;

  if (flash_time > flash_delay)    {
    flash_flag = !flash_flag;
    flash_start = millis();
  }

  if (flash_flag)  {
    switch (setup_phase) {
      case 0:
        TimeDisp[0] = 0;
        TimeDisp[1] = 0;
        break;
      case 1:
        TimeDisp[2] = 0;
        TimeDisp[3] = 0;
        break;
    }
  }

  display_number (TimeDisp);
}


void display_number (int8_t num[]) {
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


// The event handler for the button.
void handleEvent(AceButton* /* button */, uint8_t eventType,
                 uint8_t buttonState) {

  // Print out a message for all events.
  /*
    Serial.print(F("handleEvent(): eventType: "));
    Serial.print(eventType);
    Serial.print(F("; buttonState: "));
    Serial.println(buttonState);
  */

  // Control the LED only for the Pressed and Released events.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  switch (eventType) {
    case AceButton::kEventReleased:
      if (debug) Serial.println("Click");

      if (setup_mode)  {

        switch (setup_phase) {
          case 0:
            setup_hour_start++;
            if (setup_hour_start > 23) setup_hour_start = 0;
            break;
          case 1:
            setup_min_start++;
            if (setup_min_start > 59) setup_min_start = 0;
            break;
        }

      }

      break;
    case AceButton::kEventDoubleClicked:
      if (debug) Serial.println("Double Click");
      break;
    case AceButton::kEventLongPressed:
      if (debug) Serial.println("Long press");

      // Enter setup
      if (!setup_mode) {
        setup_mode = true;
        setup_phase = 0;
        flash_start = millis();

        // Remember time value on setup start
        time.gettime("s");
        setup_hour_start = time.Hours;
        setup_min_start = time.minutes;
      }
      else {
        setup_phase++;
        if (setup_phase > max_setup_phase)   {

          // Save to real time clock
          //           сек,     мин,                час,         число, месяц, год, день недели
          time.settime( 0,   setup_min_start,  setup_hour_start,   -1,    -1,    -1,    -1);

          // Exit setup
          setup_mode = false;
        }

      }


      break;
  }
}
