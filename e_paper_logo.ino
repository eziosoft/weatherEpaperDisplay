#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <StreamString.h>
#include <GxEPD2_3C.h>
#include <ArduinoJson.h>

#include "ESP8266WiFi.h"
#include "icons.h"

#define PrintString StreamString
#define ENABLE_GxEPD2_GFX 0


static const uint16_t WIDTH = 152;
static const uint16_t HEIGHT = 296;

ADC_MODE(ADC_VCC);

//milliseconds to sleep
int intervall = 60000;

struct bitmap_pair
{
  const unsigned char *black;
  const unsigned char *red;
};

GxEPD2_3C<GxEPD2_260c, GxEPD2_260c::HEIGHT> display(GxEPD2_260c(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
// GxEPD2_BW<GxEPD2_260, GxEPD2_260::HEIGHT> display(GxEPD2_260(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
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
  if (millis() > 20000)
  {
    printText("timeout");
    delay(100);
    deepSleep();
  }
}

void update(char *text)
{
  printText1(text);
  display.hibernate();
  sendTelemetry();

  delay(100);
  deepSleep();
}

void json(char *json)
{
  sendTelemetry();
  delay(100);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.println("\n\nWIFI OFF");

  const size_t capacity = JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(2) + 10 * JSON_OBJECT_SIZE(3) + 230;
  DynamicJsonDocument doc(capacity);

  // const char *json = "{\"nextRefresh\":60000,\"data\":[{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"},{\"x\":10,\"y\":10,\"text\":\"demo text\"}]}";

  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    printText("json error");
    deepSleep();
    return;
  }

  long nextRefresh = doc["NR"]; // 60000
  intervall = nextRefresh;

  int icon = doc["i"];
  JsonArray data = doc["d"];

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
      display.drawInvertedBitmap(HEIGHT - 100, 20, i01n, 100, 100, GxEPD_BLACK);
      break;
    case 2:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i02n, 100, 100, GxEPD_BLACK);
      break;
    case 3:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i03n, 100, 100, GxEPD_BLACK);
      break;
    case 4:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i04n, 100, 100, GxEPD_BLACK);
      break;
    case 9:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i09n, 100, 100, GxEPD_BLACK);
      break;
    case 10:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i10n, 100, 100, GxEPD_BLACK);
      break;
    case 11:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i11n, 100, 100, GxEPD_BLACK);
      break;
    case 13:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i13n, 100, 100, GxEPD_BLACK);
      break;
    case 50:
      display.drawInvertedBitmap(HEIGHT - 100, 20, i50n, 100, 100, GxEPD_BLACK);
      break;
    default:
    break;
    }

    for (int i = 0; i < 10; i++)
    {
      JsonObject datai = data[i];
      int data_x = datai["x"]; // 10
      int data_y = datai["y"]; // 10
      int font = datai["f"];
      int color = datai["c"];
      const char *data_text = datai["t"]; // "demo text"

      if (font == 1)
      {
        display.setFont(&FreeMonoBold24pt7b);
      }
      else
      {
        display.setFont(&FreeMonoBold9pt7b);
      }

      if (color == 1)
      {
        display.setTextColor(GxEPD_RED);
      }
      else
      {
        display.setTextColor(GxEPD_BLACK);
      }

      display.setCursor(data_x, data_y);
      display.print(data_text);
    }

  } while (display.nextPage());

  deepSleep();
}

void deepSleep()
{
  Serial.println("deep sleep");
  display.hibernate();
  delay(10);

https: //github.com/esp8266/Arduino/issues/644
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  ESP.deepSleep(intervall * 1000 /*, WAKE_RF_DISABLED*/);
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
  display.fillRect(208, 0, 88, 13, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);

  display.setCursor(250, 11);
  display.print(vddString);

  display.setCursor(210, 11);
  display.print(rssiString);
  display.setTextColor(GxEPD_BLACK);
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

// void drawBitmap()
// {

//   bitmap_pair bitmap_pairs[] =
//       {
//           {gImage_e, gImage_r}};

//   for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
//   {
//     display.firstPage();
//     do
//     {
//       display.fillScreen(GxEPD_WHITE);
//       display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
//       //      display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
//     } while (display.nextPage());
//     delay(2000);
//   }
// }

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
