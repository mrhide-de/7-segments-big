/*
Code for 7 segment wall watch
with Neopixel
*/
#include <Adafruit_NeoPixel.h>

#define DEBUG    0
#define DEBUG_HOUR    22
#define DEBUG_MIN    59
#define LED_PIN  D6
#define LED_COUNT 50

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#if DEBUG == 1
#define sp(x) Serial.print(x)
#define spl(x) Serial.println(x)
#else
#define sp(x)
#define spl(x)
#endif

const int analogInPin = A0;
int sensorValue = 0;

uint8_t lightening = 5;
uint8_t cur_cols[3] = { 0 * lightening, 5 * lightening, 7 * lightening };
uint8_t dot_cols[3] = { 0 * lightening, 5 * lightening, 7 * lightening };

uint8_t all_rgbs[][3] = {
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}
};

int brightness = 180; // (max = 255)

// all LEDs on/off
byte all_leds_onoff[LED_COUNT] = {
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0
  };

/*
LED layout
  4-5
3   6-7
  8-9
2   10-11
  1-0
*/

const byte digits[][12] = {
  {1,1,1,1,1,1,1,1,0,0,1,1}, // 0
  {0,0,0,0,0,0,1,1,0,0,1,1}, // 1
  {1,1,1,0,1,1,1,1,1,1,0,0}, // 2
  {1,1,0,0,1,1,1,1,1,1,1,1}, // 3
  {0,0,0,1,0,0,1,1,1,1,1,1}, // 4
  {1,1,0,1,1,1,0,0,1,1,1,1}, // 5
  {1,1,1,1,1,1,0,0,1,1,1,1}, // 6
  {0,0,0,0,1,1,1,1,0,0,1,1}, // 7
  {1,1,1,1,1,1,1,1,1,1,1,1}, // 8
  {1,1,0,1,1,1,1,1,1,1,1,1}  // 9
};

int the_hours = 0;
int the_mins = 0;
int hour_1 = 8;
int hour_2 = 8;
int min_1 = 8;
int min_2 = 8;
int week_day ;
int the_countdown = 0;
byte is_fading = false;
byte dot_is_fading = false;
byte starting_up = false;
int go_through_colors = 0;

// WIFI
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <TimeLib.h>


const long utcOffsetInSeconds = 3600; // DE, FR, ES, PL, IT, NW, SE, DK, ...

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

unsigned long startMS;
unsigned long currentMS;


void setup() {
  #if DEBUG == 1
  Serial.begin(115200);
  spl(); spl(); spl("Los geht's!"); spl();
  #endif

  strip.begin();
  strip.setBrightness(brightness);

  timeClient.begin();
  testSegments();

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;
  wm.setDebugOutput(DEBUG);
  // wm.resetSettings(); // CAREFULL!!
  wm.setConfigPortalTimeout(180);

  if (!wm.autoConnect("7_segments", "sevensegments")) {
    spl("no configuration");
    hour_1 = 1;
    hour_2 = 2;
    min_1 = 3;
    min_2 = 4;
    dot_cols[0] = 13 * lightening;
    dot_cols[1] = 0;
    dot_cols[2] = 0;
  } else {
    spl("connected to wifi");
    timeClient.update();
    // setSegmentsFromTime();
    go_through_colors = 1;
  }
  startMS = millis() - 2000;
}


void loop() {

  currentMS = millis();

  // sync to zero
  if (starting_up && timeClient.getSeconds() == 0) {
    startMS = millis() - 60000;
    starting_up = false;
  }
  
  if (go_through_colors > 0 && go_through_colors <= 8) {
    if (currentMS - startMS >= 2000) {
      startMS = millis();
      setSegmentsFromTime();
      go_through_colors++;
    }
  } else if (go_through_colors > 8) {
    go_through_colors = 0;
    starting_up = true;
  } else if (starting_up == false) {
    // 60s
    if (the_countdown == 0 && currentMS - startMS >= 60000) {
      startMS = millis();
      setSegmentsFromTime();
    }
    // countdown
    if (the_countdown > 0 && currentMS - startMS >= 1000) {
      startMS = millis();
      setSegmentsFromTime();
    }
  }

  // keep fading
  if (is_fading == true) {
    darthFader();
  }

  delay(7);
}



void setSegmentsFromTime() {

  // get light level
  sensorValue = analogRead(analogInPin);
  if (sensorValue < 150) {
    lightening = map(sensorValue, 0, 150, 1, 5);
  } else {
    lightening = map(sensorValue, 150, 1055, 5, 10);
  }

  if (go_through_colors > 0) {
    if (go_through_colors == 7) {
      week_day = 0;
    } else if (go_through_colors > 7) {
      week_day = timeClient.getDay();
    } else {
      week_day = go_through_colors;
    }
  } else {
    week_day = timeClient.getDay();
  }

    // get the time
  the_hours = timeClient.getHours();
  the_mins = timeClient.getMinutes();
 
  
  if (is_daylight_saving(the_hours)) {
    if (the_hours < 23) {
      the_hours++;
    } else {
      the_hours = 0;
      if (week_day < 6) {
        week_day++;
      } else {
        week_day = 0;
      }
    }
  }
  
  sp(the_hours); sp(":"); spl(the_mins);
  sp("weekday: "); spl(week_day);

  if (lightening == 1) { // night mode
    cur_cols[0] = 4 * lightening;
    cur_cols[1] = 0 * lightening;
    cur_cols[2] = 0 * lightening;
  } else if (week_day == 0 ) { // SUN - violet
    cur_cols[0] = 11 * lightening;
    cur_cols[1] = 1 * lightening;
    cur_cols[2] = 6 * lightening;
  } else if (week_day == 1 ) { // MON - red
    cur_cols[0] = 16 * lightening;
    cur_cols[1] = 0 * lightening;
    cur_cols[2] = 0 * lightening;
  } else if (week_day == 2 ) { // TUE - orange
    cur_cols[0] = 16 * lightening;
    cur_cols[1] = 4 * lightening;
    cur_cols[2] = 1 * lightening;
  } else if (week_day == 3 ) { // WED - yellow
    cur_cols[0] = 12 * lightening;
    cur_cols[1] = 7 * lightening;
    cur_cols[2] = 1 * lightening;
  } else if (week_day == 4 ) { // THU - green
    cur_cols[0] = 0 * lightening;
    cur_cols[1] = 12 * lightening;
    cur_cols[2] = 1 * lightening;
  } else if (week_day == 5 ) { // FRI - blue
    cur_cols[0] = 0 * lightening;
    cur_cols[1] = 3 * lightening;
    cur_cols[2] = 18 * lightening;
  } else if (week_day == 6 ) { // SAT - indigo
    cur_cols[0] = 4 * lightening;
    cur_cols[1] = 0 * lightening;
    cur_cols[2] = 15 * lightening;
  } else { // error - cyan
    cur_cols[0] = 0 * lightening;
    cur_cols[1] = 5 * lightening;
    cur_cols[2] = 8 * lightening;
  }
  sp("Sensor: "); spl(sensorValue);
  sp("Level: "); spl(lightening);

  // countdown to the new day
  if (the_hours == 23 && the_mins == 59) { // manage countdown
    if (the_countdown == 0) {
      the_countdown = 60;
      spl("starting countdown");
    } else {
      the_countdown--;
      sp(the_countdown); sp(" ");;
    }
  } else {
    the_countdown = 0;
  }

  // split digits
  if (the_countdown == 0) {
    hour_1 = the_hours / 10;
    hour_2 = the_hours - (hour_1 * 10);
    min_1 = the_mins / 10;
    min_2 =  the_mins - (min_1 * 10);
  } else {
    hour_1 = 0;
    hour_2 = 0;
    min_1 = the_countdown / 10;
    min_2 =  the_countdown - (min_1 * 10);
  }

  // resync time
  if (the_hours == 22 && the_mins == 58) {
    timeClient.update();
    the_hours = timeClient.getHours();
    the_mins = timeClient.getMinutes();
    starting_up = true;
  }

  // set segments
  for (size_t l = 0; l < LED_COUNT; l++) {
    all_leds_onoff[l] = 0;
  }
  if (hour_1 != 0) {
    for (size_t i = 0; i < 12; i++) { // digit 1
      if (digits[hour_1][i] == 1) {
        all_leds_onoff[i] = 1;
      }
    }
  }
  if (hour_2 != 0 || hour_2 == 0 && hour_1 != 0) {
    for (size_t n = 12; n < 24; n++) { // digit 2
      if (digits[hour_2][n-12] == 1) {
        all_leds_onoff[n] = 1;
      }
    }
    all_leds_onoff[24] = 1; // doppelpunkte
    all_leds_onoff[25] = 1;
  } else {
    all_leds_onoff[24] = 0;
    all_leds_onoff[25] = 0;
  }
  if (hour_1 == 0 && hour_2 == 0 && min_1 == 0) {
    // nix
  } else { 
    for (size_t m = 26; m < 38; m++) { // digit 3
      if (digits[min_1][m-26] == 1) {
        all_leds_onoff[m] = 1;
      }
    }
  }
  for (size_t p = 38; p < 50; p++) { // digit 4
    if (digits[min_2][p-38] == 1) {
      all_leds_onoff[p] = 1;
    } else {
      all_leds_onoff[p] = 0;
    }
  }

  // start fading
  darthFader();

}


void darthFader() {
  is_fading = false;
  for (size_t i = 0; i < LED_COUNT; i++) {
    if (all_leds_onoff[i] == 1) { // fade in
      if (all_rgbs[i][0] != cur_cols[0] || all_rgbs[i][1] != cur_cols[1] || all_rgbs[i][2] != cur_cols[2]) {
        is_fading = true;
        fadeStepper(i, false);
      }
    } else { // fade out
      if (all_rgbs[i][0] > 0 || all_rgbs[i][1] > 0 || all_rgbs[i][2] > 0) {
        is_fading = true;
        fadeStepper(i, true);
      }
    }
    strip.setPixelColor(i, strip.Color(all_rgbs[i][0], all_rgbs[i][1], all_rgbs[i][2]));
  }
  strip.show();
}

void fadeStepper(uint8_t px_c, byte toBlack) {
  if (toBlack) {
    for (size_t i = 0; i < 3; i++) {
      if (all_rgbs[px_c][i] > 0) { all_rgbs[px_c][i]--; } 
    }
  } else {
    for (size_t i = 0; i < 3; i++) {
      if (all_rgbs[px_c][i] < cur_cols[i]) { all_rgbs[px_c][i]++; } 
      else if (all_rgbs[px_c][i] > cur_cols[i]) { all_rgbs[px_c][i]--; }       
    }
  }
}
  

void testSegments() {
  strip.clear();
  strip.fill(strip.Color(cur_cols[0], cur_cols[1], cur_cols[2]), 0, LED_COUNT);
  strip.show();
}


byte is_daylight_saving(int hour) {
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int day = ptm->tm_mday;
  int month = ptm->tm_mon+1;
  int year = ptm->tm_year+1900;

  if (month > 3 && month < 10) {
    return true;
  } else if (month < 3 || month > 10) {
    return false;
  } else {
    tmElements_t other_t;
    time_t o_t;
    other_t.Hour = 12;
    other_t.Minute = 0;
    other_t.Second = 0;
    other_t.Year = year - 1970;
    other_t.Month = month; // 3 or 10
    other_t.Day = 31;
    o_t = makeTime(other_t);
    int the_wd = weekday(o_t);
    int last_sun = 31 - (the_wd - 1);
    if (last_sun < day) { // after last sun
      if (month == 3) {
        return true;
      } else {
        return false;
      }
    } else if (last_sun == day) { // changes today
      if (hour >= 2) {
        if (month == 3) {
          return true;
        } else {
          return false;
        }
      } else {
        if (month == 3) {
          return false;
        } else {
          return true;
        }
      }
    } else { // before last sun
      if (month == 3) {
        return false;
      } else {
        return true;
      }
    }
  }
}
