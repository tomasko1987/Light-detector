#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "roofLight_1"
// AWS
#define AWS_IOT_PUBLISH_TOPIC   "house/roof/roofLight_1"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
const char AWS_IOT_ENDPOINT[] = "";       
// WIFI
const char WIFI_SSID[] = "";               
const char WIFI_PASSWORD[] = "";           
// time zone at GMT+1 for Bratislava (3600 seconds = 1 hour)
const int TIMEOFFSET = 3600;
// Configurate Home Assistant MQTT Broker
const char* HA_MQTT_SERVER = ""; 
const int HA_MQTT_PORT = 1883; 
const char* HA_MQTT_USER = ""; 
const char* HA_MQTT_PASSWORD = ""; 
const char* HA_MQTT_TOPIC = "home/roof/roofLight_1"; 
// Publish Message ID
const char* PUBLISH_MESSAGE_ID = "roof1"; 
// Loop period
const int LOOP_PERIOD = 10000; //300000-5.min  600000-10.min
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                               
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)KEY";
 
// Device Private Key                                               
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";