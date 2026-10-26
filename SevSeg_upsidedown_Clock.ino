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

// Auto save and exit setup if the button was not pressed for this time
const unsigned long setup_timeout = 10000;
unsigned long setup_last_press;

// Button in setup: click - next value, hold 1-2 s and release - next field
// or save, hold longer - fast scroll while held
const unsigned int long_press_delay = 1000;
const unsigned int scroll_delay = 2000;
const unsigned int scroll_interval = 150;
boolean press_ignore;   // This press entered setup, ignore it till release
boolean press_long;
boolean press_scrolled;

// Reading DS1302 is slow (about 12 ms), so read it only this often
const unsigned long rtc_read_period = 250;
unsigned long rtc_last_read;
boolean rtc_read_now = true;
byte local_hour;
byte local_min;

// What is on the display now, to update it only on changes
int shown_num = -1;
byte shown_dots;

// Daylight saving time (Kyiv, EU rules)
// RTC always keeps standard (winter) time, summer time is shown as +1 hour.
// Switch at 01:00 UTC = 03:00 standard time, last Sunday of March and October
boolean auto_dst = true;
boolean setup_dst;


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
  sevseg.beginTimerRefresh(); // Display is refreshed from the timer interrupt

  pinMode(buttonPin, INPUT);
  button.init(buttonPin, LOW);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  // Clicks are detected on release by the press length,
  // so kFeatureClick and kFeatureDoubleClick are not used
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig->setLongPressDelay(long_press_delay);
  buttonConfig->setRepeatPressDelay(scroll_delay);
  buttonConfig->setRepeatPressInterval(scroll_interval);

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

  // Check button state
  button.check();

  // We now in setup mode
  if (setup_mode)  handle_setup_mode ();

  byte dots = 0;

  if (setup_mode)  {
    TimeDisp[0] = setup_hour_start / 10;
    TimeDisp[1] = setup_hour_start % 10;
    TimeDisp[2] = setup_min_start / 10;
    TimeDisp[3] = setup_min_start  % 10;
    dots = setup_dots ();
  }  else  {
    if (rtc_read_now || millis() - rtc_last_read >= rtc_read_period)  {
      time.gettime("s");
      local_hour = time.Hours;
      if (is_dst ())  local_hour = (local_hour + 1) % 24;
      local_min = time.minutes;
      rtc_last_read = millis();
      rtc_read_now = false;
    }
    TimeDisp[0] = local_hour / 10;
    TimeDisp[1] = local_hour % 10;
    TimeDisp[2] = local_min / 10;
    TimeDisp[3] = local_min  % 10;
  }

  // display
  display_number (TimeDisp, dots);
}

/// END ///

void handle_setup_mode () {
  if (millis() - setup_last_press > setup_timeout)  {
    save_setup_time ();
    return;
  }

  flash_time = millis() - flash_start;

  if (flash_time > flash_delay)    {
    flash_flag = !flash_flag;
    flash_start = millis();
  }
}

// Flash the dot near the first of the digits being set
// Display is upside-down: digits 0-1 are minutes, 2-3 are hours
byte setup_dots () {
  if (!flash_flag)  return 0;

  switch (setup_phase) {
    case 0:
      return B1000;
    case 1:
      return B0010;
  }
  return 0;
}


void display_number (int8_t num[], byte dots) {
  // Display is upside-down: digits go in reverse order
  int flip_num = num[3] * 1000 + num[2] * 100 + num[1] * 10 + num[0];

  // Display is refreshed from the interrupt: change it only when needed
  if (flip_num == shown_num && dots == shown_dots)  return;
  shown_num = flip_num;
  shown_dots = dots;

  sevseg.setNumber(flip_num, 4);
  sevseg.setDots(dots);
}

void setup_next_value () {
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

  switch (eventType) {
    case AceButton::kEventPressed:
      press_ignore = false;
      press_long = false;
      press_scrolled = false;
      break;

    case AceButton::kEventLongPressed:
      if (debug) Serial.println("Long press");

      // Enter setup
      if (!setup_mode) {
        setup_mode = true;
        setup_phase = 0;
        flash_start = millis();
        press_ignore = true;

        // Remember time value on setup start
        time.gettime("s");
        setup_dst = is_dst ();
        setup_hour_start = time.Hours;
        if (setup_dst)  setup_hour_start = (setup_hour_start + 1) % 24;
        setup_min_start = time.minutes;
      }
      else {
        // Next field on release, if it does not turn into a scroll
        press_long = true;
      }
      break;

    case AceButton::kEventRepeatPressed:
      // Fast scroll while held
      if (setup_mode && !press_ignore)  {
        setup_next_value ();
        press_scrolled = true;
      }
      break;

    case AceButton::kEventReleased:
      if (!setup_mode || press_ignore || press_scrolled)  break;

      if (press_long)  {
        if (debug) Serial.println("Next field");
        setup_phase++;
        if (setup_phase > max_setup_phase)   save_setup_time ();
      }
      else  {
        if (debug) Serial.println("Click");
        setup_next_value ();
      }
      break;
  }

  // Any button event restarts the setup timeout
  if (setup_mode)  setup_last_press = millis();
}

void save_setup_time () {
  // Entered time is local, RTC keeps standard time
  int std_hour = setup_hour_start;
  if (setup_dst)  std_hour--;

  if (std_hour < 0)  {
    // Summer time 00:xx is 23:xx of the previous day in standard time
    std_hour = 23;
    time.gettime("s");
    int y = 2000 + time.year;
    byte m = time.month;
    byte d = time.day - 1;
    if (d == 0)  {
      m--;
      d = days_in_month (y, m);
    }
    // Summer time never crosses a year boundary
    //           сек,     мин,           час,  число, месяц, год,       день недели
    time.settime( 0,   setup_min_start,  std_hour,  d,  m,  y - 2000,  day_of_week (y, m, d));
  }
  else  {
    // Save to real time clock
    //           сек,     мин,                час,         число, месяц, год, день недели
    time.settime( 0,   setup_min_start,  std_hour,   -1,    -1,    -1,    -1);
  }

  // Exit setup
  setup_mode = false;
  rtc_read_now = true;
}

// Is summer time now? Uses date and time read by the last time.gettime()
boolean is_dst () {
  if (!auto_dst)  return false;

  byte m = time.month;
  if (m < 3 || m > 10)  return false;
  if (m > 3 && m < 10)  return true;

  // March or October: summer time starts / ends at 03:00 standard time
  // of the last Sunday
  int y = 2000 + time.year;
  byte last_sunday = 31 - day_of_week (y, m, 31);
  boolean after_switch = time.day > last_sunday ||
                         (time.day == last_sunday && time.Hours >= 3);

  return (m == 3) ? after_switch : !after_switch;
}

// 0 - Sunday ... 6 - Saturday (Sakamoto's method)
byte day_of_week (int y, byte m, byte d) {
  static const byte t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3)  y--;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

byte days_in_month (int y, byte m) {
  static const byte days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (m == 2 && (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)))  return 29;
  return days[m - 1];
}
