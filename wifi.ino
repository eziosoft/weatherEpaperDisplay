#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 2048

const char *ssid = "Epaper";         //robot creates wifi hotspot when wifi connection is not configured
const char *outTopic = "epaper/out"; //MQTT topic for robot telemetry messages
const char *jsonTopic = "epaper/json";
const char *mqtt_server = "192.168.0.19"; //my  MQTT server

WiFiClient espClient;
PubSubClient client(espClient); //MQTT
char buffer1[20];               //multiusage

void configModeCallback(WiFiManager *myWiFiManager);

void setupWifi()
{
  client.setBufferSize(MQTT_MAX_PACKET_SIZE);

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect(ssid))
  {
    Serial.println("failed to connect and hit timeout");
    printTextCenter("failed to connect and hit timeout");
    delay(100);
    ESP.deepSleep(10e6);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
}

void loopWifi()
{
  //MQTT
  if (!client.connected())
    reconnect();
  else
    client.loop();
}

void sendTelemetry()
{
  char buf[50];
  sprintf(buf, "T;%s;%d;RSSI=%d;Bat=%d", ssid, millis(), (int)rssi, (int)(vdd * 100));
  client.publish(outTopic, buf, true);
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
  Serial.print("MQTT:");

  char buf[length];
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    buf[i] = (char)payload[i];
  }

  String strTopic = String((char *)topic);
  if (strTopic == jsonTopic)
  {
    json(buf);
  }
}

int tries = 10;
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ssid))
    {
      Serial.println("connected");
      client.subscribe(jsonTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
      tries--;
      if (tries == 0)
      {
        deepSleep();
      }
    }
  }
}
