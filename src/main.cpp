#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

const char *apName = "ESP8266-MQTT";
const char *clientID = "esp8266-led";
const char *ledTopic = "esp8266-led/led";
const int ledPin = LED_BUILTIN;

WiFiClient wlan;
PubSubClient mqtt(wlan);
WiFiManager wifiManager(apName);

// Add custom parameters for configuring MQTT server information
WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", "192.168.8.160", 40);
WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", "1883", 6);
WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", "mqtt", 40);
WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password", "mqtt123", 40);

// WiFiManager callback function
void saveConfigCallback()
{
  Serial.println("Configuration saved");
}

void reconnect()
{
  String user = custom_mqtt_user.getValue();
  String pass = custom_mqtt_password.getValue();
  while (!mqtt.connected())
  {
    Serial.println("Attempting to connect to the MQTT server...");
    if (mqtt.connect(clientID, user.c_str(), pass.c_str()))
    {
      Serial.println("MQTT connected");
      mqtt.subscribe(ledTopic);
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

  Serial.print("Received message: [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(payloadStr);

  if (strcmp(topic, ledTopic) == 0)
  {
    // If an LED control message is received, perform the corresponding action
    if (payloadStr == "on")
    {
      digitalWrite(ledPin, LOW);
    }
    else if (payloadStr == "off")
    {
      digitalWrite(ledPin, HIGH);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.autoConnect();

  // Set the MQTT server and port
  mqtt.setServer(custom_mqtt_server.getValue(), atoi(custom_mqtt_port.getValue()));
  mqtt.setCallback(onMessage);
  mqtt.setClient(wlan);
}

void loop()
{
  if (!mqtt.connected())
  {
    reconnect();
  }
  mqtt.loop();
}