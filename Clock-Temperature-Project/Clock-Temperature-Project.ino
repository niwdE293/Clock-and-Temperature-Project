/*
  Project: Clock and temperature project
  Author: Edwin Ädel
  Date: 2025-11-07
  Description: Displays the time on a led ring and displays tempreture on a screen and servo.
*/

// Include libraries 
#include "RTClib.h"
#include "U8glib.h"
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// Defines the connected led pin on the arduino and led count(number of lights on the led ring)
#define LED_PIN    6
#define LED_COUNT 24

// Constructs objects
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
RTC_DS3231 rtc;
Servo myservo;
Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

int servoPin = 9;   // The pin the servo is connected to on the arduino.
int minTemp = 23;   // Minimum expected temperature.
int maxTemp = 27;   // Maximum expected temperature.
int cx = 64;  // center x
int cy = 37;  // center y
int radius = 20; // Clock radius



char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup(){
  Serial.begin(9600);
  myservo.attach(servoPin);
  u8g.setFont(u8g_font_6x12);

  ring.begin();                       
  ring.setBrightness(10); //Brightness: 0 - 255  

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

// Gets the temperature from the rtc module.
String getTemp(){
  return String(rtc.getTemperature());
}

// Get the current time from the RTC module.
String getTime(){
  DateTime now = rtc.now();
  
  // Getting each time field in individual variables
  // And adding a leading zero when needed;

  String yearStr = String(now.year(), DEC);
  String monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
  String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
  String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC); 
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

  // Complete time string
  String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;
  return formattedTime;
  
}

// Calculates the servos position relative to the temperature.
void servoPosition(){
  float temp = (rtc.getTemperature()) * 1000; // Gets the temperature from the rtc module and multiplies it to get a more percise reading.
  int angle = map(temp , minTemp * 1000, maxTemp * 1000, 0, 180); // Converts all tempreture values to a range between 0-180 degrees on the servo.
  myservo.write(angle); // Writes out the angle of the servo.
}

void analogClock() {
  DateTime now = rtc.now();  // Get current time from RTC  

    // Draw clock circle
    u8g.drawCircle(cx, cy, radius);

    // Hour hand
    float hourAngle = ((now.hour() % 12) + now.minute()/60.0) * 30 * 3.14159 / 180.0;
    int hx = cx + (radius-10) * cos(hourAngle - 3.14159/2);
    int hy = cy + (radius-10) * sin(hourAngle - 3.14159/2);
    u8g.drawLine(cx, cy, hx, hy);

    // Draw hour markers
    for (int i = 0; i < 12; i++) {
      float angle = i * 30 * 3.14159 / 180.0; // radians
      int x1 = cx + (radius - 2) * cos(angle - 3.14159/2);
      int y1 = cy + (radius - 2) * sin(angle - 3.14159/2);
      int x2 = cx + radius * cos(angle - 3.14159/2);
      int y2 = cy + radius * sin(angle - 3.14159/2);
      u8g.drawLine(x1, y1, x2, y2);
    }

    // Minute hand
    float minuteAngle = (now.minute() + now.second()/60.0) * 6 * 3.14159 / 180.0;
    int mx = cx + (radius-5) * cos(minuteAngle - 3.14159/2);
    int my = cy + (radius-5) * sin(minuteAngle - 3.14159/2);
    u8g.drawLine(cx, cy, mx, my);

    // Second hand
    float secondAngle = now.second() * 6 * 3.14159 / 180.0;
    int sx = cx + (radius-2) * cos(secondAngle - 3.14159/2);
    int sy = cy + (radius-2) * sin(secondAngle - 3.14159/2);
    u8g.drawLine(cx, cy, sx, sy); 
}

// Sets the light color depending on the temperatur the tempreture.
void ledRingTemperature(){
  float temp = max(minTemp, rtc.getTemperature()); // Takes the highest value of minimum expected temperature and the current temperature.
  temp = min(maxTemp, temp); // Takes the lowest value of maximum expected temperature and the current temperature.

  int lightValue = map(temp * 1000, minTemp * 1000, maxTemp * 1000, 0, 255);

  // Turns on all lights and sets the color using the temperature.
  for(int i = 0; i < 24; i++){

      ring.setPixelColor(i, 0, 0, lightValue);
      ring.setPixelColor(i, 0, lightValue, 255 - lightValue);
  }
}


// Displays the correct position of thepixel and pixel color on the led ring compared to the current time.
void ledRing(){
    DateTime now = rtc.now(); // Takes the time and date from the RTC module
    int currentSecond = map(now.second(), 0, 60, 0, 24); // Converts Seconds to get a value between 0-24
    ring.setPixelColor( currentSecond, 255, 255, 255); // Diplays a white Pixel on the correct pixel position. (GRB)
    int currentMinute = map(now.minute(), 0, 60, 0, 24); // Converts Minutes to get a value between 0-24
    ring.setPixelColor( currentMinute, 255, 0, 0); // Diplays a green Pixel on the correct pixel position. (GRB)
    int currentHour = map(now.hour() % 12, 0, 12, 0, 24); // Converts the hours to get a value between 0-12.
    ring.setPixelColor( currentHour, 255, 255, 0); // Diplays a yellow Pixel on the correct pixel position. (GRB)
    
    ring.show(); // Turns on the lights.
    ring.clear(); // Clears the lights.
}


void loop () {
  String time = getTime(); // Gets the time from the getTime() function
  String temp = getTemp(); // Gets the temp from the getTemp() function
  
  servoPosition(); // Updates the position of the servo.

  ledRingTemperature(); // Updates the background light color using the current temperature.

  ledRing(); // Updates the position of the lights on the led ring.

  u8g.firstPage(); 
  do {
    analogClock();
    u8g.drawStr(0, 10, ("Temperature: " + String(getTemp())).c_str()); // Draws the the tempreture on the screen.
  } while (u8g.nextPage()); // Displays a new image on the screen
}

