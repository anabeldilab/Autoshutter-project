#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

RTC_DATA_ATTR int cont = 0;
#define TOUCH_THRESHOLD 40

// MQTT Broker
const char *mqtt_broker = "node02.myqtthub.com";
const char *topic = "esp32/autoshutter";
const char *mqtt_username = "alu0101206011";
const char *mqtt_password = "";
const int mqtt_port = 1883;

static unsigned long lastMsg = 0;

const int MOTOR_PIN1 = 25;
const int MOTOR_PIN2 = 26;
const int ENCODER_CW_PIN = 14;
const int ENCODER_CC_PIN = 27;
const int ENCODER_SW_PIN = 12;
int ENCODER_POS = 1;

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

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "alu0101206011-esp32";
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public emqx MQTT connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }

  // publish and subscribe
  // client.publish(topic, "Hi, I'm ESP32 ^^");
  //client.subscribe(topic);

  motorStop();

  /*touchAttachInterrupt(T2, awake, TOUCH_THRESHOLD);
  esp_sleep_enable_touchpad_wakeup();

  Serial.println("Sleeping...");
  delay(1000);
  esp_deep_sleep_start(); */

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
}


void reconnect() {
  String client_id = "alu0101206011-esp32";
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  unsigned long now = millis();
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if ((ENCODER_POS >= 200) || (ENCODER_POS <= 0)) {
    motorStop();
  }

  if (now - lastMsg > 5000) {
    lastMsg = now;
    Serial.print("Encoder position: ");
    Serial.println(ENCODER_POS);
    client.publish("esp32/encoder", String(ENCODER_POS).c_str());
  }
}
               
void motorStop() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
}


