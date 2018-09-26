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

#define WIFI_OFFSET 2
#define CONFIG_OFFSET 98

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
    using WebServer = ESP8266WebServer;
#endif

enum Mode {ap, api};

enum ParameterMode { get, set, both};

/**
 * Base Parameter
 */
class BaseParameter {
public:
    virtual ParameterMode getMode() = 0;
    virtual void fromJson(JsonObject *json) = 0;
    virtual void toJson(JsonObject *json) = 0;
};

/**
 * Config Parameter
 */
template<typename T>
class ConfigParameter : public BaseParameter {
public:
    ConfigParameter(const char *name, T *ptr, ParameterMode mode = both, std::function<void(const char*)> cb = NULL) {
        this->name = name;
        this->ptr = ptr;
        this->cb = cb;
        this->mode = mode;
    }

    ParameterMode getMode() {
        return this->mode;
    }

    void fromJson(JsonObject *json) {
        if (json->containsKey(name) && json->is<T>(name)) {
            *ptr = json->get<T>(name);
        }
    }

    void toJson(JsonObject *json) {
        json->set(name, *ptr);

        if (cb) {
            cb(name);
        }
    }

private:
    const char *name;
    T *ptr;
    std::function<void(const char*)> cb;
    ParameterMode mode;
};

/**
 * Config String Parameter
 */
class ConfigStringParameter : public BaseParameter {
public:
    ConfigStringParameter(const char *name, char *ptr, size_t length, ParameterMode mode = both) {
        this->name = name;
        this->ptr = ptr;
        this->length = length;
        this->mode = mode;
    }

    ParameterMode getMode() {
        return this->mode;
    }

    void fromJson(JsonObject *json) {
        if (json->containsKey(name) && json->is<char *>(name)) {
            const char * value = json->get<const char *>(name);

            memset(ptr,'\n',length);
            strncpy(ptr, const_cast<char*>(value), length - 1);
        }
    }

    void toJson(JsonObject *json) {
        json->set(name, ptr);
    }

private:
    const char *name;
    char *ptr;
    size_t length;
    ParameterMode mode;
};

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
    void setAPCallback(std::function<void(WebServer*)> callback);
    void setAPICallback(std::function<void(WebServer*)> callback);
    void loop();

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

    std::unique_ptr<DNSServer> dnsServer;
    std::unique_ptr<WebServer> server;
    std::list<BaseParameter*> parameters;

    std::function<void(WebServer*)> apCallback;
    std::function<void(WebServer*)> apiCallback;

    JsonObject &decodeJson(String jsonString);

    void handleAPGet();
    void handleAPPost();
    void handleRESTGet();
    void handleRESTPut();
    void handleNotFound();

    bool wifiConnected();
    void setup();
    void startAP();
    void startApi();

    void readConfig();
    void writeConfig();
    boolean isIp(String str);
    String toStringIP(IPAddress ip);
};

#endif /* __CONFIGMANAGER_H__ */
