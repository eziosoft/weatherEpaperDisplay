#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <StreamString.h>
#include "ESP8266WiFi.h"
#define PrintString StreamString

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#include "e.c"
#include "r.c"

ADC_MODE(ADC_VCC);

//milliseconds to sleep
const int intervall = 60000;

struct bitmap_pair
{
  const unsigned char *black;
  const unsigned char *red;
};

//GxEPD2_3C<GxEPD2_260c, GxEPD2_260c::HEIGHT> display(GxEPD2_260c(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
GxEPD2_BW<GxEPD2_260, GxEPD2_260::HEIGHT> display(GxEPD2_260(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
PrintString vddString;
PrintString rssiString;
float vdd, rssi;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  delay(100);
  display.init(115200);
  display.setFullWindow();
 

  WiFi.forceSleepWake();
  delay(1);
  setupWifi();

  vdd = ESP.getVcc() / 1000.0;
  vddString.print(vdd, 2);
  Serial.println(vddString);

  rssi = dBmtoPercentage(WiFi.RSSI());
  rssiString.print(rssi, 0);
  Serial.println(rssi);
}

void loop()
{
  loopWifi();
}

void update(char *text)
{
   printText1(text);
  display.hibernate();
  sendTelemetry();

  delay(100);
  deepSleep();
}


void deepSleep()
{
  //https://github.com/esp8266/Arduino/issues/644
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  ESP.deepSleep(intervall * 1000/*, WAKE_RF_DISABLED*/);
}

void printText1(char *text)
{
  //Serial.println("helloWorld");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    printStatic();

    display.setCursor(0, y);
    display.print(text);
  } while (display.nextPage());
  //Serial.println("helloWorld done");
}

void printStatic()
{
  display.setCursor(250, 11);
  display.print(vddString);

  display.setCursor(215, 11);
  display.print(rssiString);
}

void printText(char *text)
{
  //Serial.println("helloWorld");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    printStatic();

    display.setCursor(x, y);
    display.print(text);
  } while (display.nextPage());
  //Serial.println("helloWorld done");
}

void drawBitmap()
{

  bitmap_pair bitmap_pairs[] =
      {
          {gImage_e, gImage_r}};

  for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
  {
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      //      display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
    } while (display.nextPage());
    delay(2000);
  }
}



const int RSSI_MAX = -50;  // define maximum strength of signal in dBm
const int RSSI_MIN = -100; // define minimum strength of signal in dBm
int dBmtoPercentage(int dBm)
{
  int quality;
  if (dBm <= RSSI_MIN)
  {
    quality = 0;
  }
  else if (dBm >= RSSI_MAX)
  {
    quality = 100;
  }
  else
  {
    quality = 2 * (dBm + 100);
  }

  return quality;
} //dBmtoPercentage
