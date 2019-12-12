#include <U8g2lib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#include "FrSkySportSensor.h"
#include "FrSkySportSensorGps.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportTelemetry.h"

TinyGPSPlus gps;

static const int RXPin = 13, TXPin = 15;
static const uint32_t GPSBaud = 9600;

//U8g2 Contructor
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

u8g2_uint_t offset;     // current offset for the scrolling text
u8g2_uint_t width;      // pixel width of the scrolling text (must be lesser than 128 unless U8G2_16BIT is defined
const char *text = "ROB01 "; // scroll this text from right to left

SoftwareSerial ss(RXPin, TXPin);

FrSkySportSensorGps frGps;                               // Create GPS sensor with default ID
FrSkySportTelemetry telemetry;                         // Create Variometer telemetry object


void setup(void) {
  u8g2.begin();

  u8g2.setFont(u8g2_font_logisoso32_tf); // set the target font to calculate the pixel width
  width = u8g2.getUTF8Width(text);    // calculate the pixel width of the text

  u8g2.setFontMode(0);    // enable transparent mode, which is faster

  Serial.begin(57600);
  ss.begin(GPSBaud);

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();

  telemetry.begin(FrSkySportSingleWireSerial::SOFT_SERIAL_PIN_12, &frGps, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

}


void loop(void) {
  
  u8g2.firstPage();
  u8g2.setFont(u8g2_font_micro_tr);

  while (ss.available() > 0) {
    u8g2.setCursor(0, 21);
    if (gps.encode(ss.read())) {
      u8g2.print("gps ok...");
      u8g2.setCursor(70, 21);
      u8g2.print(gps.speed.kmph());
      displayInfo();

    // Set GPS data
    frGps.setData(gps.location.lat(), gps.location.lng(),   // Latitude and longitude in degrees decimal (positive for N/E, negative for S/W)
              gps.altitude.meters(),                        // Altitude in m (can be nevative)
              gps.speed.mps(),                              // Speed in m/s
              gps.course.deg(),                             // Course over ground in degrees
              gps.date.year(), gps.date.month(), gps.date.day(),             // Date (year - 2000, month, day)
              gps.time.hour(), gps.time.minute(), gps.time.second());        // Time (hour, minute, second) - will be affected by timezone setings in your radio
      
    } else {
      u8g2.print("gps checking...");
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  } else {
    do{
      u8g2.setCursor(0, 27);
      u8g2.print("hahahaha....");
    } while( u8g2.nextPage() );
    telemetry.send();
  }
}


void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
