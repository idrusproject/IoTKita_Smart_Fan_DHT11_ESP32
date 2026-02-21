#include <IoTKita.h>
#include <DHT.h>

// ================= CONFIG =================
const char* ssid     = "nama wifi yang di pakai esp32";
const char* password = "password wifi yang di pakai esp32";
const char* apiKey   = "iotkita_xxxxxxxxxxxxxxxxxxxxxx";

const char* mqttHost = "mqtt.iotkita.com";
const char* mqttUser = "iot_kita";
const char* mqttPass = "iot_kita_shared_mqtt";
int mqttPort         = 1883;

// ================= PIN =================
#define DHTPIN 33
#define DHTTYPE DHT11
#define RELAY_PIN 32   // Active LOW

// ================= TOPIC =================
String topicTemp  = "iotkita/temp";         // Jika menggunakan shared broker IoT Kita pastikan menggunakan topic yang sangat unik
String topicHum   = "iotkita/hum";          // Jika menggunakan shared broker IoT Kita pastikan menggunakan topic yang sangat unik
String topicRelay = "iotkita/relay";        // Jika menggunakan shared broker IoT Kita pastikan menggunakan topic yang sangat unik

// ================= OBJECT =================
DHT dht(DHTPIN, DHTTYPE);
IoTKita iotkita;

unsigned long lastSend = 0;

// ================= CALLBACK RELAY =================
void aksiRelay(String topic, String message) {
  Serial.printf("Topic: %s | Message: %s\n", topic.c_str(), message.c_str());

  if (message == "1" || message == "on") {
    digitalWrite(RELAY_PIN, LOW);   // ON (active LOW)
    Serial.println("Relay ON");
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // OFF
    Serial.println("Relay OFF");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // default OFF

  dht.begin();

  iotkita.begin(apiKey);
  iotkita.setupWiFi(ssid, password);
  iotkita.setupMQTT(mqttHost, mqttPort, mqttUser, mqttPass);

  iotkita.dataSubscribe(topicRelay, aksiRelay);

  Serial.println("=== SYSTEM READY ===");
}

void loop() {
  iotkita.sync();

  unsigned long now = millis();
  if (now - lastSend >= 1000) {
    lastSend = now;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Gagal baca DHT11");
      return;
    }

    Serial.printf("Temp: %.2f C | Hum: %.2f %%\n", temperature, humidity);

    bool ok1 = iotkita.sendRaw(topicTemp, String(temperature));
    bool ok2 = iotkita.sendRaw(topicHum, String(humidity));

    if (ok1 && ok2) {
      Serial.println(">> Data terkirim!");
    } else {
      Serial.println(">> Gagal kirim salah satu data!");
    }
  }
}
