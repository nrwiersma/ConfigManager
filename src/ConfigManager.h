#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <DNSServer.h>
#include <EEPROM.h>
#include <FS.h>

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32) //ESP32
    #include <SPIFFS.h>
    #include <WiFi.h>
    #include <WebServer.h>
#endif

#include <functional>
#include <list>
#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
    #define WIFI_OPEN  ENC_TYPE_NONE
#elif defined(ARDUINO_ARCH_ESP32) //ESP32
    #define WIFI_OPEN  WIFI_AUTH_OPEN
#endif

#include "ConfigParameter.h"
#include "ConfigStringParameter.h"

#define MAGIC_LENGTH 2
#define SSID_LENGTH 32
#define PASSWORD_LENGTH 64
#define CONFIG_OFFSET 98 // sum of previous - where configs start in memory

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
    using WebServer = ESP8266WebServer;
#endif

extern const char mimeHTML[];
extern const char mimeJSON[];
extern const char mimePlain[];
extern const char mimeCSS[];
extern const char mimeJS[];

enum Mode {ap, api};

/**
 * Config Manager
 */
class ConfigManager {
public:
    ConfigManager() {}

    Mode getMode();
    void setAPName(const char *name);
    void setAPPassword(const char *password);
    void setAPFilename(const char *filename);
    void setAPTimeout(const int timeout);
    void setWifiConnectRetries(const int retries);
    void setWifiConnectInterval(const int interval);
    void setWebPort(const int port);
    void setAPCallback(std::function<void(WebServer*)> callback);
    void setAPICallback(std::function<void(WebServer*)> callback);
    void loop();
    void streamFile(const char *file, const char mime[]);
    void handleNotFound();
    void clearSettings(bool reboot);
    void clearWifiSettings(bool reboot);

    template<typename T>
    void begin(T &config) {
        this->config = &config;
        this->configSize = sizeof(T);

        EEPROM.begin(CONFIG_OFFSET + this->configSize);

        setup();
    }

    template<typename T>
    void addParameter(const char *name, T *variable) {
        parameters.push_back(new ConfigParameter<T>(name, variable));
    }
    template<typename T>
    void addParameter(const char *name, T *variable, ParameterMode mode) {
        parameters.push_back(new ConfigParameter<T>(name, variable, mode));
    }
    void addParameter(const char *name, char *variable, size_t size) {
        parameters.push_back(new ConfigStringParameter(name, variable, size));
    }
    void addParameter(const char *name, char *variable, size_t size, ParameterMode mode) {
        parameters.push_back(new ConfigStringParameter(name, variable, size, mode));
    }
    void save();

private:
    Mode mode;
    void *config;
    size_t configSize;

    char *apName = (char *)"Thing";
    char *apPassword = NULL;
    char *apFilename = (char *)"/index.html";
    int apTimeout = 0;
    unsigned long apStart = 0;

    int wifiConnectRetries = 20;
    int wifiConnectInterval = 500;

    int webPort = 80;

    std::unique_ptr<DNSServer> dnsServer;
    std::unique_ptr<WebServer> server;
    std::list<BaseParameter*> parameters;

    std::function<void(WebServer*)> apCallback;
    std::function<void(WebServer*)> apiCallback;

    JsonObject &decodeJson(String jsonString);

    void handleAPGet();
    void handleAPPost();
    void handleScanGet();
    void handleRESTGet();
    void handleRESTPut();

    bool wifiConnected();
    void setup();
    void startAP();
    void startApi();
    void createBaseWebServer();

    void readConfig();
    void writeConfig();
    void storeWifiSettings(String ssid, String password, bool resetMagic);
    boolean isIp(String str);
    String toStringIP(IPAddress ip);
};

#endif /* __CONFIGMANAGER_H__ */
