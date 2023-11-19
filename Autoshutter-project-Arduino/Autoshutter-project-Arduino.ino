#include <WiFi.h>

WiFiServer server(80);

const int MOTOR_PIN1 = 25;
const int MOTOR_PIN2 = 26;
const int ENCODER_CW_PIN = 14;
const int ENCODER_CC_PIN = 27;
const int ENCODER_SW_PIN = 12;
int ENCODER_POS = 1;

String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

const char* HTML_UI = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html { font-family: Arial; display: inline-block; text-align: center;}
    .button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
    .button2 {background-color: #555555;}
  </style>
</head>
<body>
  <h1>ESP32 Blind Control</h1>
  <p><a href="/lowerBlinds"><button class="button">Lower Blinds</button></a></p>
  <p><a href="/raiseBlinds"><button class="button button2">Raise Blinds</button></a></p>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(ENCODER_CW_PIN, INPUT);
  pinMode(ENCODER_CC_PIN, INPUT);
  pinMode(ENCODER_SW_PIN, INPUT);

  motorStop();

  attachInterrupt(digitalPinToInterrupt(ENCODER_CW_PIN), getEncoderTurn, FALLING);

  startWiFi();

  // Iniciar servidor web
  Serial.println("Servidor activo");
  server.begin();
}

void loop() {
  if ((ENCODER_POS >= 200) || (ENCODER_POS <= 0)) {
    motorStop();
  }
  handleClientRequest();
}


void handleClientRequest() {
  WiFiClient client = server.available();

  if (client) {                             
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          
    String currentLine = "";                
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  
      currentTime = millis();
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);                    
        header += c;
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /lowerBlinds") >= 0) {
              Serial.println("LowerBlinds");
              if (ENCODER_POS <= 200) {
                digitalWrite(MOTOR_PIN1, HIGH);
                digitalWrite(MOTOR_PIN2, LOW);
              }
            } else if (header.indexOf("GET /raiseBlinds") >= 0) {
              Serial.println("raiseBlinds");
              if (ENCODER_POS > 0) {
                digitalWrite(MOTOR_PIN1, LOW);
                digitalWrite(MOTOR_PIN2, HIGH);
              }
            }
            
            client.println(HTML_UI);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;     
        }
      }
    }

    header = "";

    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void getEncoderTurn () {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 60) {
    int pinA = digitalRead(ENCODER_CW_PIN);
    int pinB = digitalRead(ENCODER_CC_PIN);
    int result = pinB > pinA ? 1 : -1;
    //if (ENCODER_POS > 0) {
      ENCODER_POS += 1 * result;
    //}
    Serial.print("POS: ");
    Serial.println(ENCODER_POS);
  }
  last_interrupt_time = interrupt_time;
}

void motorStop() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
}


