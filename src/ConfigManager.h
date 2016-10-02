#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
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
 * Config Parameter
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
};

/**
 * Config String Parameter
 */
class ConfigStringParameter : public BaseParameter {
public:
    ConfigStringParameter(const char *name, char *ptr, size_t length) {
        this->name = name;
        this->ptr = ptr;
        this->length = length;
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
};

/**
 * Config Manager
 */
class ConfigManager {
public:
    ConfigManager() {}

    void setAPName(const char *name);
    void setAPFilename(const char *filename);
    void setAPICallback(std::function<void(ESP8266WebServer*)> callback);
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
    void addParameter(const char *name, char *variable, size_t size) {
        parameters.push_back(new ConfigStringParameter(name, variable, size));
    }
    void save();

private:
    void *config;
    size_t configSize;
    char *apName = (char *)"Thing";
    char *apFilename = (char *)"/index.html";
    std::unique_ptr<ESP8266WebServer> server;
    std::list<BaseParameter*> parameters;
    std::function<void(ESP8266WebServer*)> apiCallback;

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
