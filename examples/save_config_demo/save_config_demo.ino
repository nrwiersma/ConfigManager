#include "ConfigManager.h"

const char *settingsHTML = (char *)"/settings.html";
const char *stylesCSS = (char *)"/styles.css";
const char *mainJS = (char *)"/main.js";

struct Config {
  char device_name[32];
  float inching_delay;
  int8_t led;
  bool enabled;
} config;

struct Metadata {
  int8_t version;
} meta;

ConfigManager configManager;

void APCallback(WebServer *server) {
    server->on("/styles.css", HTTPMethod::HTTP_GET, [server](){
        configManager.streamFile(stylesCSS, mimeCSS);
    });

    DebugPrintln(F("AP Mode Enabled. You can call other functions that should run after a mode is enabled ... "));
}


void APICallback(WebServer *server) {
  server->on("/disconnect", HTTPMethod::HTTP_GET, [server](){
    configManager.clearWifiSettings(false);
  });

  server->on("/settings.html", HTTPMethod::HTTP_GET, [server](){
    configManager.streamFile(settingsHTML, mimeHTML);
  });

  // NOTE: css/js can be embedded in a single page HTML
  server->on("/styles.css", HTTPMethod::HTTP_GET, [server](){
    configManager.streamFile(stylesCSS, mimeCSS);
  });

  server->on("/main.js", HTTPMethod::HTTP_GET, [server](){
    configManager.streamFile(mainJS, mimeJS);
  });

  server->on("/reboot", HTTPMethod::HTTP_GET, [server](){
    ESP.restart();
  });

}

void setup() {
  Serial.begin(115200);
  DEBUG_MODE = true;
  DebugPrintln(F(""));

  meta.version = 3;

  // Setup config manager
  configManager.setAPName("Demo");
  configManager.setAPFilename("/index.html");

  // Settings variables
  configManager.addParameter("device_name", config.device_name, 32);
  configManager.addParameter("inching_delay", &config.inching_delay);
  configManager.addParameter("led", &config.led);
  configManager.addParameter("enabled", &config.enabled);

  // Meta Settings
  configManager.addParameter("version", &meta.version, get);

  // Init Callbacks
  configManager.setAPCallback(APCallback);
  configManager.setAPICallback(APICallback);

  configManager.begin(config);
}

void loop() {
  configManager.loop();

  // Add your loop code here
}
