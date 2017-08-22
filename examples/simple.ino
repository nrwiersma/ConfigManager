#include "ConfigManager.h"

struct Config {
    char name[20];
    bool enabled;
    int8 hour;
    char password[20];
} config;

struct Metadata {
    int8 version;
} meta;

ConfigManager configManager;

void createCustomRoute(ESP8266WebServer *server) {
    server->on("/custom", HTTPMethod::HTTP_GET, [server](){
        server->send(200, "text/plain", "Hello, World!");
    });
}

void setup() {
    meta.version = 3;

    // Setup config manager
    configManager.setAPName("Demo");
    configManager.setAPFilename("/index.html");
    configManager.addParameter("name", config.name, 20);
    configManager.addParameter("enabled", &config.enabled);
    configManager.addParameter("hour", &config.hour);
    configManager.addParameter("password", config.password, 20, set);
    configManager.addParameter("version", &meta.version, get);
    configManager.begin(config);

    configManager.setAPICallback(createCustomRoute);

    //
}

void loop() {
    configManager.loop();

    // Add your loop code here
}
