#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <functional>
#include <list>
#include "ArduinoJson.h"

/**
 * Base Parameter
 */
class BaseParameter {
public:
    virtual void fromJson(JsonObject *json) = 0;
    virtual void toJson(JsonObject *json) = 0;
};

/**
 * Parameter
 */
template<typename T>
class ConfigParameter : public BaseParameter {
public:
    ConfigParameter(const char *name, T *ptr, std::function<void(const char*)> cb = NULL) {
        this->name = name;
        this->ptr = ptr;
        this->cb = cb;
    }

    void fromJson(JsonObject *json) {
        if (json->is<T>(name)) {
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
};

/**
 * Config Manager
 */
class ConfigManager {
public:
    ConfigManager() {};

    void setAPName(const char *name);
    void setAPFilename(const char *filename);
    void loop();

    template<typename T>
    void begin(T &config) {
        this->config = &config;
        this->configSize = sizeof(T);

        EEPROM.begin(96 + this->configSize);

        setup();
    }

    template<typename T>
    void addParameter(const char *name, T *variable) {
        parameters.push_back(new ConfigParameter<T>(name, variable));
    }

private:
    void *config;
    size_t configSize;
    char *apName = (char *)"Thing";
    char *apFilename = (char *)"/index.html";
    std::unique_ptr<ESP8266WebServer> server;
    std::list<BaseParameter*> parameters;

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
