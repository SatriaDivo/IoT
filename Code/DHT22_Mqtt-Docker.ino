#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Pastikan pustaka ArduinoJson diinstal


#define DHTPIN D4        // Pin yang terhubung dengan DHT11
#define DHTTYPE DHT22    // Jenis DHT


DHT dht(DHTPIN, DHTTYPE);
const char* ssid = "Satria";          // Ganti dengan SSID WiFi Anda
const char* password = "Satria12";   // Ganti dengan Password WiFi Anda
const char* mqttServer = "192.168.26.60"; // Ganti dengan alamat broker MQTT
const int mqttPort = 1883;                 // Port MQTT
const char* mqttUser = "";   // Ganti dengan username MQTT jika diperlukan
const char* mqttPassword = ""; // Ganti dengan password MQTT jika diperlukan


WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  dht.begin();


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");


  client.setServer(mqttServer, mqttPort);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  float h = dht.readHumidity();
  float t = dht.readTemperature();


  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  // Membuat objek JSON
  StaticJsonDocument<200> doc;
  doc["temperature"] = t;
  doc["humidity"] = h;


  // Mengubah objek JSON menjadi string
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);


  Serial.print("Sending JSON: ");
  Serial.println(jsonBuffer);


  // Mengirim data JSON ke broker MQTT
  client.publish("home/dht11", jsonBuffer);


  delay(2000); // Interval pengiriman data
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("NodemcuClient", mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 2 seconds");
      delay(2000);
    }
  }
}