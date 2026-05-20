#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== WiFi Credentials =====
const char* WIFI_SSID = "Cheerstopeace";
const char* WIFI_PASSWORD = "Wedontshare@321";

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
... (same key continues exactly as yours) ...
-----END RSA PRIVATE KEY-----
)EOF";

// ===== Pins =====
#define ONE_WIRE_BUS 4
#define PH_PIN 34
#define CO2_PIN 33

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ===== Sleep =====
const uint64_t SLEEP_INTERVAL_US = 300ULL * 1000000ULL;

WiFiClientSecure net;
PubSubClient client(net);

// ===== Time =====
String getTimeStamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "";
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buffer);
}

// ===== Temperature =====
float readTemperature() {
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);
  if (isnan(t) || t < -20 || t > 80) return NAN;
  return t;
}

// ===== pH =====
float readPH() {
  analogSetPinAttenuation(PH_PIN, ADC_11db);

  const int SAMPLES = 10;
  float sum = 0;
  int valid = 0;

  for (int i = 0; i < SAMPLES; i++) {
    int raw = analogRead(PH_PIN);
    float v = raw * (3.3 / 4095.0);

    if (v > 0.1 && v < 3.2) {
      float ph = (-5.882 * v) + 23.29;

      if (ph >= 0 && ph <= 14) {
        sum += ph;
        valid++;
      }
    }
    delay(100);
  }

  if (valid == 0) return NAN;
  return sum / valid;
}

// ===== CO2 =====
float readCO2() {
  int raw = analogRead(CO2_PIN);
  float voltage = raw * (3.3 / 4095.0);

  if (voltage < 0.1 || voltage > 3.2) return NAN;

  float co2 = voltage * 1000;
  return co2;
}

// ===== AWS =====
void connectAWS() {
  while (!client.connected()) {
    if (client.connect("panelA-client")) break;
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  net.setCACert(ROOT_CA);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(PRIVATE_KEY);
  client.setServer(AWS_HOST, AWS_PORT);

  sensors.begin();

  configTime(0, 0, "pool.ntp.org");
  while (time(nullptr) < 100000) delay(500);

  connectAWS();

  float temperature = readTemperature();
  float ph = readPH();
  float co2 = readCO2();
  String timestamp = getTimeStamp();

  if (isnan(temperature)) temperature = -1;
  if (isnan(ph)) ph = -1;
  if (isnan(co2)) co2 = -1;

  String payload = "{";
  payload += "\"podId\":\"pod1\",";
  payload += "\"panelId\":\"panelA\",";
  payload += "\"panel_type\":\"algae\",";
  payload += "\"timestamp\":\"" + timestamp + "\",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"ph\":" + String(ph, 2) + ",";
  payload += "\"co2\":" + String(co2, 2);
  payload += "}";

  client.publish(MQTT_TOPIC, payload.c_str());

  client.loop();
  delay(500);

  esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL_US);
  esp_deep_sleep_start();
}

void loop() {}