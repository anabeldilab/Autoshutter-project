void setup() {
  Serial.begin(115200);
  startWiFi();

  // Iniciar servidor web
  startWebServer();
}

void loop() {
  // Aquí podrías incluir lógica adicional si es necesaria
}

