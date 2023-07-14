// Include Wire Library for I2C
#include <Wire.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"

// Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Reset pin not used but needed for library
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Wifi Declearations
const char* ssid       = "superman";
const char* password   = "aa00bb11cc";

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char* time_zone = "EST5EDT,M3.2.0,M11.1.0";  // TimeZone rule for Toronto including daylight adjustment rules (optional)

// Buttons
const int button1 = 15;
const int button2 = 4;

int buttonstate1 = 0;
int buttonstate2 = 0;


// Global alarm setting
int globAlarmMin = -1;
int globAlarmHour = -1;

void buttonReads() {
  buttonstate1 = digitalRead(button1);
  buttonstate2 = digitalRead(button2);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


void setup() {
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
//  pinMode(speakerPin, OUTPUT);


  // Start Wire library for I2C
  Wire.begin();
  // initialize OLED with I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  Serial.begin(115200);
  // set notification call-back function
  sntp_set_time_sync_notification_cb( timeavailable );
  sntp_servermode_dhcp(1);    // (optional)
  configTzTime(time_zone, ntpServer1, ntpServer2);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("CONNECTED");

  Serial.println("Reaches");
  digitalWrite(button1, LOW);
  digitalWrite(button2, LOW);
  beep();

}
// ---------------------------------------------------------------------------------------------
void printLocalTime()
{
  struct tm timeinfo; // Empty Structure
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0, 0);
  display.println(&timeinfo, "%A");
  display.println(&timeinfo, "%B %d %Y");
  display.println(&timeinfo, "%H:%M:%S");

  display.setCursor(75, 0);
  if (globAlarmHour == -1 && globAlarmMin == -1) {
    display.print("A:NONE");
  }
  else {
    if (globAlarmMin == 0) {
      display.print("A:" + String(globAlarmHour) + ":00");
    }
    else {
      display.print("A:" + String(globAlarmHour) + ":" + String(globAlarmMin));
    }
  }

  // Gets current hour and minute
  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;

  // If a certain button is clicked, the alarm menu will show up
  buttonReads();
  // Testing if Buttons work
  //    if (buttonstate2 == HIGH) {
  //    Serial.println("Button 2 pressed!");
  //    delay(500);  // Add a small delay for debounce
  //  }
  //  // Check if the button is pressed (active HIGH)
  //  if (buttonstate1 == HIGH) {
  //    Serial.println("Button 1 pressed!");
  //    delay(500);  // Add a small delay for debounce
  //  }
  if (buttonstate1 == HIGH) setAlarm();
  alarm(minute, hour);

}
// ---------------------------------------------------------------------------------------------

void oledDisplay(String msg, int fontSize = 1) {
  // Clear the display
  display.display();
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(fontSize);
  //Set the cursor coordinates
  display.setCursor(0, 0);

  display.println(msg);

}

void setAlarm() {
  Serial.println("Set alarm function");
  static bool clicked = false;
  int lmin = 0;
  int lhour = 0;

  // Sets Hour First
  beep();
  delay(1000);
  Serial.print("Hour first code");
  while (1) {
    oledDisplay("|Hour: " + String(lhour) + "|\nMinute: " + String(lmin) + "\nClear Alarm");
    //    Serial.println("buttonstate1 was low");
    buttonReads();
    if (buttonstate2 == HIGH && clicked == false) {
      lhour += 1;
      if (lhour == 24) lhour = 0;
      clicked = true;
    }
    else if (buttonstate1 == HIGH) break;
    if (buttonstate2 == LOW) clicked = false;
  }
  beep();
  delay(1000);

  // Sets Minute Next
  while (1) {
    Serial.println("I reached minutes");
    oledDisplay("Hour: " + String(lhour) + "\n|Minute: " + String(lmin) + "|" + "\nClear Alarm");
    //      Serial.println("buttonstate1 was low");
    buttonReads();
    if (buttonstate2 == HIGH && clicked == false) {
      lmin += 5;
      if (lmin == 60) lmin = 0;
      clicked = true;
      //        oledDisplay("Hour: " + String(lhour) + "\n|Minute: " + String(lmin) + "|");
    }
    else if (buttonstate1 == HIGH) break;
    if (buttonstate2 == LOW) clicked = false;
  }
  beep();
  delay(1000);

  while (1) {
    oledDisplay("Hour: " + String(lhour) + "\nMinute: " + String(lmin) + "\n|Clear Alarm|");
    buttonReads();
    if (buttonstate2 == HIGH && clicked == false) {
      lhour = -1;
      lmin = -1;
      break;
      if (lhour == 24) lhour = 0;
      clicked = true;
    }
    else if (buttonstate1 == HIGH) break;
    if (buttonstate2 == LOW) clicked = false;
  }
  beep();
  delay(1000);

  globAlarmMin = lmin;
  globAlarmHour = lhour;
  Serial.print(globAlarmMin);
  Serial.print(globAlarmHour);
}

void alarm(int cmin, int chour) {
  Serial.println("Alarm function");
  if (cmin == globAlarmMin && chour == globAlarmHour) {
    while(1){
      if (buttonstate2 == HIGH || buttonstate1 == HIGH) {
        globAlarmMin = -1;
        globAlarmMin = -1;
        Serial.print("BREAK FUNCTION");
        break;
      }
      beep();
      delay(200);
    }
  }

}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void beep() {
  for (int i = 0; i < 2; i++) {
    tone(14, 800 + i * 100, 100);
  }
}


void loop() {
  //  displayTime();
  printLocalTime();
  display.display();
}
