#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Pastikan pustaka ArduinoJson diinstal

// WiFi and MQTT server credentials
const char* ssid = "Satria";        // Ganti dengan SSID WiFi Anda
const char* password = "Satria12";  // Ganti dengan Password WiFi Anda
const char* mqtt_server = "192.168.26.60";  // Ganti dengan alamat broker MQTT
const int mqtt_port = 1883;                 // Port MQTT
const char* mqtt_user = "";   // Ganti dengan username MQTT jika diperlukan
const char* mqtt_password = ""; // Ganti dengan password MQTT jika diperlukan

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Pin definitions
const int flowPin = D5;    // Flow sensor pin
const int soilPin = A0;    // Soil moisture sensor pin
const int trigPin = D1;    // Ultrasonic sensor trigger pin
const int echoPin = D2;    // Ultrasonic sensor echo pin

// Flow sensor variables
volatile int flowCount = 0;
int tankHeight = 100;  // Tank height in cm

void IRAM_ATTR countFlow() {
  flowCount++;
}

void setup() {
  Serial.begin(115200);

  // Initialize pin modes
  pinMode(flowPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Attach interrupt for flow sensor
  attachInterrupt(digitalPinToInterrupt(flowPin), countFlow, RISING);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Connect to MQTT
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    client.connect("ESP8266Client", mqtt_user, mqtt_password);
    delay(500);
  }
  Serial.println("MQTT connected");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Flow sensor reading
  flowCount = 0;
  delay(1000);  // Measure for 1 second
  float flowRate = (flowCount / 7.5);  // Flow rate in L/min

  // Soil moisture sensor reading
  int soilMoistureValue = analogRead(soilPin);

  // Ultrasonic water level reading
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;  // Convert to cm
  int waterLevel = tankHeight - distance;

  // Create JSON object and add sensor data
  StaticJsonDocument<200> doc;
  doc["flowRate"] = flowRate;
  doc["soilMoistureValue"] = soilMoistureValue;
  doc["waterLevel"] = waterLevel;

  // Serialize JSON to string
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  // Send JSON to MQTT
  client.publish("sensor/data", jsonBuffer);

  // Output to serial monitor for debugging
  Serial.print("Flow rate: ");
  Serial.print(flowRate);
  Serial.println(" L/min");
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoistureValue);
  Serial.print("Water Level: ");
  Serial.println(waterLevel);

  // Wait before next measurement cycle
  delay(2000);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 2 seconds");
      delay(2000);
    }
  }
}
