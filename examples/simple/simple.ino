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

void initCallback() {
    config.enabled = false;
    configManager.save();
}

void setup() {
    DEBUG_MODE = true; // will enable debugging and log to serial monitor
    Serial.begin(115200);
    DebugPrintln("");

    meta.version = 3;

    // Setup config manager
    configManager.setAPName("Demo");
    configManager.setAPFilename("/index.html");
    configManager.addParameter("name", config.name, 20);
    configManager.addParameter("enabled", &config.enabled);
    configManager.addParameter("hour", &config.hour);
    configManager.addParameter("password", config.password, 20, set);
    configManager.addParameter("version", &meta.version, get);

    // Create custom routes to serve via callback hooks
    configManager.setAPCallback(createCustomRoute);
    configManager.setAPICallback(createCustomRoute);

    // Create a initialization hook
    // Will run until a memory write is performed.
    configManager.setInitCallback(initCallback);

    configManager.begin(config);
}

void loop() {
    configManager.loop();

    // Add your loop code here
}
