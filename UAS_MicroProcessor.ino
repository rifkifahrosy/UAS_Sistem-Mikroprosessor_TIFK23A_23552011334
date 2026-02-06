#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

const char* ssid        = "Nailah";
const char* password    = "mindatati123";
const char* mqtt_server = "broker.hivemq.com"; 

#define PIN_BUTTON 14   
#define PIN_LED    12   

const int freq = 5000;
const int resolution = 8; 

Preferences preferences;
WiFiClient espClient;
PubSubClient client(espClient);

volatile bool buttonPressed = false;
int brightnessStep = 0; // 0, 1, 2, 3 (untuk level terang)
int currentPWM = 0;

void IRAM_ATTR handleButton() {
  buttonPressed = true; 
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Terhubung!");
}

void reconnectMQTT() {
  while (!client.connected()) {
    String clientId = "ESP32-Wemos-" + String(random(0, 999));
    if (client.connect(clientId.c_str())) {
      client.subscribe("wemos/led/set"); 
    } else { vTaskDelay(2000 / portTICK_PERIOD_MS); }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  int val = msg.toInt();
  if (val >= 0 && val <= 255) {
    currentPWM = val;
    ledcWrite(PIN_LED, currentPWM);
  }
}

// TASK 1: Komunikasi
void TaskNetwork(void *pvParameters) {
  setupWiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
  for (;;) {
    if (!client.connected()) reconnectMQTT();
    client.loop();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// TASK 2: Kontrol & Logika Interupsi
void TaskControl(void *pvParameters) {
  preferences.begin("uas", false);
  currentPWM = preferences.getInt("led_val", 0);
  ledcWrite(PIN_LED, currentPWM);

  for (;;) {
    if (buttonPressed) {
      buttonPressed = false; // Reset flag interupsi
      
      // Logika: Berpindah level terang (Cycle)
      brightnessStep++;
      if (brightnessStep > 3) brightnessStep = 0;

      if (brightnessStep == 0) currentPWM = 0;
      else if (brightnessStep == 1) currentPWM = 80;
      else if (brightnessStep == 2) currentPWM = 170;
      else if (brightnessStep == 3) currentPWM = 255;

      ledcWrite(PIN_LED, currentPWM);
      preferences.putInt("led_val", currentPWM); // Simpan status
      
      // Kirim status terbaru ke MQTT
      String status = "Tombol ditekan, PWM: " + String(currentPWM);
      client.publish("wemos/status", status.c_str());
      
      Serial.println(status);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  ledcAttach(PIN_LED, freq, resolution);
  
  // Implementasi INTERUPSI
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), handleButton, FALLING);

  xTaskCreatePinnedToCore(TaskNetwork, "Net", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskControl, "Ctrl", 4096, NULL, 1, NULL, 1);
}

void loop() {}