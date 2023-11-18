#include <WebServer.h>

#include "../include/config.h"
#include "../include/WebServer.h"
#include "../include/HtmlUI.h"

WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", HTML_UI);
}

void handle_notfound() {
  server.send(404, "text/plain", "Error: Página no encontrada");
}

void handleOn() {
  htmlButtonState = 1;
  server.send(200, "text/plain", "Modo ON activado");
}

void startWebServer() {
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.onNotFound(handle_notfound);
  Serial.println("Servidor activo");
  server.begin();
}

