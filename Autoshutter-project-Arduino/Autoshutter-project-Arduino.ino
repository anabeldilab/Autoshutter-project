#include <WiFi.h>
#include <PubSubClient.h>
#include <AccelStepper.h>

WiFiClient espClient;
PubSubClient client(espClient);

RTC_DATA_ATTR bool powerSaveMode = false;

// MQTT Broker
const char *mqtt_broker = "192.168.1.48";
const char *autoshutterTopic = "esp32/autoshutter";
const char *sleepTopic = "esp32/sleep";
const char *mqtt_username = "alu0101206011";
const char *mqtt_password = "";
const int mqtt_port = 1883;

// Timers 
unsigned long lastSleep = 0;
unsigned long lastMsg = 0;

const int MOTOR_PIN1 = 26;
const int MOTOR_PIN2 = 25;
const int MOTOR_PIN3 = 33;
const int MOTOR_PIN4 = 32;

const int ENCODER_CW_PIN = 14; // Clockwise pin
const int ENCODER_CC_PIN = 27; // Counter clockwise pin
const int ENCODER_SW_PIN = 12;
int ENCODER_POS = 1;

// Light control
const int LDR_PIN = 34;
const int MAX_LIGHT_LEVEL = 4100;

#define MotorInterfaceType 8
AccelStepper stepper(MotorInterfaceType, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

void stopMotor() {
  stepper.stop(); // Stop the motor with AccelStepper
  disableMotor(); // Disable the motor coils
}

// Function to disable the motor
void disableMotor() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
  digitalWrite(MOTOR_PIN3, LOW);
  digitalWrite(MOTOR_PIN4, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(MOTOR_PIN3, OUTPUT);
  pinMode(MOTOR_PIN4, OUTPUT);
  pinMode(ENCODER_CW_PIN, INPUT);
  pinMode(ENCODER_CC_PIN, INPUT);
  pinMode(ENCODER_SW_PIN, INPUT);
  
  startWiFi();

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  connect();

  // Initialize the stepper
  stepper.setMaxSpeed(1000);     // Ajustar valores con pruebas reales (Depende del peso)
  stepper.setAcceleration(50);   // Ajustar valores con pruebas reales (Depende del peso)

  // despertar cada 10 segundos
  esp_sleep_enable_timer_wakeup(10000000);
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
      stepper.move(-4096);
    } else if (messageTemp == "UP") {
      stepper.move(4096);
    } else if (messageTemp == "STOP") {
      stopMotor();
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

  // Run the stepper
  if (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  // Sends light sensor percentage every 5 seconds
  if (now - lastMsg > 5000) {
    lastMsg = now;
    float LDR_input = analogRead(LDR_PIN);
    float LDR_percentage = (LDR_input / MAX_LIGHT_LEVEL) * 100;
    Serial.print("Light level: ");
    Serial.print(LDR_percentage);
    Serial.println("%");
    client.publish("esp32/light", String((LDR_input / MAX_LIGHT_LEVEL) * 100).c_str());
  }

  if (powerSaveMode && now - lastSleep > 5000) { // Time to receive MQTT callback
    lastSleep = now;
    Serial.println("Sleeping...");
    esp_deep_sleep_start(); 
  }
}


