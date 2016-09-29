#include "ConfigManager.h"

struct Config {
    char name[20];
    bool enabled;
    int8 hour;
} config;

ConfigManager configManager;

void setup() {
    // Setup config manager
    configManager.setAPName("Demo");
    configManager.setAPFilename("/index.html");
    configManager.addParameter("name", config.name, 20);
    configManager.addParameter("enabled", &config.enabled);
    configManager.addParameter("hour", &config.hour);
    configManager.begin(config);

    //
}

void loop() {
    configManager.loop();

    // Add your loop code here
}
