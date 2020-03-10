#include <U8g2lib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#include "SmartPortGps.h"

#include "FrSkySportSensor.h"
#include "FrSkySportSensorGps.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportTelemetry.h"


TinyGPSPlus gps;

static const int RXPin = 13, TXPin = 15;
static const uint32_t GPSBaud = 9600;

#define PRG_PIN 0
#define MODE_ADDR 0

#ifdef TTGO_NODEMCU
  #define OLED_SDA  2
  #define OLED_SCL 14
  #define OLED_RST  4

  U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA , OLED_RST);
#else
//U8g2 Contructor
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);
#endif


u8g2_uint_t offset;     // current offset for the scrolling text
u8g2_uint_t width;      // pixel width of the scrolling text (must be lesser than 128 unless U8G2_16BIT is defined
// const char *text = "JONGSUNG";
// const char *text = "SOONMIN";
const char *text = "YOUNGCHUL";

SoftwareSerial ss(RXPin, TXPin);

FrSkySportSensorGps frGps;                               // Create GPS sensor with default ID
FrSkySportTelemetry telemetry;                         // Create Variometer telemetry object

char recvGpsSignal = 0;
long prevMillis = 0;
int maxSpeed = 0;


uint8_t curMode = MODE_MAX;

void setMode() {
  // curMode = EEPROM.read(MODE_ADDR);
  EEPROM.begin(512);
  EEPROM.get(MODE_ADDR, curMode);
  Serial.print("PREV : ");
  Serial.print(curMode, DEC);
  Serial.print(", ");

  if( curMode == MODE_MAX) curMode = MODE_ALT;
  else if( curMode == MODE_ALT ) curMode = MODE_INFO;
  else if( curMode == MODE_INFO ) curMode = MODE_MAX;
  else curMode = MODE_MAX;

  Serial.print("CUR : ");
  Serial.println(curMode, DEC);
  
  // EEPROM.write(MODE_ADDR, curMode);
  EEPROM.put(MODE_ADDR, curMode);
  EEPROM.commit();
  EEPROM.end();
  
}


void setup(void) {
  u8g2.begin();

//  u8g2.setFont(u8g2_font_logisoso32_tf); // set the target font to calculate the pixel width
//  width = u8g2.getUTF8Width(text);    // calculate the pixel width of the text

  // u8g2.setFontMode(0);    // enable transparent mode, which is faster
  u8g2.setFontMode(1);    // enable transparent mode, which is faster

//  pinMode(0, INPUT);
  // pinMode(PRG_PIN, INPUT_PULLUP);
  

  Serial.begin(57600);
  ss.begin(GPSBaud);

  telemetry.begin(FrSkySportSingleWireSerial::SOFT_SERIAL_PIN_12, &frGps);

  setMode();

}

void loop(void) {
  
  u8g2.clearBuffer();
  u8g2.firstPage();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.setFontPosTop();

  //u8g2_font_luBIS18_tf

  char curTime[10];
  getCurTime(curTime);

  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      u8g2.drawUTF8(103, 0, "r");
      setMaxSpeed();
    }
  }
  
  do {
    drawHeader(curTime);
    if(curMode == MODE_MAX) {
      drawCenterLine();
      drawMaxMode();
    }
    else if(curMode == MODE_INFO) {
      drawCenterLine();
      drawInfoMode();
    }
    else if(curMode == MODE_ALT) {
      drawMoreRightLine();
      drawAltMode();
    }
    drawAntenna();
  } while( u8g2.nextPage() );

  

  sendSmartPortData();
}

void drawCenterLine() {
  u8g2.drawLine(0, 8, 128, 8);
  u8g2.drawLine(64, 8, 64, 48);
}

void drawMoreRightLine() {
  u8g2.drawLine(0, 8, 128, 8);
  u8g2.drawLine(58, 8, 58, 48);
}

void nextMode() {
  if( curMode == MODE_MAX) curMode = MODE_ALT;
  else if( curMode == MODE_ALT ) curMode = MODE_INFO;
  else if( curMode == MODE_INFO ) curMode = MODE_MAX;
  
}

void drawAltMode() {
  char buff[8];
  
  u8g2.drawStr(51, 9, "k");
  u8g2.drawStr(51, 17, "/");
  u8g2.drawStr(51, 25, "h");

//  u8g2.drawStr(120, 9, "a");
//  u8g2.drawStr(120, 17, "l");
//  u8g2.drawStr(120, 25, "t");

  getStrValue(buff, gps.speed.kmph());

  u8g2.setFont(u8g2_font_logisoso22_tf);
  u8g2.setFontPosTop();
  u8g2.drawStr(1, 10, buff);

  getStrValue(buff, gps.altitude.meters(), 4, 1);
  u8g2.drawStr(60, 10, buff);
}


void drawMaxMode() {
  char buff[8];
  
  u8g2.drawStr(56, 9, "k");
  u8g2.drawStr(56, 17, "/");
  u8g2.drawStr(56, 25, "h");

  u8g2.drawStr(120, 9, "k");
  u8g2.drawStr(120, 17, "/");
  u8g2.drawStr(120, 25, "h");

  getStrValue(buff, gps.speed.kmph());

  u8g2.setFont(u8g2_font_logisoso22_tf);
  u8g2.setFontPosTop();
  u8g2.drawStr(1, 10, buff);

  getStrValue(buff, maxSpeed);
  u8g2.drawStr(66, 10, buff);
}

void drawInfoMode() {
  char buff[8];

  u8g2.drawStr(0, 10, "Cur: ");
  getStrValue(buff, gps.speed.kmph());
  u8g2.drawStr(22, 10, buff);
  u8g2.drawStr(44, 10, "kmph");

  getStrValue(buff, gps.location.lat(), 8, 4);
  u8g2.drawStr(0, 17, "LAT: ");
  u8g2.drawStr(22, 17, buff);

  getStrValue(buff, gps.location.lng(), 8, 4);
  u8g2.drawStr(0, 24, "LNG: ");
  u8g2.drawStr(22, 24, buff);

  getStrValue(buff, maxSpeed);
  u8g2.drawStr(66, 10, "MAX: ");
  u8g2.drawStr(88, 10, buff);
  u8g2.drawStr(109, 10, "kmph");

  getStrValue(buff, gps.altitude.meters(), 6, 1);
  u8g2.drawStr(66, 17, "ALT: ");
  u8g2.drawStr(80, 17, buff);
}

void drawHeader(char* curTime) {
  char buff[8];
  u8g2.drawStr(0, 0, curTime);
  u8g2.drawStr(48, 0, text);
  
  getStrValue(buff, gps.satellites.value());
  u8g2.drawUTF8(109, 0, buff);
}

void drawAntenna() {
  if( gps.satellites.value() > 4 ) {
    u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
    u8g2.setFontPosTop();
    u8g2.drawUTF8(120, 0, "\u00f8");
  }
}

int getLocalHour(int org, int gmt) {
  int ret = org + gmt;
  if(ret >= 24) ret = ret - 24;
  return ret;
    
}

void getStrValue(char* buff, int value) {
  sprintf(buff, "%d", value);
}

void getStrValue(char* buff, double value, int width, int len) {
  dtostrf(value, width, len, buff);
}

void getCurTime(char* curTime) {

  sprintf(curTime, "%02d:%02d:%02d", getLocalHour(gps.time.hour(), 9), gps.time.minute(), gps.time.second());
}

void setMaxSpeed() {

  if( gps.speed.kmph() > ( maxSpeed + 100) )
    return;
    
  if( gps.speed.kmph() > maxSpeed)
    maxSpeed = gps.speed.kmph();
}

void sendSmartPortData() {
    frGps.setData(gps.location.lat(), gps.location.lng(),   // Latitude and longitude in degrees decimal (positive for N/E, negative for S/W)
            gps.altitude.meters(),                        // Altitude in m (can be nevative)
//            gps.speed.mps(),                              // Speed in m/s
                        17,                              // Speed in m/s
            gps.course.deg(),                             // Course over ground in degrees
            gps.date.year(), gps.date.month(), gps.date.day(),             // Date (year - 2000, month, day)
            gps.time.hour(), gps.time.minute(), gps.time.second());        // Time (hour, minute, second) - will be affected by timezone setings in your radio

    telemetry.send();
}
