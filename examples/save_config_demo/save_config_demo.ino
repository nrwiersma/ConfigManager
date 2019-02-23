#include "ConfigManager.h"

const char mimeHTML[] PROGMEM = "text/html";
const char *configHTMLFile = "/settings.html";

struct Config {
  char name[255];
  bool enabled;

  char mqtt_host[255];
  char mqtt_port[10];
  char mqtt_username[255];
  char mqtt_password[255];
  char mqtt_topic[255];
} config;

struct Metadata {
  int8_t version;
} meta;

ConfigManager configManager;

void createCustomRoute(WebServer *server) {
  server->on("/settings.html", HTTPMethod::HTTP_GET, [server](){
    SPIFFS.begin();

    File f = SPIFFS.open(configHTMLFile, "r");
    if (!f) {
      Serial.println(F("file open failed"));
      server->send(404, FPSTR(mimeHTML), F("File not found"));
      return;
    }

    server->streamFile(f, FPSTR(mimeHTML));

    f.close();
  });
}

void setup() {
  Serial.begin(115200);
  Serial.println("");

  meta.version = 3;

  // Setup config manager
  configManager.setAPName("Demo");
  configManager.setAPFilename("/index.html");

  configManager.addParameter("name", config.name, 255);

  configManager.addParameter("mqtt_host", config.mqtt_host, 255);
  configManager.addParameter("mqtt_port", config.mqtt_port, 10);
  configManager.addParameter("mqtt_username", config.mqtt_username, 255);
  configManager.addParameter("mqtt_password", config.mqtt_password, 255, set);
  configManager.addParameter("mqtt_topic", config.mqtt_topic, 255);

  configManager.addParameter("enabled", &config.enabled);
  configManager.addParameter("version", &meta.version, get);

  configManager.setAPICallback(createCustomRoute);

  configManager.begin(config);
}

void loop() {
  configManager.loop();
}
