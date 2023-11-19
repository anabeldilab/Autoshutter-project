
void handleRoot() {
  server.send(200, "text/html", HTML_UI);
}

void handle_notfound() {
  server.send(404, "text/plain", "Error: Página no encontrada");
}

void handleOn() {
  //htmlButtonState = 1;
  server.send(200, "text/plain", "Modo ON activado");
}

void handleOff() {
  //htmlButtonState = 0;
  server.send(200, "text/plain", "Modo OFF activado");
}

void startWebServer() {
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.onNotFound(handle_notfound);
  Serial.println("Servidor activo");
  server.begin();
}

