#include <GxEPD2_BW.h> //https://github.com/eziosoft/GxEPD2
#include <GxEPD2_3C.h>

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include <StreamString.h>
#define PrintString StreamString

#include <GxEPD2_3C.h>
#include <ArduinoJson.h>

#include "ESP8266WiFi.h"
#include "icons.h" //weather icons from OpenWeather

#define FW_VERSION 2

// GxEPD2_BW<GxEPD2_260, GxEPD2_260::HEIGHT> display(GxEPD2_260(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); //BW - faster refresh
GxEPD2_3C<GxEPD2_260c, GxEPD2_260c::HEIGHT> display(GxEPD2_260c(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); //BRW - slow refresh
static const uint16_t WIDTH = 152;
static const uint16_t HEIGHT = 296;

ADC_MODE(ADC_VCC); //enable VCC measurement
#define VDD_offset -0.17
#define ENABLE_GxEPD2_GFX 1 //enable Adafruit GFX library

//milliseconds to sleep
static const uint16_t minute = 60;
long interval = minute; //deep sleep duration

PrintString vddString;
PrintString rssiString;
float vdd, rssi;

void setup()
{
  Serial.begin(115200);
  display.init(115200);
  display.setFullWindow();

  WiFi.forceSleepWake();
  delay(1);
  setupWifi();

  vdd = (ESP.getVcc() / 1000.0) + VDD_offset;
  vddString.print(vdd, 2);

  rssi = dBmtoPercentage(WiFi.RSSI());
  rssiString.print(rssi, 0);
}

void loop()
{
  loopWifi();
  if (millis() > 20000) //if this runs more than 20sek something was wrong. usually takes 8sek
  {
    printTextCenter("timeout");
    delay(100);

    interval = minute;
    deepSleep();
  }
}

void json(char *json)
{
  sendTelemetry();

  const size_t capacity = JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(20) + 24 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 550;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    printTextCenter("json error");
    interval = minute;
    deepSleep();
    return;
  }

  int version = doc["v"];
  if (version > FW_VERSION)
  {
    updateFirmware();
  }

  delay(100);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.println("\n\nWIFI OFF");

  interval = doc["NR"]; // in seconds

  int icon = doc["i"];
  int iconColor = GxEPD_BLACK;

  if (doc["ic"] == 1)
  {
    iconColor = GxEPD_RED;
  }

  JsonArray data = doc["d"];
  JsonArray lines = doc["l"];

  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    printStatic();

    switch (icon)
    {
    case 1:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i01n, 100, 100, iconColor);
      break;
    case 2:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i02n, 100, 100, iconColor);
      break;
    case 3:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i03n, 100, 100, iconColor);
      break;
    case 4:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i04n, 100, 100, iconColor);
      break;
    case 9:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i09n, 100, 100, iconColor);
      break;
    case 10:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i10n, 100, 100, iconColor);
      break;
    case 11:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i11n, 100, 100, iconColor);
      break;
    case 13:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i13n, 100, 100, iconColor);
      break;
    case 50:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i50n, 100, 100, iconColor);
      break;
    default:
      break;
    }

    for (int i = 0; i < 20; i++)
    {
      JsonObject datai = data[i];
      int data_x = datai["x"];            // 10
      int data_y = datai["y"];            // 10
      int font = datai["f"];              //font number
      int color = datai["c"];             // 0 or 1 - black or red
      const char *data_text = datai["t"]; // "text"

      switch (font)
      {
      case 0:
        display.setFont(&FreeMonoBold9pt7b);
        break;

      case 1:
        display.setFont(&FreeMonoBold24pt7b);
        break;

      case 2:
        display.setFont(&FreeSans9pt7b);
        break;
      case 3:
        display.setFont(&FreeSans12pt7b);
        break;
      case 4:
        display.setFont(&FreeSans18pt7b);
        break;
      case 5:
        display.setFont(&FreeSans24pt7b);
        break;

      default:
        display.setFont(&FreeMonoBold9pt7b);
        break;
      }

      if (color == 1)
        display.setTextColor(GxEPD_RED);
      else
        display.setTextColor(GxEPD_BLACK);

      display.setCursor(data_x, data_y);
      display.print(data_text);
    }

    for (int i = 0; i < 10; i++)
    {
      JsonObject line = lines[i];
      int x1 = line["x1"];   // 10
      int x2 = line["x2"];   // 10
      int y1 = line["y1"];   // 10
      int y2 = line["y2"];   // 10
      int color = line["c"]; // 0 or 1 - black or red

      if (color == 1)
        display.drawLine(x1, y1, x2, y2, GxEPD_RED);

      else
        display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
    }

  } while (display.nextPage());

  deepSleep();
}

void deepSleep()
{
  display.hibernate();
  delay(10);

  // https: //github.com/esp8266/Arduino/issues/644
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  ESP.deepSleep(interval * 1e6 /*, WAKE_RF_DISABLED*/); //in seconds
}

void printStatic() //prints wifi signal strength and VCC in top right corner
{
  display.fillRect(208, 0, 88, 13, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);

  display.setCursor(250, 11);
  display.print(vddString);

  display.setCursor(210, 11);
  display.print(rssiString);
  display.setTextColor(GxEPD_BLACK);
  // display.drawLine(0, 13, HEIGHT, 13, GxEPD_RED);
}

void printTextCenter(char *text) // prints text in center of the screen. Used to display errors
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
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
}

int dBmtoPercentage(int dBm)
{
  const int RSSI_MAX = -50;  // define maximum strength of signal in dBm
  const int RSSI_MIN = -100; // define minimum strength of signal in dBm
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
}

/////////////////////////////////////////
void update_started()
{
  Serial.println("CALLBACK:  HTTP update process started");
  printTextCenter("Firmware update...");
}

void update_finished()
{
  Serial.println("CALLBACK:  HTTP update process finished");
  printTextCenter("Firmware update DONE!");
}


#include "ESP8266httpUpdate.h"

void updateFirmware()
{

  // https://raw.githubusercontent.com/eziosoft/weatherEpaperDisplay/e_paper_logo.ino.d1.bin
  // const char *host = "raw.githubusercontent.com";
  // const int httpsPort = 443;
  // const char *url = "/eziosoft/weatherEpaperDisplay/master/e_paper_logo.ino.d1.bin";

  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  // ESPhttpUpdate.onProgress(update_progress);
  // ESPhttpUpdate.onError(update_error);

  
  auto ret = ESPhttpUpdate.update("http://eziosoft.com/api/weatherDisplayFW/e_paper_logo.ino.d1.bin");

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    printTextCenter("FW update failed");
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");

    break;
  }
}
