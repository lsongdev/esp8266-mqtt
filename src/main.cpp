#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = "wifi@lsong.org";
const char *password = "song940@163.com";
const char *mqttServer = "broker.emqx.io";
const char *clientID = "esp8266-mqtt";
const char *relayTopic = "esp8266-mqtt";

const int ledPin = LED_BUILTIN;

WiFiClient wlan;
PubSubClient mqtt(wlan);

void reconnect()
{
  while (!mqtt.connected())
  {
    Serial.println("Attempting to connect to the MQTT server...");
    if (mqtt.connect(clientID))
    {
      Serial.println("MQTT connected");
      mqtt.publish(relayTopic, "online");
      mqtt.subscribe(relayTopic);
    }
    else
    {
      Serial.println("MQTT connect failed, retrying...");
      delay(2000);
    }
  }
}

void onMessage(char *topic, byte *payload, unsigned int length)
{
  // Handle received messages
  String payloadStr = "";
  for (unsigned int i = 0; i < length; i++)
  {
    payloadStr += (char)payload[i];
  }
  Serial.println("Received message: [" + String(topic) + "] " + payloadStr);

  if (payloadStr == "on")
  {
    digitalWrite(ledPin, LOW); // Turn on the LED
  }
  else if (payloadStr == "off")
  {
    digitalWrite(ledPin, HIGH); // Turn off the LED
  }
  else if (payloadStr == "ping")
  {
    mqtt.publish(relayTopic, "pong");
  }
}

void setup()
{

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Set the MQTT server and port
  mqtt.setClient(wlan);
  mqtt.setServer(mqttServer, 1883);
  mqtt.setKeepAlive(15);
  mqtt.setCallback(onMessage);
}

void loop()
{
  if (!mqtt.connected())
  {
    reconnect();
  }
  mqtt.loop();
}
