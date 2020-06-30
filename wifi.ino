#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 2048

const char *ssid = "Epaper";         //robot creates wifi hotspot when wifi connection is not configured
const char *outTopic = "epaper/out"; //MQTT topic for robot telemetry messages
const char *inTopic = "epaper/in";   //MQTT topic for control messages
const char *jsonTopic = "epaper/json";
const char *mqtt_server = "192.168.0.19"; //my  MQTT server
const char *mqtt_server1 = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient); //MQTT
char buffer1[20];               //multiusage

void configModeCallback(WiFiManager *myWiFiManager);

void setupWifi()
{
  client.setBufferSize(MQTT_MAX_PACKET_SIZE);
  
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  // printText("Connecting...");
  if (!wifiManager.autoConnect(ssid))
  {
    Serial.println("failed to connect and hit timeout");
    printText("failed to connect and hit timeout");
    delay(100);
    ESP.deepSleep(10e6);
  }

  // printText(WiFi.localIP());
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

  //send telemetry every 200ms
  // if (millis() % 10000 == 0)
  // {
  //   sprintf(buffer1, "T;%s;%d;RSSI=%d;Bat=%s", ssid, millis() / 1000, WiFi.RSSI(),vddString);
  //   client.publish(outTopic, buffer1);
  // }
}

void sendTelemetry()
{
  char buf[50];
  sprintf(buf, "T;%s;%d;RSSI=%d;Bat=%d", ssid, millis(), (int)rssi, (int)(vdd * 100));
  client.publish(outTopic, buf);
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  printText("Configuration");
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char buf[length];
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    buf[i] = (char)payload[i];
  }

  String strTopic = String((char *)topic);
  if (strTopic == inTopic)
  {
    update(buf);
  }
  else
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
      // Once connected, publish an announcement...
      // client.publish(outTopic, "Tank READY");
      // ... and resubscribe
      // client.subscribe(inTopic);
      client.subscribe(jsonTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
      tries--;
      if (tries == 0)
      {
        deepSleep();
      }
    }
  }
}
