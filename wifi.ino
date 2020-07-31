#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 2048

const char *ssid = "Epaper";              //robot creates wifi hotspot when wifi connection is not configured
const char *outTopic = "epaper/out";      //MQTT topic for telemetry messages
const char *jsonTopic = "epaper/json";    //MQTT control topic
const char *mqtt_server = "192.168.0.19"; //my  MQTT server

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); //MQTT
// char buffer1[20];               //multiusage

uint32_t ID = ESP.getChipId();

void configModeCallback(WiFiManager *myWiFiManager);

void setupWifi()
{
  mqttClient.setBufferSize(MQTT_MAX_PACKET_SIZE);

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect(ssid))
  {
    Serial.println("failed to connect and hit timeout");
    printTextCenter("failed to connect and hit timeout");
    delay(100);
    ESP.deepSleep(10e6);
  }

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void loopMQTT()
{
  //MQTT
  if (!mqttClient.connected())
    mqttRecontect();
  else
    mqttClient.loop();
}

void sendTelemetry(char *text) //text - max 80 chars
{
  if (WiFi.status() == WL_CONNECTED)
  {
    char buf[200];

    sprintf(buf, "{\"name\":\"epaperDisplay\",\"id\":%d,\"version\":%d,\"ssid\":\"%s\",\"upTimeMs\":%d,\"RSSI\":%d,\"VDD\":%d,\"err\":\"%s\"}", ID, FW_VERSION, ssid, millis(), (int)rssi, (int)(vdd * 100), text);
    mqttClient.publish(outTopic, buf, true);
  }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  printTextCenter("Configuration");
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  char buf[length];
  for (int i = 0; i < length; i++)
  {
    buf[i] = (char)payload[i];
  }

  String strTopic = String((char *)topic);
  if (strTopic == jsonTopic)
  {
    json(buf);
  }
}

int reconnectAttempts = 10;
void mqttRecontect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(ssid))
    {
      Serial.println("connected");
      mqttClient.subscribe(jsonTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 1 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
      reconnectAttempts--;
      if (reconnectAttempts == 0)
      {
        printTextCenter("MQTT: unable to connect");
        delay(100);
        deepSleep();
      }
    }
  }
}
