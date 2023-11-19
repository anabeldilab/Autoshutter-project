#include <WebServer.h>

WebServer server(80);

void setup() {
  Serial.begin(115200);
  startWiFi();

  // Iniciar servidor web
  startWebServer();
}

void loop() {
  server.handleClient();
  // Aquí podrías incluir lógica adicional si es necesaria
}

