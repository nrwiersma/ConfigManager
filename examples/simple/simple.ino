#include "ConfigManager.h"

struct Config {
    char name[20];
    bool enabled;
    int8_t hour;
    char password[20];
} config;

struct Metadata {
    int8_t version;
} meta;

ConfigManager configManager;

void createCustomRoute(WebServer *server) {
    server->on("/custom", HTTPMethod::HTTP_GET, [server](){
        server->send(200, "text/plain", "Hello, World!");
    });
}

void setup() {
    Serial.begin(115200);
    Serial.println("");

    meta.version = 3;

    // Setup config manager
    configManager.setAPName("Demo");
    configManager.setAPFilename("/index.html");
    configManager.addParameter("name", config.name, 20);
    configManager.addParameter("enabled", &config.enabled);
    configManager.addParameter("hour", &config.hour);
    configManager.addParameter("password", config.password, 20, set);
    configManager.addParameter("version", &meta.version, get);

    configManager.setAPCallback(createCustomRoute);
    configManager.setAPICallback(createCustomRoute);

    configManager.begin(config);
}

void loop() {
    configManager.loop();

    // Add your loop code here
}
