#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

RTC_DATA_ATTR bool powerSaveMode = false;

// MQTT Broker
const char *mqtt_broker = "node02.myqtthub.com";
const char *autoshutterTopic = "esp32/autoshutter";
const char *sleepTopic = "esp32/sleep";
const char *mqtt_username = "alu0101206011";
const char *mqtt_password = "";
const int mqtt_port = 1883;

unsigned long lastSleep = 0;
unsigned long lastMsg = 0;

const int MOTOR_PIN1 = 25;
const int MOTOR_PIN2 = 26;
const int ENCODER_CW_PIN = 14;
const int ENCODER_CC_PIN = 27;
const int ENCODER_SW_PIN = 12;
int ENCODER_POS = 1;

const int LDR_PIN = 34;
const int MAX_LIGHT_LEVEL = 4100;

void IRAM_ATTR getEncoderTurn () {
  if (ENCODER_POS < 0) {
    ENCODER_POS = 0;
  }
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 60) {
    int pinA = digitalRead(ENCODER_CW_PIN);
    int pinB = digitalRead(ENCODER_CC_PIN);
    int result = pinB > pinA ? 1 : -1;
    ENCODER_POS += result;
  }
  last_interrupt_time = interrupt_time;
}


void awake() {
  Serial.println("I'm awake");
}


void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(ENCODER_CW_PIN, INPUT);
  pinMode(ENCODER_CC_PIN, INPUT);
  pinMode(ENCODER_SW_PIN, INPUT);



  startWiFi();
  //esp_task_wdt_deinit();

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  connect();

  motorStop();

  // despertar cada 10 segundos
  esp_sleep_enable_timer_wakeup(10000000);

  attachInterrupt(digitalPinToInterrupt(ENCODER_CW_PIN), getEncoderTurn, FALLING);
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/autoshutter") {
    if (messageTemp == "DOWN") {
      Serial.println("LowerBlinds");
      if (ENCODER_POS <= 200) {
        digitalWrite(MOTOR_PIN1, HIGH);
        digitalWrite(MOTOR_PIN2, LOW);
      }
    } else if (messageTemp == "UP") {
      Serial.println("raiseBlinds");
      if (ENCODER_POS > 0) {
        digitalWrite(MOTOR_PIN1, LOW);
        digitalWrite(MOTOR_PIN2, HIGH);
      }
    } else if (messageTemp == "STOP") {
      Serial.println("stopBlinds");
      motorStop();
    }
  } else if (String(topic) == "esp32/sleep") {
    if (messageTemp == "ON") {
      Serial.println("PowerSaveMode ON");
      powerSaveMode = true;
    } else if (messageTemp == "OFF") {
      Serial.println("PowerSaveMode OFF");
      powerSaveMode = false;
    }
  }

}


void connect() {
  String client_id = "alu0101206011-esp32";
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(autoshutterTopic);
      client.subscribe(sleepTopic);
    } else {
      Serial.print("failed, reconnect ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  unsigned long now = millis();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi: lost connection");
    startWiFi();
  }
  if (!client.connected()) {
    Serial.println("MQTT: lost connection");
    connect();
  }

  client.loop();

  if ((ENCODER_POS >= 200) || (ENCODER_POS <= 0)) {
    motorStop();
  }

  if (now - lastMsg > 5000) {
    lastMsg = now;
    float LDR_input = analogRead(LDR_PIN);
    Serial.print("Light level: ");
    Serial.print((LDR_input / MAX_LIGHT_LEVEL) * 100);
    Serial.println("%");
    client.publish("esp32/light", String((LDR_input / MAX_LIGHT_LEVEL) * 100).c_str());
  }

  if (powerSaveMode && now - lastSleep > 5000) { // Time to receive MQTT callback
    lastSleep = now;
    Serial.println("Sleeping...");
    esp_deep_sleep_start(); 
  }
}
               
void motorStop() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
}


