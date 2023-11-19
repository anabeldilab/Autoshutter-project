const char* ssid = "Redmi Note 11S";
const char* password = "Gpa7cnsi";

void startWiFi() {
  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a la red WiFi. IP: ");
  Serial.println(WiFi.localIP());
}