#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "time.h"
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>

int LED_BUILTIN = 2;

// ===== WiFi Credentials =====
const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "password";

// ===== AWS IoT Config =====
const char* AWS_HOST = "a2s4crf9kqjne6-ats.iot.us-east-1.amazonaws.com";
const int AWS_PORT = 8883;

// ===== MQTT Topic =====
const char* MQTT_TOPIC = "airbuild/pods/pod1";

// ===== Certificates =====
static const char ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char DEVICE_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUS4/eccRHiMT1HG2EgrHmJJydd3AwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTIxMjAyNDIx
NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbkmCArH2OG4ZIeJEN5
C2F1ZVx1rdR7YcHfQwM9oIWTJEqclfRFaRNIkG9L9+NPKqG495jGm9ipFycSENSP
m0smmGxNeGm9Ohxv6sWGDpv6DKUSD94lq1AlYPpC2+Sh6snyRc+or/3rD7lJxR7v
7wcMvRH+yk4IjvWchR1srW9GI2wnRl8YMXPQdjiRaU5Gl1+OLkoKTE9WyDbb4Q+l
tbfgu5bn/i30xQs8TT18XmDMe1T7HJ4/haTqcH0SKwoLyfww2AXDaIG0ZWLW51KG
49Rl5R1AkEqcj/9pGAziPiw4lXSkB77lPIEvb4Zf90a5BtK+Cr/B2hr31a4p7Ipt
LN0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUmFftopg0x4xRRdkVl9XFDelme28wHQYD
VR0OBBYEFFXsKRV0rsOw1k3CpuBDPCBNO37BMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBxBr+Rf73gogOx91oIMIfQjcX+
pjfPab/7MHRw8fBS4+9kqYyLEiSMZyIMhFUa5lUbowH8ywd+a8oqT5mSGstTI33A
0GOVgWSnTrYvnAHnIlNkSgWZhaSsv9g9E/r22gPCc/JC8+yoDf3AOWn6rHXNZnmz
dNZEHWKvC9skkr/149ppgVVrl6LvxlDj/q3DskjLmzE3Rw66H3XkDXiMWKW+Z9MO
gCnSbQg7gqgy4Y7mpbvJUFI/2qMue2GhAikaO4Do3odGgXLDNQyH7mFzfX/fuAsV
cunNj0Z3988r8PsHELtotk4U3QQ4ybImEXIvLZirb9G1KegNBVfRc7ekk1n6
-----END CERTIFICATE-----
)EOF";

static const char PRIVATE_KEY[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEAxuSYICsfY4bhkh4kQ3kLYXVlXHWt1Hthwd9DAz2ghZMkSpyV
9EVpE0iQb0v3408qobj3mMab2KkXJxIQ1I+bSyaYbE14ab06HG/qxYYOm/oMpRIP
3iWrUCVg+kLb5KHqyfJFz6iv/esPuUnFHu/vBwy9Ef7KTgiO9ZyFHWytb0YjbCdG
Xxgxc9B2OJFpTkaXX44uSgpMT1bINtvhD6W1t+C7luf+LfTFCzxNPXxeYMx7VPsc
nj+FpOpwfRIrCgvJ/DDYBcNogbRlYtbnUobj1GXlHUCQSpyP/2kYDOI+LDiVdKQH
vuU8gS9vhl/3RrkG0r4Kv8HaGvfVrinsim0s3QIDAQABAoIBAQC7ctpFBx9QJ+EU
W0AGL2Cge4oKOAaaiaY6l/aKaVyOR30erLFPRNOhDvTBUDHSk9w0CyWfYR+kdMiT
VB10l9veWgOxntnrniHUh04W1x+RtXqRYJJ/5bStlmztsHGZ61hi9+j/q5Yik5WA
JZyYab03QbQ375AvNzW1Gc0DTFew1BnOdHj2R6VlQjEvmXFwd2s9lfuSiB/XljSH
OCy/2YDJTbDjzcNiDNNpIX8js2JDF7ufPJaoGWTpcf2Wkfs/SWlRb5TuHz3dFd8y
9UgN/u4iTygN+D1QYfw6KLvN7Tw9zsYUaG/2SDYNvVYYo5Fh7nsQ7U7KiAl2DreK
ic+/zEIBAoGBAO1/WAgmItiCmmHOyLcm1dDUyA0hyh+lSZyT5hUu6ANiHSAw1gli
en80XY88BqRSN+yVSgfx9N8zCq8aNqjBIsOFdw2OOMzJ4XbI1neBzvo+5q9Xz4G4
N4J037rlOOqG7BaxvD5IcO5WrWcBgJEHlRtggfTUUyJG/G/93xooJ0dBAoGBANZj
UjvZPz1zdInEM5uOw4NFFI79LjOcp9lomabwwUZlYzWZa4JrTcDBBh2Rk4sx/g++
X3Ka5nxbNyutnNNbkR07Kbyu2Oa3/8a30yXaOOLKNBtl7jZmTF1b1JTjyK9FmBPy
u5pq7FExHfkkc8uWBYS+Xu9qkjRu5IUw9lXngfqdAoGAHbgmwvZKu/8z5HNk6I5i
iTxnQEVvLqCXYpGbZCE3hVzcui+CSTBaldpursLbLerf9qoD1pSviNKxTpIgLuPk
PsVPzZFziCTnIt8k/1VuGiCO63g4jkoIIDoR2ShgA/EYUAqzvLOB9kFAd7hXZ06k
mQVtnb6IHKz3X+hL59EIgUECgYEAp3cUEG4GCUF7ofXh/jxJmuvGQF+izGwtbRVZ
GN9tiki+c2rxYQHizZO57DUAFLYraC7kLgizzCYQdYz7ONqJA1Vop9vI0IigQqDr
TeElmHoss84ugs3sDu0hpGA+LjNltQdh5iBpko6UhD4mObxpeSSf0dLkfBK3bRcv
BTF/Bb0CgYEAvorpSsWrQaypqNhPsf5me1sM/gqCBlQLNjnGnrif57HNqZLMOvfy
1+Jqn0UgTqky5YMIeNnipTIb0HQpOHSAlUJ9iW6OWn50WmAk4rSs/IO0cH6uUqSH
JSKIXJNzC5E46FVU8aixr2lubT+Fm62b2DvezL7xWm6V6bn3rPMwjPM=
-----END RSA PRIVATE KEY-----
)EOF";

// ===== Sensors =====
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

// ===== WiFi / MQTT =====
WiFiClientSecure net;
PubSubClient client(net);

const uint64_t SLEEP_TIME = 300ULL * 1000000ULL;

// ===== Timestamp =====
String getTimeStamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "";
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%z", &timeinfo);
  return String(buffer);
}

// ===== Sensor Reads =====
float readTemp() {
  float t = dht.readTemperature();
  if (isnan(t) || t < -20 || t > 80) return NAN;
  return t;
}

float readHum() {
  float h = dht.readHumidity();
  if (isnan(h) || h < 0 || h > 100) return NAN;
  return h;
}

float readLux() {
  float l = lightMeter.readLightLevel();
  if (l < 0 || l > 120000) return NAN;
  return l;
}

// ===== AWS Connect =====
void connectAWS() {
  while (!client.connected()) {
    client.connect("panelB-client");
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  net.setCACert(ROOT_CA);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(PRIVATE_KEY);

  client.setServer(AWS_HOST, AWS_PORT);

  dht.begin();

  Wire.begin(21, 22);     // I2C fix for BH1750
  lightMeter.begin();

  configTime(0, 0, "pool.ntp.org");

  connectAWS();

  float temp = readTemp();
  float hum = readHum();
  float lux = readLux();
  String timestamp = getTimeStamp();

  String payload = "{";
  payload += "\"podId\":\"pod1\",";
  payload += "\"panelId\":\"panel2\",";
  payload += "\"panel_type\":\"algae\",";
  payload += "\"timestamp\":\"" + timestamp + "\",";
  payload += "\"temperature\":" + String(temp, 2) + ",";
  payload += "\"humidity\":" + String(hum, 2) + ",";
  payload += "\"lux\":" + String(lux, 2);
  payload += "}";

  if (client.publish(MQTT_TOPIC, payload.c_str())) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);
  }

  client.loop();

  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  esp_deep_sleep_start();
}

void loop() {}
