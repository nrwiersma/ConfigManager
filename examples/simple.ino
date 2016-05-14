#include "ConfigManager.h"

struct Config {
    bool enabled;
    int8 hour;
} config;

void setup() {
    // Setup config manager
    configManager.setAPName("Demo");
    configManager.setAPFilename("/index.html");
    configManager.addParameter("enabled", &config.enabled);
    configManager.addParameter("hour", &config.hour);
    configManager.begin(config);

    //
}

void loop() {
    configManager.loop();

    // Add your loop code here
}
