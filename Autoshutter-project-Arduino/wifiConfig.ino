const char* ssid = "Amplificador_wifi";
const char* password = "123456789";

void startWiFi() {
  // Conexión WiFi
  static long last_millis = 0;
  bool connected = false;
  while (!connected) {
    unsigned long now = millis();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && now - last_millis < 5000) {
      now = millis();
      delay(500);
      Serial.print(".");
    }
    last_millis = now;
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
    } else {
      Serial.println("\nConnection failed");
      Serial.println("Trying to reconnect");
    }
  }
  Serial.println("");
  Serial.print("Conectado a la red WiFi. IP: ");
  Serial.println(WiFi.localIP());
}
