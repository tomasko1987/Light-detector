#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include "WiFi.h"
#include <WiFiUdp.h>
#include <DFRobot_B_LUX_V30B.h> // @https://github.com/DFRobot/DFRobot_B_LUX_V30B 

//The sensor chip is set to 13 pins, SCL and SDA adopt default configuration
DFRobot_B_LUX_V30B    myLux(13);
// Settings for AWS 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
// Settings for local Home Assistant 
WiFiClient localWifiClient;
PubSubClient localClient(localWifiClient);
// Settings NTP time client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", TIMEOFFSET);
// sensor data variable
constexpr size_t bufferSize = 512;
char jsonBuffer[bufferSize];

void reconnectWiFi() {
  WiFi.disconnect();
  WiFi.reconnect();

  Serial.println("Trying to reconnect to WiFi...");
  while (WiFi.status() != WL_CONNECTED ) {
    delay(5000);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi reconnected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Failed to reconnect to WiFi. Check your credentials or WiFi signal.");
  }
}

bool sensorData()
{
  //check if "NTP" time server is reachable
  if(timeClient.forceUpdate())
  {
    // get actual time
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    // Formát "yyyy-MM-dd HH:mm:ss"
    char dateTimeStr[20]; 
    strftime(dateTimeStr, sizeof(dateTimeStr), "%Y-%m-%d %H:%M:%S", ptm);
    
    // Read the light level in lux
    double lux = myLux.lightStrengthLux(); 
    // Cast double to int, truncating the decimal part
    int luxInt = static_cast<int>(lux); 
    
    // generate JSON 
    StaticJsonDocument<200> doc;
    doc["light"] = luxInt;
    doc["date"] = dateTimeStr;
    doc["id"] = PUBLISH_MESSAGE_ID;

    // serialized JSON
    serializeJson(doc, /*buffer*/jsonBuffer, bufferSize); 
    return true;
  }
  else
  {
    Serial.println("NTP update failed. Check your internet connection or NTP server address");
    return false;
  }
}

void sendToLocalMQTT()
{
  // check if connection to local MQTT broker exist
  if (!localClient.connected()) 
  {
    Serial.println("Connecting to local MQTT broker...");
    if (localClient.connect("ESP32Client", HA_MQTT_USER, HA_MQTT_PASSWORD)) 
    {
      Serial.println("Connected to local MQTT broker");
    } 
    else 
    {
      Serial.println("Connecting to local MQTT broker ERROR");
      return;
    }     
  }
  
  // publish to local MQTT broker
  if(localClient.publish(HA_MQTT_TOPIC, jsonBuffer))
  {
    Serial.println("Publish to local MQTT broker");
    Serial.println(jsonBuffer);      
  }
  else
  {
    Serial.println("Publish to local MQTT broker ERROR");      
  }  
}

void sendToAWS()
{
  Serial.println("Connecting to AWS IOT"); 
  // connect to AWS IoT Thing
  if(client.connect(THINGNAME))
  {
    Serial.println("Connected to AWS IOT");
    // Subscribe to a topic
    if(client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC))
    {
      Serial.println("Subscribe to AWS Topic");
      // publish message to AWS
      if(client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer))
      {
        Serial.println("Publish Message to AWS TOPIC");
        Serial.println(jsonBuffer);
      }
      else
      {
        Serial.println("Publish Message to AWS TOPIC ERROR");
      } 

      // unSubscribe from a topic
      if(client.unsubscribe(AWS_IOT_SUBSCRIBE_TOPIC))
      {
        Serial.println("unSubscribe from a AWS Topic");    
      }
      else
      {
        Serial.println("unSubscribe from a AWS Topic ERROR");    
      }
    }
    else
    {     
      Serial.println("Subscribe to AWS Topic ERROR");      
    }
  }
  else
  {
    Serial.println("Connected to AWS IOT ERROR");    
  }

  // disconnect from AWS
  client.disconnect();
  Serial.println("disconnect from AWS IoT");
}

void setup() {
  // serial port begin
  Serial.begin(115200); 

  // wifi begin
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // light sensor begin
  myLux.begin();

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint
  client.setServer(AWS_IOT_ENDPOINT, 8883);
   
  // Connected to local MQTT broker
  localClient.setServer(HA_MQTT_SERVER, HA_MQTT_PORT);

  // NTP time client 
  timeClient.begin();
}

void loop() {
  // check if WIFI connection is establish
  if(WiFi.status() != WL_CONNECTED) 
  {
    reconnectWiFi();
  }
  // get data from sensor
  if(sensorData())
  {
    // send data to local MQTT Broker
    sendToLocalMQTT();
    // send data to AWS
    sendToAWS();
  }
  client.loop();
  delay(LOOP_PERIOD);
}
